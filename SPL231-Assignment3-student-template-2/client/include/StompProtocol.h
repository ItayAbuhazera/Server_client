#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"
#include "../include/event.h"

enum Type {disconnect, subscribe, unsubscribe};
enum Command {login, join, unjoin, logout, sendMessage, report, summary};
enum Frame {message, receipt, connected, error};

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol(ConnectionHandler& ch);
    StompProtocol(const StompProtocol& protocol);
    const StompProtocol& operator=(const StompProtocol& protocol);
    void processFrame(StompFrame newFrame);
    vector<string> processKeyboard(string msg);
    bool validateCommand(vector<string> command);

private:
    void initCommands();
    vector<string> tokenize(string source, char delimiter);
    string login(vector<string> msg);
    string logout();
    string send(const string& dest, const string& body);
    vector<string> report(const string& file);
    string extractUser(const string& frameBody);

    int mDisconnectRec;
    int mReceiptCounter;
    int mSubId;
    string user;
    ConnectionHandler* mConnectionHandler;
    map<string, int> commands;
    map<string, int> subscriptions;
    map<int, tuple<Type, int, string>> excpectedReciepts;
    map<tuple<string, string>, map<int, Event*>> allReports; //(topic, user) tuple to a map of (time : event)
};
