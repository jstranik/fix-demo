// Server.cpp
#include "quickfix/FixFields.h"
#include "quickfix/FixValues.h"
#include <quickfix/FileStore.h>
#include <quickfix/SocketAcceptor.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/fix42/NewOrderSingle.h>
#include <quickfix/fix42/ExecutionReport.h>
#include <quickfix/FileLog.h>
#include <iostream>

class ServerApplication : public FIX::Application, public FIX::MessageCracker {
public:
    void onCreate(const FIX::SessionID&) {}
    void onLogon(const FIX::SessionID&) {}
    void onLogout(const FIX::SessionID&) {}
    void toAdmin(FIX::Message&, const FIX::SessionID&) {}
    void toApp(FIX::Message&, const FIX::SessionID&) {}
    void fromAdmin(const FIX::Message&, const FIX::SessionID&)  {}

    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID)  {
        crack(message, sessionID);
    }

    void onMessage(const FIX42::NewOrderSingle& message, const FIX::SessionID& sessionID) {
        std::cout << "Received NewOrderSingle" << std::endl;

        FIX42::ExecutionReport executionReport;
        executionReport.setField(FIX::OrderID("123"));
        executionReport.setField(FIX::ExecID("456"));
        executionReport.setField(FIX::ExecTransType(FIX::ExecTransType_NEW));
        executionReport.setField(FIX::ExecType(FIX::ExecType_FILL));

        FIX::Symbol symbol;
        FIX::Side side;
        FIX::OrderQty qty;
        message.get(symbol);
        message.get(side);
        message.get(qty);


        executionReport.setField(FIX::OrdStatus(FIX::OrdStatus_FILLED));
        executionReport.setField(symbol);
        executionReport.setField(side);
        executionReport.setField(FIX::LeavesQty(0));
        executionReport.setField(FIX::CumQty(qty.getField()));
        executionReport.setField(FIX::AvgPx(100.0));  // Example price

        FIX::Session::sendToTarget(executionReport, sessionID);
    }
};

int main(int argc, char* argv[]) {
    try {
        FIX::SessionSettings settings("server_settings.cfg", true);
        ServerApplication application;
        FIX::FileStoreFactory storeFactory(settings);
        FIX::FileLogFactory logFactory(settings);
        FIX::SocketAcceptor acceptor(application, storeFactory, settings, logFactory);

        acceptor.start();
        std::cout << "Server started. Press enter to quit." << std::endl;
        std::cin.get();
        acceptor.stop();

        return 0;
    } catch (FIX::ConfigError& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}
