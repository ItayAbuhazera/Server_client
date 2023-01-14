package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

import java.util.Hashtable;

public class StompProtocol implements StompMessagingProtocol<StompFrame> {
    private int connectionId;
    private ConnectionsImpl<StompFrame> connections;
    private static final Hashtable<String, Integer> commandToInt = new Hashtable<>();
    private boolean shouldTerminate;
    private int msgId;

    public StompProtocol(){
        connectionId = -1;
        connections = new ConnectionsImpl<>();
        shouldTerminate = false;
        msgId = -1;
        commandToInt.put("CONNECT", 1);
        commandToInt.put("SEND", 2);
        commandToInt.put("SUBSCRIBE", 3);
        commandToInt.put("UNSUBSCRIBE", 4);
        commandToInt.put("DISCONNECT", 5);
    }

    @Override
    public void start(int connectionId, Connections<StompFrame> connections) {
//        this.connectionId = connectionId;
//        this.connections = connections;
    }

    private int assignMsgId(){
        msgId++;
        return msgId;
    }

    @Override
    public StompFrame process(StompFrame newFrame) {

        switch (commandToInt.get(newFrame.getCommand())) {
            case 1 -> {
                //CONNECT
                connect(newFrame);
            }

            case 2 -> {
                //SEND
                send(newFrame);
            }

            case 3 -> {
                //SUBSCRIBE
                subscribe(newFrame);
            }

            case 4 -> {
                //UNSUBSCRIBE
                unsubscribe(newFrame);
            }

            case 5 -> {
                //DISCONNECT
                disconnect(newFrame);
            }

            //Invalid command
            default -> {
                error(connectionId, "Invalid frame", "", newFrame);
            }
        }
        return null;
    }

    private void connect(StompFrame frame){
        if (frame.getHeaders().containsKey("accept-version")) {
            if (frame.getHeaders().get("accept-version").equals("1.2")) {
                    if(connections.getUserNamePassword().containsKey(frame.getHeaderValue("login"))) {
                        if (connections.getUserNamePassword().get(frame.getHeaders().get("login")).equals(frame.getHeaders().get("passcode"))) {
                            Hashtable<String, String> sendHeaders = new Hashtable<>();
                            sendHeaders.put("version", frame.getHeaderValue("version"));
                            connections.send(connectionId, new StompFrame("CONNECTED", sendHeaders, ""));

                        } else {
                            error(connectionId, "Invalid login or password", "", frame);
                        }
                    }
                        else{
                            connections.register(frame.getHeaderValue("login"), frame.getHeaderValue("passcode"));
                        }
            } else {
                error(connectionId, "Invalid version", "", frame);
            }
        } else {
            error(connectionId, "Missing version", "", frame);
        }
    }

    private void send(StompFrame frame){
        if (frame.getHeaders().containsKey("destination")) {
            if (!frame.getBody().isEmpty()) {
                Hashtable<String, String> sendHeaders = new Hashtable<>();
                sendHeaders.put("destination", frame.getHeaderValue("destination"));
                sendHeaders.put("subscription", String.valueOf(connectionId));
                sendHeaders.put("message-id", String.valueOf(assignMsgId()));
                connections.send(frame.getHeaderValue("destination"), frame);
                if(frame.getHeaders().containsKey("receipt-id"))
                    receipt(frame);
            } else {
                error(connectionId, "Missing body", "", frame);
            }
        } else {
            error(connectionId, "Missing destination", "", frame);
        }
    }

    private void subscribe(StompFrame frame){
        if (frame.getHeaders().containsKey("destination")) {
            if (frame.getHeaders().containsKey("id")) {
                connections.subscribe(connectionId, frame.getHeaderValue("destination"));
                if(frame.getHeaders().containsKey("receipt-id"))
                    receipt(frame);

            } else {
                error(connectionId, "Missing id", "", frame);
            }
        } else {
            error(connectionId, "Missing destination", "", frame);
        }
    }

    private void unsubscribe(StompFrame frame){
        if (frame.getHeaders().containsKey("destination")) {
            if (frame.getHeaders().containsKey("id")) {
                connections.unsubscribe(connectionId, frame.getHeaderValue("destination"));
                if(frame.getHeaders().containsKey("receipt-id"))
                    receipt(frame);
            } else {
                error(connectionId, "Missing id", "", frame);
            }
        } else {
            error(connectionId, "Missing destination", "", frame);
        }
    }

    private void disconnect(StompFrame sourceFrame){
        receipt(sourceFrame);
        connections.disconnect(connectionId);
    }

    private void error(int connectionId, String message, String body, StompFrame sourceFrame){

        Hashtable<String, String> sendHeaders = new Hashtable<>();
        sendHeaders.put("message", message);
        if(sourceFrame.getHeaders().containsKey("receipt - id"))
            sendHeaders.put("receipt - id", message);
        if(sourceFrame.getHeaders().containsKey("message - id"))
            sendHeaders.put("message - id", message);
        connections.send(connectionId, new StompFrame("ERROR", sendHeaders, body));
        connections.disconnect(connectionId);
    }

    private void receipt(StompFrame sourceFrame){
        Hashtable<String, String> receiptHeaders = new Hashtable<>();
        receiptHeaders.put("receipt-id", sourceFrame.getHeaderValue("receipt-id"));
        connections.send(connectionId, new StompFrame("RECEIPT", receiptHeaders, ""));
    }



    @Override
    public boolean shouldTerminate() {
        return this.shouldTerminate;
    }
}
