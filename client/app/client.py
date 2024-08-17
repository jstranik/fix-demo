import quickfix as fix
import quickfix42 as fix42
import sys
import time, os, threading, random, argparse

class ClientApplication(fix.Application):
    def onCreate(self, sessionID):
        print("onCreate : Session (%s)" % sessionID.toString())

    def onLogon(self, sessionID):
        print("Logged on to session '%s'." % sessionID.toString())
        self.startSendingOrders(sessionID)

    def onLogout(self, sessionID):
        print("Logged out of session '%s'." % sessionID.toString())

    def toAdmin(self, message, sessionID):
        pass

    def fromAdmin(self, message, sessionID):
        pass

    def toApp(self, message, sessionID):
        print("Sending message: %s" % message.toString())

    def fromApp(self, message, sessionID):
        print("Received message: %s" % message.toString())

        msgType = fix.MsgType()
        message.getHeader().getField(msgType)

        if msgType.getValue() == fix.MsgType_ExecutionReport:
            self.onExecutionReport(message, sessionID)

    def onExecutionReport(self, message, sessionID):
        execType = fix.ExecType()
        ordStatus = fix.OrdStatus()
        message.getField(execType)
        message.getField(ordStatus)

        print("Received ExecutionReport")
        print("ExecType:", execType.getValue())
        print("OrdStatus:", ordStatus.getValue())

    def startSendingOrders(self, sessionID):
        def doOrders():
            while True:
                self.sendOrder(sessionID)
                time.sleep(1)

        thread = threading.Thread(target=doOrders)
        thread.daemon = True
        thread.start()


    def sendOrder(self, sessionID):
        session = fix.Session.lookupSession(sessionID)
        if not session.isLoggedOn():
            print("Skipping sending order")
            return

        print("Sending order...")
        message = fix42.NewOrderSingle()
        message.setField(fix.ClOrdID("ClientOrder1"))
        message.setField(fix.Symbol("AAPL"))
        message.setField(fix.Side(fix.Side_BUY))
        message.setField(fix.OrderQty(100))
        message.setField(fix.OrdType(fix.OrdType_MARKET))
        message.setField(fix.TransactTime())
        message.setField(fix.HandlInst(fix.HandlInst_MANUAL_ORDER_BEST_EXECUTION))
        message.setField(fix.Price(random.uniform(10,20)))

        fix.Session.sendToTarget(message, sessionID)

def main():
    parser = argparse.ArgumentParser(description="Fix client application")
    parser.add_argument("--host", type=str, help="Specify the hostname")
    parser.add_argument("--cfg", type=str, help="Fix Config file")

    args = parser.parse_args()
    print(f"args are {args}")
    try:
        os.makedirs(os.environ['QUICKFIX_STATE'], exist_ok=True)
        os.environ['FIX_TARGET_HOST'] = args.host if args.host else 'localhost'
        config_file = args.cfg if args.cfg else f"{sys.path[0]}/client_settings.cfg"
        settings = fix.SessionSettings(config_file, True)
        application = ClientApplication()
        storeFactory = fix.FileStoreFactory(settings)
        logFactory = fix.FileLogFactory(settings)
        initiator = fix.SocketInitiator(application, storeFactory, settings, logFactory)

        initiator.start()
        print("Client started. Press Ctrl+C to quit.")

        # Run until interrupted
        input()

    except (fix.ConfigError, fix.RuntimeError) as e:
        print(e)
        sys.exit(1)
    except KeyboardInterrupt:
        print("Ctrl+C pressed. Stopping client...")
    finally:
        initiator.stop()

if __name__ == '__main__':
    main()
