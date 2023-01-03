package bgu.spl.net.impl.stomp;

import java.util.Hashtable;

import bgu.spl.net.api.MessageEncoderDecoder;

//class for the frame of the stomp protocol
public class StompFrame {
    private String frameCommand;
    private Hashtable<String, String> frameHeaders;
    private String frameBody;
    final String endOfFrame = "\u0000";

    public StompFrame(String frameCommand, Hashtable<String, String> frameHeaders, String frameBody) {
        this.frameCommand = frameCommand;
        this.frameHeaders = frameHeaders;
        this.frameBody = frameBody;
    }

    public StompFrame(String[] message) {
        this.frameCommand = message[0];
        this.frameHeaders = new Hashtable<>();
        this.frameBody = "";

        if (!message[message.length - 1].equals(endOfFrame))
            throw new RuntimeException("Error: frame is not ended with " + endOfFrame);

        for (int i = 1; i < message.length - 1; i++) { // i = 1 because the first line is the command
            if (message[i].contains(":")) { // if the line contains a header
                String[] header = message[i].split(":"); // split the line to header and value
                this.frameHeaders.put(header[0], header[1]); // add the header to the frame
            } else if (message[i] != "\n") { // if the line is not empty
                this.frameBody = message[i]; // add the line to the frame body
            }
        }
    }
}