package bgu.spl.net.srv;

public interface Connections<T> {

    //sends a message T to all clients subscribed to channel
    void send(String channel, T msg);

    //subscribes the client represented by connectionId to the channel
    boolean subscribe(int connectionId, int subId, String channel);

    //unsubscribes the client represented by connectionId from the channel
    boolean unsubscribe(int connectionId, int subId);

    //assigns a new connectionId to the given handler
    int connect(ConnectionHandler<T> handler);

    //returns the subscriptionId of the given connectionId to the channel, or -1 if the client isn't subscribed to the channel
    int getSubId(int connectionId, String channel);

    //checks if the given login exists in the system
    boolean checkLogin(String login);

    //checks if the given password matches the login
    boolean checkPassword(String login, String passcode);

}
