#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

using namespace std;

enum class FrameCommand {
    CONNECT,
    CONNECTED,
    SUBSCRIBE,
    UNSUBSCRIBE,
    SEND,
    MESSAGE,
    RECEIPT,
    ERROR,
    DISCONNECT
};

class StompFrame
{
    public:
        StompFrame(string msg);
        StompFrame(FrameCommand command);
        StompFrame(FrameCommand command, map<string, string> headers, string body);
        void addHeader(const string& header, const string& value);
        void setBody(const string& body);
        const FrameCommand& getCommand() const;
        const string& getHeaderValue(const string& header) const;
        const string& getBody() const;
        string toString() const;
        void printFrame(bool includeHeaders) const;

        static FrameCommand stringToCommand(string str);
        static string commandToString(FrameCommand command);

    private:
        FrameCommand mCommand;
        map<string, string> mHeaders;
        string mBody;
};