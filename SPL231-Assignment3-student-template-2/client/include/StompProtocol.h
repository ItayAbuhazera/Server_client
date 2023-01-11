#pragma once

#include "../include/ConnectionHandler.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol(ConnectionHandler &ch);
    string processFrame(string msg);
    string processKeyboard(string msg);

private:
    int disconnectRec;
    int receiptCounter;
    int subId;
    ConnectionHandler* ch;
};
