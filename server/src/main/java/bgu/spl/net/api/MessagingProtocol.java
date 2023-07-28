package bgu.spl.net.api;

import bgu.spl.net.srv.ConnectionHandler;

import java.io.Serializable;

public interface MessagingProtocol<T> {
 
    /**
     * process the given message 
     * @param msg the received message
     * @return the response to send or null if no response is expected by the client
     */
    T process(T msg, ConnectionHandler<T> ch);


    /**
     * @return true if the connection should be terminated
     */
    boolean shouldTerminate();

    void start(ConnectionHandler<T> ch, int connectionId);
 
}