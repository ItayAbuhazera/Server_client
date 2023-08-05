package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;

import java.util.Hashtable;

public class StompProtocol implements StompMessagingProtocol<StompFrame> {
    private final ConnectionsImpl<StompFrame> connections;
    private boolean shouldTerminate;
    private static int msgId;

    public StompProtocol(){
        connections = ConnectionsImpl.getInstance();
        shouldTerminate = false;
        msgId = -1;
    }

    private int assignMsgId(){
        msgId++;
        return msgId;
    }

    private StompFrame createConnectedFrame(){
        Hashtable<String, String> headers = new Hashtable<>();
        headers.put("version", "1.2");
        return new StompFrame(FrameCommands.CONNECTED, headers, "");
    }

    private StompFrame createErrorFrame(String message, String body, String receiptId, String messageId) {
        Hashtable<String, String> sendHeaders = new Hashtable<>();
        sendHeaders.put("message", message);
        if (receiptId != null)
            sendHeaders.put("receipt-id", receiptId);
        if (messageId != null)
            sendHeaders.put("message-id", messageId);
        return new StompFrame(FrameCommands.ERROR, sendHeaders, body);
    }

    private StompFrame createErrorFrame(String message, String body, String receiptId) {
        return createErrorFrame(message, body, receiptId, null);
    }

    private StompFrame createErrorFrame(String message, String body) {
        return createErrorFrame(message, body, null, null);
    }

    private StompFrame createReceiptFrame(String receiptId){
        Hashtable<String, String> receiptHeaders = new Hashtable<>();
        receiptHeaders.put("receipt-id", String.valueOf(receiptId));
        return new StompFrame(FrameCommands.RECEIPT, receiptHeaders, "");
    }

    @Override
    public StompFrame process(StompFrame newFrame, ConnectionHandler<StompFrame> ch) {
        int connectionId = connections.connect(ch);
        String receiptId = newFrame.getHeaderValue("receipt");
        String destination = newFrame.getHeaderValue("destination");
        int subId = connections.getSubId(connectionId, destination);
        System.out.println('\n' + "=== Received ===" + '\n' + newFrame + '\n');

        //Handle incoming frame
        switch (newFrame.getCommand()) {
            case CONNECT:
                String loginName = newFrame.getHeaderValue("login");
                String passcode = newFrame.getHeaderValue("passcode");
                String version = newFrame.getHeaderValue("accept-version");
                return connect(loginName, passcode, version);
            case SEND:
                String body = newFrame.getBody();
                return send(destination, subId, body, receiptId);
            case SUBSCRIBE:
                return subscribe(connectionId, destination, subId, receiptId);
            case UNSUBSCRIBE:
                return unsubscribe(connectionId, subId, receiptId);
            case DISCONNECT:
                return disconnect(connectionId, receiptId);
            default:
                // ERROR : invalid frame command
                shouldTerminate = true;
                return createErrorFrame("Invalid frame command", "");
        }
    }

    private StompFrame connect(String loginName, String passcode, String version){
        if (loginName == null || passcode == null || version == null) {
            // ERROR : missing headers
            //connections.send(connectionId, createErrorFrame("Invalid CONNECT frame", "Missing login, passcode or version"));
            shouldTerminate = true;
            return createErrorFrame("Invalid CONNECT frame", "Missing login, passcode or version");
        }

        if(connections.checkLogin(loginName)) {
            if (connections.checkPassword(loginName, passcode)) {
                //connections.send(connectionId, createConnectedFrame());
                System.out.println(loginName + " logged in");
                return createConnectedFrame();
            } else {
                //ERROR : wrong password
                //connections.send(connectionId, createErrorFrame("Invalid password", ""));
                shouldTerminate = true;
                return createErrorFrame("Invalid password", "");
            }
        } else {
            //login doesn't exist => create new user
            connections.register(loginName, passcode);
            //connections.send(connectionId, createConnectedFrame());
            System.out.println(loginName + " registered");
            return createConnectedFrame();
        }
    }

    private StompFrame send(String destination, int subId, String body, String receiptId){
        if (destination == null) {
            // ERROR : missing destination
            shouldTerminate = true;
            return createErrorFrame("Missing destination", "");
        }

        if (subId == -1) {
            // ERROR : user is not subscribed to this channel
            shouldTerminate = true;
            return createErrorFrame("User is not subscribed to this channel", "");
        }

        if (!body.isEmpty()) {
            //Send MESSAGE frame
            Hashtable<String, String> sendHeaders = new Hashtable<>();
            sendHeaders.put("destination", destination);
            sendHeaders.put("subscription", String.valueOf(subId));
            sendHeaders.put("message-id", String.valueOf(assignMsgId()));
            connections.send(destination, new StompFrame(FrameCommands.MESSAGE, sendHeaders, body));

            //Send RECEIPT frame if needed
            if(receiptId != null)
                return createReceiptFrame(receiptId);
            return null;
        } else {
            // ERROR : missing body
            return createErrorFrame("Missing body", "");
        }
    }

    private StompFrame subscribe(int connectionId, String destination, int subId, String receiptId){
        if (destination == null) {
            // ERROR : missing destination
            shouldTerminate = true;
            return createErrorFrame("Invalid SUBSCRIBE frame", "Missing destination");
        }

        if (subId == -1) {
            // ERROR : missing id
            shouldTerminate = true;
            return createErrorFrame("Invalid SUBSCRIBE frame", "Missing id");
        }

        if (connections.subscribe(connectionId, subId, destination)) {
            //Send RECEIPT frame if needed
            if (receiptId != null)
                return createReceiptFrame(receiptId);
            return null;
        } else {
            // ERROR : user is already subscribed to this channel
            return createErrorFrame("Subscription failed", "User is already subscribed to this channel");
        }
    }

    private StompFrame unsubscribe(int connectionId, int subId, String receiptId){
        if (subId == -1) {
            // ERROR : missing id
            shouldTerminate = true;
            return createErrorFrame("Invalid UNSUBSCRIBE frame", "Missing id");
        }

        if (connections.unsubscribe(connectionId, subId)) {
            //Send RECEIPT frame if needed
            if (receiptId != null)
                return createReceiptFrame(receiptId);
            return null;
        } else {
            // ERROR : user is not subscribed to this channel
            return createErrorFrame("Unsubscription failed", "User is not subscribed to this channel");
        }
    }

    private StompFrame disconnect(int connectionId, String receiptId){
        if (receiptId != null){
            connections.setActive(connectionId, false);
            return createReceiptFrame(receiptId);
        } else {
            // ERROR : missing receipt id
            shouldTerminate = true;
            return createErrorFrame("Invalid DISCONNECT frame", "Missing receipt id");
        }
    }

    @Override
    public boolean shouldTerminate() {
        return this.shouldTerminate;
    }
}
