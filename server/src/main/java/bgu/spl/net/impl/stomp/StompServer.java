package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.BaseServer;
import bgu.spl.net.srv.BlockingConnectionHandler;
import bgu.spl.net.srv.Server;

import java.io.IOException;
import java.util.function.Supplier;

public class StompServer extends BaseServer {
    public StompServer(int port, Supplier<MessagingProtocol<StompFrame>> protocolFactory, Supplier<MessageEncoderDecoder<StompFrame>> encdecFactory) {
        super(port, protocolFactory, encdecFactory);
    }

    public static void main(String[] args) {
        int port = Integer.parseInt(args[0]);
        switch (args[1]) {
            case "tpc":
                runTPC(port);
                break;
            case "reactor":
                runReactor(port);
                break;
            default:
                System.out.println("Invalid argument");
                break;
        }
    }

    @Override
    public void close() throws IOException {
        this.sock.close();
        super.close();
    }

    @Override
    protected void execute(BlockingConnectionHandler handler) {

    }

    private static void runTPC(int port){
        // you can use any server...
        Server.threadPerClient(
                port, //port
                () -> new StompProtocol(), //protocol factory
                StompEncDec::new //message encoder decoder factory
        ).serve();
    }

    private static void runReactor(int port){
        // you can use any server...
        Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                port, //port
                () -> new StompProtocol(), //protocol factory
                StompEncDec::new //message encoder decoder factory
        ).serve();
    }
}
