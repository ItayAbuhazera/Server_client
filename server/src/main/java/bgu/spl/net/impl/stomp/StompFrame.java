package bgu.spl.net.impl.stomp;

import java.util.Hashtable;

//class for the frame of the stomp protocol
public class StompFrame {
    private final FrameCommands command;
    private Hashtable<String, String> headers;
    private String body;

    public StompFrame(FrameCommands command, Hashtable<String, String> headers, String body){
        this.command = command;
        this.headers = new Hashtable<>(headers);
        this.body = body;
    }

    public StompFrame(FrameCommands command){
        this.command = command;
        this.headers = new Hashtable<>();
        this.body = "";
    }

    public StompFrame(String input) {
        headers = new Hashtable<>();

        //command
        int commandIndex = input.indexOf('\n');
        command = FrameCommands.fromString(input.substring(0, commandIndex));
        input = input.substring(commandIndex + 1);

        //headers
        String headersString = input.substring(0, input.indexOf("\n\n"));
        String[] lines = headersString.split("\n");
        for (String line : lines) {
            String[] parts = line.split(":");
            if (parts.length == 2) {
                headers.put(parts[0], parts[1]);
            }
        }

        //body
        body = input.substring(input.indexOf("\n\n") + 2);
    }

    public void addHeader(String key, String value){
        headers.put(key, value);
    }

    public void setBody(String body){
        this.body = body;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (String key : headers.keySet()) {
            String value = headers.get(key);
            sb.append(key + ":" + value);
            sb.append("\n");
        }

        return command + "\n" + sb + "\n" + body + "\n";
    }

    public String getHeaderValue(String header){
        return headers.get(header);
    }

    public String getBody() {
        return body;
    }

    public FrameCommands getCommand() {
        return command;
    }
}