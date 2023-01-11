#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include "../include/ConnectionHandler.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
public:
    StompProtocol(ConnectionHandler &ch);
    string processFrame(string msg);
    string processKeyboard(string msg);

private:
    void initCommands();
    string logout(vector<string> msg);
    string message(string msg);
    void receipt(vector<string> msg);
    void error(vector<string> msg);
    string login(vector<string> msg);
    string join(vector<string> msg);
    string report(vector<string> msg);
    string send(string msg, string topic);
    string look4Header (string header,vector <string> msg);
    void addToSubscription(string topic,int id);
    void removeSubscription(string topic);
    string exit(vector<string> msg);
    int getSubId(string topic);
    void addToReceipts (int recId, string topic, bool action);
    unordered_map<int,pair<string,bool >> getReceipts ();
    int mDisconnectRec;
    int mReceiptCounter;
    int mSubId;
    ConnectionHandler* mConnectionHandler;
    map<string, int> commands;
    unordered_map <string, int> subscriptionsByTopic; 
    unordered_map <int, string> subscriptionsById;
    unordered_map <int,pair<string, bool>> receipts;

};
