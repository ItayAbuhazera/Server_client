package bgu.spl.net.impl.stomp;

public enum FrameCommands {
    CONNECT,
    CONNECTED,
    MESSAGE,
    RECEIPT,
    ERROR,
    SEND,
    SUBSCRIBE,
    UNSUBSCRIBE,
    DISCONNECT;

    public static FrameCommands fromString(String string) {
        try {
            return FrameCommands.valueOf(string.toUpperCase());
        } catch (IllegalArgumentException ex) {
            return null;
        }
    }
}
