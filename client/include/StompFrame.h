#pragma once

#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class StompFrame
{
    public:
        StompFrame(string msg);
        StompFrame(string command, map<string, string> headers, string body);
        const string& getCommand() const;
        const string& getHeaderValue(const string& header) const;
        const string& getBody() const;
        void printFrame(bool includeHeaders) const;

    private:
        string mCommand;
        map<string, string> mHeaders;
        string mBody;
};