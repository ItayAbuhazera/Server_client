#pragma once

#include <mutex>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include <queue>

class KeyboardThread {
public:
    KeyboardThread(ConnectionHandler& ch,StompProtocol& protocol);
    void run();
    KeyboardThread(const KeyboardThread& kt);
    KeyboardThread & operator=(const KeyboardThread &kt);
    void clearSendQueue();
    static void sendFrame(StompFrame* frame);
    ~KeyboardThread();

private:
    ConnectionHandler* mConnectionHandler;
    StompProtocol* mProtocol;
    static std::queue<StompFrame*> sendQueue;
};