// Server.cpp
#include "python-call.hpp"
#include "price-server.hpp"

#include "quickfix/FixFields.h"
#include "quickfix/FixValues.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/python.hpp>
#include <boost/stacktrace.hpp>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <exception>
#include <expected>
#include <fmt/format.h>
#include <iostream>
#include <limits>
#include <optional>
#include <quickfix/Application.h>
#include <quickfix/FileLog.h>
#include <quickfix/FileStore.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/SocketAcceptor.h>
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/fix42/NewOrderSingle.h>
#include <thread>

namespace po = boost::program_options;

class ServerApplication : public FIX::Application, public FIX::MessageCracker {
public:
  ServerApplication(PythonLogic &logic, update_map &prices, WSEndpoint& ws)
      : logic_(logic), prices_(prices), ws_(ws) {}

  void onCreate(const FIX::SessionID &) {}
  void onLogon(const FIX::SessionID &) {}
  void onLogout(const FIX::SessionID &) {}
  void toAdmin(FIX::Message &, const FIX::SessionID &) {}
  void toApp(FIX::Message &, const FIX::SessionID &) {}
  void fromAdmin(const FIX::Message &, const FIX::SessionID &) {}

  void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID) {
    crack(message, sessionID);
  }

  void onMessage(const FIX42::NewOrderSingle &message,
                 const FIX::SessionID &sessionID) {
    try {
      std::cout << "Received NewOrderSingle" << std::endl;

      FIX::Symbol symbol;
      FIX::Side side;
      FIX::OrderQty qty;
      FIX::Price priceF;
      message.get(symbol);
      message.get(side);
      message.get(qty);

      double price = NAN;
      if (message.getIfSet(priceF))
        price = priceF.getValue();

      prices_.push_price(symbol.getString(), price);
      json v;
      v["price"] = {{"symbol", symbol.getString()}, {"price", price}};
      ws_.send(v);

      bool result =
          logic_.should_trade(symbol.getString(), price).value_or(false);

      if (result) {
        json v;
        v["trade"] = {
          {"symbol", symbol.getString()},
          {"qty", qty.getValue()},
          {"price", price}
        };
        ws_.send(v);
      }
      FIX42::ExecutionReport executionReport;
      executionReport.setField(FIX::OrderID("123"));
      executionReport.setField(FIX::ExecID("456"));
      executionReport.setField(FIX::ExecTransType(FIX::ExecTransType_NEW));
      executionReport.setField(
          FIX::ExecType(result ? FIX::ExecType_FILL : FIX::ExecType_REJECTED));

      executionReport.setField(FIX::OrdStatus(
          result ? FIX::OrdStatus_FILLED : FIX::OrdStatus_CANCELED));
      executionReport.setField(symbol);
      executionReport.setField(side);
      executionReport.setField(FIX::LeavesQty(0));
      executionReport.setField(FIX::CumQty(qty.getField()));
      executionReport.setField(FIX::AvgPx(100.0)); // Example price

      FIX::Session::sendToTarget(executionReport, sessionID);
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
    }
  }

private:
  PythonLogic &logic_;
  update_map &prices_;
  WSEndpoint &ws_;
};

bool do_run = true;
void signalHandler(int signum) {
  fmt::print("Interrupt signal ... \n");
  do_run = false;
}

int main(int argc, char *argv[]) {
  std::set_terminate([] {
    std::cerr << boost::stacktrace::stacktrace() << std::endl;
    std::exit(EXIT_FAILURE);
  });

  std::signal(SIGINT, signalHandler);

  try {
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")(
        "settings", po::value<std::string>()->required(),
        "path to Python script")("logic", po::value<std::string>()->required(),
                                 "logic file")(
        "state", po::value<std::string>()->required(),
        "state directory")("quickfix", po::value<std::string>()->required(),
                           "quick fix directory")
      ("wsport", po::value<int>()->default_value(9998), "Listening web socket port");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 1;
    }

    po::notify(vm);
    setenv("QUICKFIX_DIR", vm["quickfix"].as<std::string>().c_str(), true);
    setenv("QUICKFIX_STATE", vm["state"].as<std::string>().c_str(), true);

    auto logic = vm["logic"].as<std::string>();

    update_map prices;
    Py_Initialize();
    PythonLogic interface(logic);

    FIX::SessionSettings settings(vm["settings"].as<std::string>(), true);

    boost::asio::io_context io;

    WSEndpoint ws(vm["wsport"].as<int>(), io);
    ServerApplication application(interface, prices, ws);
    FIX::FileStoreFactory storeFactory(settings);
    FIX::FileLogFactory logFactory(settings);
    FIX::SocketAcceptor acceptor(application, storeFactory, settings,
                                 logFactory);

    acceptor.start();

    PriceServer price_server(prices, io);
    price_server.run(9999);

    std::error_code ec;
    ws.set_running(true, ec);
    if (ec) throw std::runtime_error(fmt::format("Failed starting ws: {}", ec.message()));

    std::thread t([&io] { io.run(); });
    Py_BEGIN_ALLOW_THREADS;
    fmt::print("Server started. Press enter to quit.\n");
    while(do_run) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    fmt::print("Stopping ...\n");
    Py_END_ALLOW_THREADS;
    acceptor.stop();
    price_server.stop();
    ws.set_running(false, ec);
    t.join();

    return 0;
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
