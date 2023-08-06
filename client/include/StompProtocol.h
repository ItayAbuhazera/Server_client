#pragma once

//#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <sstream>

#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"
#include "../include/event.h"
#include "../include/json.hpp"

enum ReceiptType {disconnect, subscribe, unsubscribe};
enum ClientCommand {invalid, login, join, unjoin, logout, sendMsg, report, summary};
enum Frame {message, receipt, connected, error};

class StompProtocol {
    public:
        StompProtocol(ConnectionHandler& ch);
        StompProtocol(const StompProtocol& protocol);
        const StompProtocol& operator=(const StompProtocol& protocol);
        void processFrame(StompFrame newFrame);
        StompFrame* processKeyboard(string msg);

    private:
        //Client commands
        StompFrame* login(vector<string> msg);
        StompFrame* logout();
        StompFrame* subscribe(string destination);
        StompFrame* unsubscribe(string destination, int subId);
        StompFrame* send(const string& dest, const string& body);
        vector<StompFrame*> report(const string& file);

        //Incoming frames
        void disconnect();
        void message(const StompFrame& frame);
        void handleReceipt(const StompFrame& frame);

        //Helper functions
        vector<string> tokenize(string source, char delimiter);
        string extractUser(const string& frameBody);
        void parseToFile(const map<int, Event*>& reports, const string& file);

        //Validate Commands
        bool validateCommand(vector<string> command);
        bool validate(vector<string> command, std::size_t excpected, string structure);
        static ClientCommand stringToCommand(string str);

        int mDisconnectRec;
        int mReceiptCounter;
        int mSubId;
        string user;
        ConnectionHandler* mConnectionHandler;
        map<string, int> commands;
        map<string, int> subscriptions;
        map<int, tuple<ReceiptType, int, string>> excpectedReciepts;
        map<tuple<string, string>, map<int, Event*>> allReports; //(topic, user) tuple to a map of (time : event)
};
