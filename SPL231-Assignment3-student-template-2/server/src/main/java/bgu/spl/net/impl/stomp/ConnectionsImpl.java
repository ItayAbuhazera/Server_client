package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {
    private volatile ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private volatile ConcurrentHashMap<String, String> userNamePassword;

    // ConnectionID : <SubId : Channel>
    private volatile ConcurrentHashMap<Integer, ConcurrentHashMap<Integer, String>> subscriptionsIds;

    private int nextId;
    private boolean shouldTerminate = false;

    public ConnectionsImpl() {
        this.subscriptionsIds = new ConcurrentHashMap<>();
        this.connections = new ConcurrentHashMap<>();
        this.userNamePassword = new ConcurrentHashMap<>();
        this.nextId = 0;
    }

    public int getSubId(int connectionId, String channel){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);
        for (Integer key : subs.keySet()) {
            if(subs.get(key) == channel)
                return key;
        }
        return -1;
    }


    @Override
    public ConcurrentHashMap<String, String> getUserNamePassword() {
        return userNamePassword;
    }

    @Override
    public synchronized boolean subscribe(int connectionId, int subId, String channel){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);
        if(subs == null)
            subs = new ConcurrentHashMap<>();
        else if(subs.containsValue(channel)){
            subscriptionsIds.forEach((key, value) -> System.out.println(key + " : " + value.values()));
            return false;
        }
        subs.put(subId, channel);
        subscriptionsIds.put(connectionId, subs);
        subscriptionsIds.forEach((key, value) -> System.out.println(key + " : " + value.values()));
        return true;
    }

    @Override
    public synchronized boolean unsubscribe(int connectionId, int subId){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);
        if(subs == null)
            return false;
        String unsub = subs.remove(subId);
        subscriptionsIds.put(connectionId, subs);
        subscriptionsIds.forEach((key, value) -> System.out.println(key + " : " + value.values()));
        return true;
    }

    @Override
    public synchronized boolean send(int connectionId, T msg) {
        ConnectionHandler<T> client = connections.get(connectionId);
        System.out.println("=== Sent ===");
        System.out.println(msg);
        if(client != null)
            synchronized (client){
                client.send(msg);
                return true;
            }
        return false;
    }

    @Override
    public synchronized void send(String channel, T msg) {
        for (Integer key : subscriptionsIds.keySet()) {
            if(subscriptionsIds.get(key).containsValue(channel)){
                send(key, msg);
            }
        }
    }

    @Override
    public synchronized void disconnect(int connectionId) {

    }

    @Override
    public synchronized void unsubscribeAll(int connectionId){
        /*
        if(SubscriberToChannels.get(connectionId) == null)
            return;
        else{
            for (String channel: SubscriberToChannels.get(connectionId)) {
                unsubscribe(connectionId, channel);
            }
        }*/
    }

    @Override
    public synchronized int connect(ConnectionHandler<T> ch) {
        for (Integer key : connections.keySet()) {
            ConnectionHandler<T> value = connections.get(key);
            if(value == ch)
                return key;
        }
        nextId++;
        connections.put(nextId, ch);
        return nextId;
    }

    public synchronized void register(String login, String passcode) {
        userNamePassword.put(login, passcode);
    }

}

