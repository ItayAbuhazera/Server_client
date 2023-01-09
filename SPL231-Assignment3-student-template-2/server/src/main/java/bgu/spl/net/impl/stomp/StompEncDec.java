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
        String[] messageParts = message.split("\n");
        String command = messageParts[0];
        Hashtable<String,String> headers = new Hashtable<>();
        for (int i = 1; i < messageParts.length; i++) {
            if(messageParts[i].equals("\n")){
                break;
            }
            String[] header = messageParts[i].split(":");
            headers.put(header[0],header[1]);
        }
        StringBuilder body = new StringBuilder();
        for (int i = messageParts.length-2; i >= 0; i--) {
            if(messageParts[i].equals("\n")){
                break;
            }
            body.insert(0, messageParts[i]);
        }
        return new StompFrame(command, headers, body.toString());
    }

    @Override
    public byte[] encode(StompFrame message) {
        return (message + "\n").getBytes(); //uses utf8 by default
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
