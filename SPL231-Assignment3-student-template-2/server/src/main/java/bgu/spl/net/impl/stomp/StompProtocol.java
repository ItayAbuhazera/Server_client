package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.util.Hashtable;

public class StompProtocol implements StompMessagingProtocol<StompFrame> {
    private ConnectionsImpl<StompFrame> connections;
    private static final Hashtable<String, Integer> commandToInt = new Hashtable<>();

    private boolean shouldTerminate;
    private static int msgId;
    private static int nextConnectionId;



    public StompProtocol(){
        connections = ConnectionsImpl.getInstance();
        shouldTerminate = false;
        msgId = -1;
        nextConnectionId = 0;
        commandToInt.put("CONNECT", 1);
        commandToInt.put("SEND", 2);
        commandToInt.put("SUBSCRIBE", 3);
        commandToInt.put("UNSUBSCRIBE", 4);
        commandToInt.put("DISCONNECT", 5);
    }

    @Override
    public void start(ConnectionHandler<StompFrame> ch, int connectionId) {

    }

    private int assignMsgId(){
        msgId++;
        return msgId;
    }

    @Override
    public synchronized StompFrame process(StompFrame newFrame, ConnectionHandler<StompFrame> ch) {
        int connectionId = connections.connect(ch);
        System.out.println('\n' + "=== Received ===" + '\n' + newFrame);

        switch (commandToInt.get(newFrame.getCommand())) {
                //CONNECT
            case 1:
                connect(connectionId, newFrame);
                break;

            //SEND
            case 2:
                send(connectionId, newFrame);
                break;

            //SUBSCRIBE
            case 3:
                subscribe(connectionId, newFrame);
                break;

            //UNSUBSCRIBE
            case 4:
                unsubscribe(connectionId, newFrame);
                break;

            //DISCONNECT
            case 5:
                disconnect(connectionId, newFrame);
                break;

            //Invalid command
            default:
                error(connectionId, "Invalid frame", newFrame.getCommand(), newFrame);
                break;
        }
        return null;
    }

    private void connect(int connectionId, StompFrame frame){
        String loginName = frame.getHeaderValue("login");
        String passcode = frame.getHeaderValue("passcode");
        if (frame.getHeaders().containsKey("accept-version")) {
            if (frame.getHeaderValue("accept-version").equals("1.2")) {
                    if(connections.getUserNamePassword().containsKey(loginName)) {
                        if(!connections.getIsLogged().get(loginName)) {
                            if (connections.getUserNamePassword().get(loginName).equals(passcode)) {
                                connections.setActive(loginName, true);
                                connections.setConnectionId(connectionId, loginName);
                                Hashtable<String, String> sendHeaders = new Hashtable<>();
                                sendHeaders.put("version", frame.getHeaderValue("accept-version"));
                                connections.send(connectionId, new StompFrame("CONNECTED", sendHeaders, ""));

                            } else {
                                error(connectionId, "Invalid login or password", "", frame);
                            }
                        }else{
                                error(connectionId, "User already logged in", "", frame);
                            }
                        }
                        else{
                            connections.register(loginName, passcode);
                            connections.setConnectionId(connectionId, loginName);
                            Hashtable<String, String> sendHeaders = new Hashtable<>();
                            sendHeaders.put("version", frame.getHeaderValue("accept-version"));
                            connections.send(connectionId, new StompFrame("CONNECTED", sendHeaders, ""));
                        }
            } else {
                error(connectionId, "Invalid version", "", frame);
            }
        } else {
            error(connectionId, "Missing version", "", frame);
        }

    }

    private void send(int connectionId, StompFrame frame){
        if (frame.getHeaders().containsKey("destination")) {
            if (!frame.getBody().isEmpty()) {
                int subId = connections.getSubId(connectionId, frame.getHeaderValue("destination"));
                if(subId == -1){
                    error(connectionId, "Invalid Send", "You cannot sent message to an unsubscribed channel", frame);
                    return;
                }
                Hashtable<String, String> sendHeaders = new Hashtable<>();
                sendHeaders.put("destination", frame.getHeaderValue("destination"));
                sendHeaders.put("subscription", String.valueOf(subId));
                sendHeaders.put("message-id", String.valueOf(assignMsgId()));
                connections.send(frame.getHeaderValue("destination"), new StompFrame("MESSAGE", sendHeaders, frame.getBody()));
                if(frame.getHeaders().containsKey("receipt-id"))
                    receipt(connectionId, frame);
            } else {
                error(connectionId, "Missing body", "", frame);
            }
        } else {
            error(connectionId, "Missing destination", "", frame);
        }
    }

    private void subscribe(int connectionId, StompFrame frame){
        if (frame.getHeaders().containsKey("destination")) {
            if (frame.getHeaders().containsKey("id")) {
                if(connections.subscribe(connectionId, Integer.parseInt(frame.getHeaderValue("id")), frame.getHeaderValue("destination")))
                    receipt(connectionId, frame);
                else
                    error(connectionId, "Subscription failed", "An error accrued or you are already subscribed to this channel", frame);
            } else {
                error(connectionId, "Missing id", "", frame);
            }
        } else {
            error(connectionId, "Missing destination", "", frame);
        }
    }

    private void unsubscribe(int connectionId, StompFrame frame){
        if (frame.getHeaders().containsKey("id")) {
            if(connections.unsubscribe(connectionId, Integer.parseInt(frame.getHeaderValue("id"))))
                receipt(connectionId, frame);
            else
                error(connectionId, "Invalid subscription id", "", frame);
        } else {
            error(connectionId, "Missing id", "", frame);
        }
    }

    private void disconnect(int connectionId, StompFrame sourceFrame){
        if(sourceFrame.getHeaders().containsKey("receipt")){
            receipt(connectionId, sourceFrame);
            connections.setActive(connectionId, false);
        } else {
            error(connectionId, "Missing receipt", "", sourceFrame);
        }
    }

    private void error(int connectionId, String message, String body, StompFrame sourceFrame){

        Hashtable<String, String> sendHeaders = new Hashtable<>();
        sendHeaders.put("message", message);
        if(sourceFrame.getHeaders().containsKey("receipt-id"))
            sendHeaders.put("receipt-id", message);
        if(sourceFrame.getHeaders().containsKey("message-id"))
            sendHeaders.put("message-id", message);
        connections.send(connectionId, new StompFrame("ERROR", sendHeaders, body));
        //connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    private void receipt(int connectionId, StompFrame sourceFrame){
        Hashtable<String, String> receiptHeaders = new Hashtable<>();
        receiptHeaders.put("receipt-id", sourceFrame.getHeaderValue("receipt"));
        connections.send(connectionId, new StompFrame("RECEIPT", receiptHeaders, ""));
    }

    @Override
    public void start(int connectionId, Connections<StompFrame> connections) {

    }

    @Override
    public boolean shouldTerminate() {
        return this.shouldTerminate;
    }
}
