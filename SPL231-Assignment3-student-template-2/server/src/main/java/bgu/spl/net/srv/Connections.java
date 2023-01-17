package bgu.spl.net.srv;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    void subscribe(int connectionId, String channel);

    void unsubscribe(int connectionId, String channel);

    void unsubscribeAll(int connectionId);

    int connect(ConnectionHandler<T> handler);

    ConcurrentHashMap<String, String> getUserNamePassword();
}
