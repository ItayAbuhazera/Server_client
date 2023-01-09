package bgu.spl.net.impl.stomp;

import java.util.Hashtable;
import java.util.Map;

import bgu.spl.net.api.MessageEncoderDecoder;

//class for the frame of the stomp protocol
public class StompFrame {
    private String command;
    private Hashtable<String, String> headers;
    private String body;

    public StompFrame(String command, Hashtable<String, String> headers, String body){
        this.command = command;
        this.headers = new Hashtable<String, String>(headers);
        this.body = body;
    }
    public String toString() {
        StringBuilder header = new StringBuilder();
        for (Map.Entry<String, String> entry : headers.entrySet()) {
            String key = entry.getKey();
            String value = entry.getValue();
            header.append(key).append(":").append(value).append("\n");
        }
        return command + "\n" + header + "\n" + body + "\n" + "\u0000";
    }
    public Hashtable<String, String> getHeaders() {
        return headers;
    }

    public String getHeaderValue(String header){
        return headers.get(header);
    }

    public String getBody() {
        return body;
    }

    public String getCommand() {
        return command;
    }
}