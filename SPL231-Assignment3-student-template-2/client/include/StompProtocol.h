#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"

enum Type {disconnect, subscribe, unsubscribe};

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol(ConnectionHandler& ch);
    StompProtocol(const StompProtocol& protocol);
    const StompProtocol& operator=(const StompProtocol& protocol);
    void processFrame(StompFrame newFrame);
    string processKeyboard(string msg);
    bool validateCommand(vector<string> command);

private:
    void initCommands();
    vector<string> tokenize(string source, char delimiter);
    string login(vector<string> msg);
    string logout();
    string send(const string& dest, const string& body);
    string report(const string& file);

    int mDisconnectRec;
    int mReceiptCounter;
    int mSubId;
    ConnectionHandler* mConnectionHandler;
    map<string, int> commands;
    map<string, int> subscriptions;
    map<int, tuple<Type, int, string>> excpectedReciepts;
};
