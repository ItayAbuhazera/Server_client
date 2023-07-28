package bgu.spl.net.impl.echo;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;

import java.time.LocalDateTime;

public class EchoProtocol implements MessagingProtocol<String> {

    private boolean shouldTerminate = false;


    public String process(String msg, ConnectionHandler<String> ch) {
        shouldTerminate = "bye".equals(msg);
        System.out.println("[" + LocalDateTime.now() + "]: " + msg);
        return createEcho(msg);
    }

    private String createEcho(String message) {
        String echoPart = message.substring(Math.max(message.length() - 2, 0), message.length());
        return message + " .. " + echoPart + " .. " + echoPart + " ..";
    }

    public String process(String msg) {
        return null;
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    @Override
    public void start(ConnectionHandler<String> ch, int connectionId) {

    }
}
