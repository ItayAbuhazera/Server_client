package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> connections;
    private ConcurrentHashMap<String, ArrayList<Integer>> channels;
    private ConcurrentHashMap<Integer, ArrayList<String>> subscribedChannels;
    private ConcurrentHashMap<String, String> userNamePassword;

    private int nextId;
    private boolean shouldTerminate = false;

    public ConnectionsImpl(ConcurrentHashMap<Integer,ConnectionHandler<T>> connections) {
        this.connections = connections;
        this.channels = new ConcurrentHashMap<>();
        this.subscribedChannels = new ConcurrentHashMap<>();
        this.nextId = 0;
    }

    public void connect(ConnectionHandler<T> handler) {
        connections.put(nextId, handler);
        nextId++;
    }

    @Override
    public ConcurrentHashMap<String, String> getUserNamePassword() {
        return userNamePassword;
    }

    @Override
    public void subscribe(int connectionId, String channel){
        ArrayList<Integer> subs = channels.get(channel);
        if(subs == null)
            subs = new ArrayList<Integer>();
        subs.add(connectionId);
        subscribedChannels.putIfAbsent(connectionId, new ArrayList<>());
        subscribedChannels.get(connectionId).add(channel);
        channels.put(channel, subs);
    }

    @Override
    public void unsubscribe(int connectionId, String channel){
        ArrayList<Integer> subs = channels.get(channel);
        if(subs == null)
            return;
        subs.remove(connectionId);
        channels.put(channel, subs);

        ArrayList<String> subChannels = subscribedChannels.get(connectionId);
        if(subChannels == null)
            return;
        subChannels.remove(channel);
        subscribedChannels.put(connectionId, subChannels);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> client = connections.get(connectionId);
        if(client != null)
            synchronized (client){
                client.send(msg);
                return true;
            }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> users = channels.get(channel);
        if(users == null)
            return;
        for (Integer user: users) {
            send(user, msg);
        }
    }

    @Override
    public void disconnect(int connectionId) {
        connections.remove(connectionId);

    }

    @Override
    public void unsubscribeAll(int connectionId){
        if(subscribedChannels.get(connectionId) == null)
            return;
        else{
            for (String channel: subscribedChannels.get(connectionId)) {
                unsubscribe(connectionId, channel);
            }
        }
    }
}

