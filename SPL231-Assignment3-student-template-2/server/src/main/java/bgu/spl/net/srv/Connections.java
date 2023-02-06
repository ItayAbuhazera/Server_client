package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    boolean subscribe(int connectionId, int subId, String channel);

    boolean unsubscribe(int connectionId, int subId);

    void unsubscribeAll(int connectionId);

    int connect(ConnectionHandler<T> handler);

    ConcurrentHashMap<String, String> getUserNamePassword();
}
