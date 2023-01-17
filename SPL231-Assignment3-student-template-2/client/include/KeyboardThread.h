#pragma once

#include <mutex>
#include "ConnectionHandler.h"
#include "StompProtocol.h"

class KeyboardThread {
public:
    KeyboardThread(ConnectionHandler& ch,StompProtocol& protocol);
    void run();
    KeyboardThread(const KeyboardThread& kt);
    KeyboardThread & operator=(const KeyboardThread &kt);
    ~KeyboardThread();

    void terminate();

private:
    ConnectionHandler* mConnectionHandler;
    StompProtocol* mProtocol;
    bool shouldTerminate;
};