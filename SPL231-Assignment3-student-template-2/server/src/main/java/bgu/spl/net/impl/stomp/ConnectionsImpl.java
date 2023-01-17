package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {
    public ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String, ArrayList<ConnectionHandler<T>>> channelToSubscribers;
    private ConcurrentHashMap<ConnectionHandler<T>, ArrayList<String>> SubscriberToChannels;
    private ConcurrentHashMap<String, String> userNamePassword;

    private int nextId;
    private boolean shouldTerminate = false;

    public ConnectionsImpl() {
        this.connections = new ConcurrentHashMap<>();
        this.channelToSubscribers = new ConcurrentHashMap<>();
        this.SubscriberToChannels = new ConcurrentHashMap<>();
        this.userNamePassword = new ConcurrentHashMap<>();
        this.nextId = 0;
    }


    @Override
    public ConcurrentHashMap<String, String> getUserNamePassword() {
        return userNamePassword;
    }

    @Override
    public void subscribe(int connectionId, String channel){
        ArrayList<ConnectionHandler<T>> subs = channelToSubscribers.get(channel);
        if(subs == null)
            subs = new ArrayList<ConnectionHandler<T>>();
        subs.add(connections.get((connectionId)));
        SubscriberToChannels.putIfAbsent(connections.get(connectionId), new ArrayList<>());
        SubscriberToChannels.get(connectionId).add(channel);
        channelToSubscribers.put(channel, subs);
    }

    @Override
    public void unsubscribe(int connectionId, String channel){
        ArrayList<ConnectionHandler<T>> subs = channelToSubscribers.get(channel);
        if(subs == null)
            return;
        subs.remove(connectionId);
        channelToSubscribers.put(channel, subs);

        ArrayList<String> subChannels = SubscriberToChannels.get(connectionId);
        if(subChannels == null)
            return;
        subChannels.remove(channel);
        SubscriberToChannels.put(connections.get(connectionId), subChannels);
    }

    @Override
    public boolean send(int connectionId, T msg) {
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
    public void send(String channel, T msg) {
        List<ConnectionHandler<T>> users = channelToSubscribers.get(channel);
        if(users == null)
            return;
        for (ConnectionHandler<T> user: users) {
            user.send(msg);
        }
    }

    @Override
    public void disconnect(int connectionId) {
        try {
            connections.get(connectionId).close();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    @Override
    public void unsubscribeAll(int connectionId){
        if(SubscriberToChannels.get(connectionId) == null)
            return;
        else{
            for (String channel: SubscriberToChannels.get(connectionId)) {
                unsubscribe(connectionId, channel);
            }
        }
    }

    @Override
    public int connect(ConnectionHandler<T> ch) {
        for (Integer key : connections.keySet()) {
            ConnectionHandler<T> value = connections.get(key);
            if(value == ch)
                return key;
        }
        connections.put(nextId, ch);
        nextId++;
        return nextId - 1;
    }

    public void register(String login, String passcode) {
        userNamePassword.put(login, passcode);
    }
}

