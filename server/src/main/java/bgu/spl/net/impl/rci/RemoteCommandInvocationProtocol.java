package bgu.spl.net.impl.rci;

import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.srv.ConnectionHandler;

import java.io.Serializable;

public class RemoteCommandInvocationProtocol<T> implements MessagingProtocol<Serializable> {

    private T arg;

    public RemoteCommandInvocationProtocol(T arg) {
        this.arg = arg;
    }

    public Serializable process(Serializable msg, ConnectionHandler<Serializable> ch) {
        return ((Command) msg).execute(arg);
    }

    public Serializable process(Serializable msg) {
        return null;
    }

    @Override
    public boolean shouldTerminate() {
        return false;
    }

}
