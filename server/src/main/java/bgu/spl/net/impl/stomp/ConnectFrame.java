package bgu.spl.net.impl.stomp;

import java.util.Hashtable;
import java.util.Map;

public class ConnectFrame extends StompFrame {
    private Hashtable<String, String> headers;
    private String body;

    public ConnectFrame(Hashtable<String, String> headers, String body) {
        super("CONNECT", headers, body);
        this.headers = headers;
        this.body = body;
    }


    @Override
    public Hashtable<String, String> getHeaders() {
        return headers;
    }

    @Override
    public String getBody() {
        return body;
    }
}
