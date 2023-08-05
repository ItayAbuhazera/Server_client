package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {
    private static ConnectionsImpl<StompFrame> instance;
    private volatile ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private volatile ConcurrentHashMap<String, String> userNamePassword;

    // ConnectionID : <SubId : Channel>
    private volatile ConcurrentHashMap<Integer, ConcurrentHashMap<Integer, String>> subscriptionsIds;

    private volatile ConcurrentHashMap<String, Boolean> isLogged;

    private volatile ConcurrentHashMap<Integer, String> connectionIdToUserName;

    private static int nextId;
    private boolean shouldTerminate = false;

    private ConnectionsImpl() {
        this.subscriptionsIds = new ConcurrentHashMap<>();
        this.connections = new ConcurrentHashMap<>();
        this.userNamePassword = new ConcurrentHashMap<>();
        this.isLogged = new ConcurrentHashMap<>();
        this.connectionIdToUserName = new ConcurrentHashMap<>();
        this.nextId = 0;
    }

    public static ConnectionsImpl<StompFrame> getInstance(){
        if(instance == null)
            instance = new ConnectionsImpl<StompFrame>();
        return instance;
    }

    @Override
    public synchronized int getSubId(int connectionId, String channel){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);

        if(subs == null)
            return -1;

        for (Integer key : subs.keySet()) {
            if(subs.get(key).equals(channel))
                return key;
        }
        return -1;
    }

    @Override
    public synchronized boolean checkLogin(String login){
        return userNamePassword.containsKey(login);
    }

    @Override
    public synchronized boolean checkPassword(String login, String passcode){
        return userNamePassword.get(login).equals(passcode);
    }

    @Override
    public synchronized boolean subscribe(int connectionId, int subId, String channel){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);

        if(subs == null)
            subs = new ConcurrentHashMap<>();

        if(subs.containsValue(channel))
            return false;
        subs.put(subId, channel);
        subscriptionsIds.put(connectionId, subs);
        return true;
    }

    @Override
    public synchronized boolean unsubscribe(int connectionId, int subId){
        ConcurrentHashMap<Integer, String> subs = subscriptionsIds.get(connectionId);
        if(subs == null || !subs.containsKey(subId))
            return false;
        subs.remove(subId);
        subscriptionsIds.put(connectionId, subs);
        return true;
    }

    private synchronized boolean send(int connectionId, T msg) {
        ConnectionHandler<T> client = connections.get(connectionId);
        System.out.println('\n' + "=== Sent ===" + '\n' + msg);
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
    public synchronized int connect(ConnectionHandler<T> ch) {
        for (Integer key : connections.keySet()) {
            ConnectionHandler<T> value = connections.get(key);
            if(value.hashCode() == ch.hashCode())
                return key;
        }
        nextId++;
        connections.put(nextId, ch);
        return nextId;
    }

    public synchronized void register(String login, String passcode) {
        userNamePassword.put(login, passcode);
        isLogged.put(login, true);
    }

    public void setActive(int connectionId, boolean b) {
        try {
            String loginName = connectionIdToUserName.get(connectionId);
            isLogged.put(loginName, b);
        } catch (NullPointerException e) {}
    }

    public void setActive(ConnectionHandler<T> ch, boolean b) {
        int id = -1;
        for(int k : connections.keySet())
            if(connections.get(k).hashCode() == ch.hashCode())
                id = k;

        if(id == -1)
            return;
        setActive(id, b);
    }
}

