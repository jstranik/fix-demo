// Server.cpp
#include <quickfix/FileStore.h>
#include <quickfix/SocketAcceptor.h>
#include <quickfix/Session.h>
#include <quickfix/SessionSettings.h>
#include <quickfix/Application.h>
#include <quickfix/MessageCracker.h>
#include <quickfix/fix44/NewOrderSingle.h>
#include <quickfix/fix44/ExecutionReport.h>
#include <iostream>

class ServerApplication : public FIX::Application, public FIX::MessageCracker {
public:
    void onCreate(const FIX::SessionID&) {}
    void onLogon(const FIX::SessionID&) {}
    void onLogout(const FIX::SessionID&) {}
    void toAdmin(FIX::Message&, const FIX::SessionID&) {}
    void toApp(FIX::Message&, const FIX::SessionID&) throw(FIX::DoNotSend) {}
    void fromAdmin(const FIX::Message&, const FIX::SessionID&) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) {}

    void fromApp(const FIX::Message& message, const FIX::SessionID& sessionID) throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType) {
        crack(message, sessionID);
    }

    void onMessage(const FIX44::NewOrderSingle& message, const FIX::SessionID& sessionID) {
        std::cout << "Received NewOrderSingle" << std::endl;

        FIX44::ExecutionReport executionReport;
        executionReport.setField(FIX::OrderID("123"));
        executionReport.setField(FIX::ExecID("456"));
        executionReport.setField(FIX::ExecType(FIX::ExecType_FILL));
        executionReport.setField(FIX::OrdStatus(FIX::OrdStatus_FILLED));
        executionReport.setField(FIX::Symbol(message.getSymbol()));
        executionReport.setField(FIX::Side(message.getSide()));
        executionReport.setField(FIX::LeavesQty(0));
        executionReport.setField(FIX::CumQty(message.getOrderQty()));
        executionReport.setField(FIX::AvgPx(100.0));  // Example price

        FIX::Session::sendToTarget(executionReport, sessionID);
    }
};

int main(int argc, char* argv[]) {
    try {
        FIX::SessionSettings settings("server_settings.cfg");
        ServerApplication application;
        FIX::FileStoreFactory storeFactory(settings);
        FIX::ScreenLogFactory logFactory(settings);
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
