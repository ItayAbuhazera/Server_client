#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol(ConnectionHandler& ch);
    StompProtocol(const StompProtocol& protocol);
    const StompProtocol& operator=(const StompProtocol& protocol);
    bool processFrame(StompFrame newFrame);
    string processKeyboard(string msg);
    bool validateCommand(vector<string> command);

private:
    void initCommands();
    vector<string> tokenize(string source, char delimiter);
    string logout(vector<string> msg);
    string login(vector<string> msg);

    int mDisconnectRec;
    int mReceiptCounter;
    int mSubId;
    ConnectionHandler* mConnectionHandler;
    map<string, int> commands;
};
