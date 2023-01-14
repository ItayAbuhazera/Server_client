package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.impl.rci.ObjectEncoderDecoder;
import bgu.spl.net.srv.BaseServer;
import bgu.spl.net.srv.BlockingConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Map;
import java.util.function.Supplier;

public class StompServer extends BaseServer {
    private final int port;
    private final Supplier<MessagingProtocol<StompFrame>> protocolFactory;
    private final Supplier<MessageEncoderDecoder<StompFrame>> encdecFactory;
    private ServerSocket sock;
    private Map<Integer, Integer> connections;
    private ConnectionsImpl connectionsImpl;
    public StompServer(int port, Supplier<MessagingProtocol<StompFrame>> protocolFactory, Supplier<MessageEncoderDecoder<StompFrame>> encdecFactory) {
        super(port, protocolFactory, encdecFactory);
        this.port = port;
        this.protocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
        this.sock = null;
        this.connectionsImpl = new ConnectionsImpl();
    }
//    public StompServer(int port, Supplier<MessagingProtocol<T>>  protocolFactory, Supplier<MessageEncoderDecoder<T>> encdecFactory) {
//        this(port, protocolFactory, encdecFactory);
//        this.port = port;
//        this.protocolFactory = protocolFactory;
//        this.encdecFactory = encdecFactory;
//    }

    public static void main(String[] args) {
        int port = Integer.parseInt(args[0]);
        switch (args[1]) {
            case "tpc" -> runTPC(port);
            case "reactor" -> runReactor(port);
            default -> System.out.println("Invalid argument");
        }
    }

    private int getPort() {
        return this.port; //port
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
