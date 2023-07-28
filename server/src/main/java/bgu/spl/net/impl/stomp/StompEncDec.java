package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.MessageEncoderDecoder;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Hashtable;

public class StompEncDec implements MessageEncoderDecoder<StompFrame> {
    private byte[] bytes = new byte[1 << 10]; //start with 1k
    private int len = 0;

    @Override
    public StompFrame decodeNextByte(byte nextByte) {
        //notice that the top 128 ascii characters have the same representation as their utf-8 counterparts
        //this allows us to do the following comparison
        if (nextByte ==  '\u0000') {
            return stringToFrame(popString());
        }
        pushByte(nextByte);
        return null; //not a frame yet
    }

    private StompFrame stringToFrame(String message){
        String body, command;
        Hashtable<String, String> headers = new Hashtable<>();

        //command
        int commandIndex = message.indexOf('\n');
        command = message.substring(0, commandIndex);
        message = message.substring(commandIndex + 1);

        //headers
        int pos = 0;
        String headersString = message.substring(0, message.indexOf("\n\n"));
        String[] lines = headersString.split("\n");
        for (String line : lines) {
            String[] parts = line.split(":");
            if (parts.length == 2) {
                headers.put(parts[0], parts[1]);
            }
        }

        //body
        body = message.substring(message.indexOf("\n\n") + 2);

        return new StompFrame(command, headers, body);
    }

    @Override
    public byte[] encode(StompFrame message) {
        return (message.toString()).getBytes(); //uses utf8 by default
    }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, len * 2);
        }

        bytes[len++] = nextByte;
    }

    private String popString() {
        //notice that we're explicitly requesting that the string will be decoded from UTF-8
        //this is not actually required as it is the default encoding in java.
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return result;
    }
}
