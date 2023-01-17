#include "../include/StompFrame.h"

using namespace std;


StompFrame::StompFrame(string msg): mCommand(msg.substr(0, msg.find('\n'))), mHeaders(), mBody() {
    msg = msg.substr(msg.find('\n') + 1);

    //headers
    while(msg.substr(0, 1) != "\n"){
        string key, val;
        key = msg.substr(0, msg.find(':'));
        val = msg.substr(msg.find(':') + 1, msg.find('\n'));
        mHeaders[key] = val;
        msg = msg.substr(msg.find('\n') + 1);
    }

    //body
    mBody = msg.substr(0, msg.find('\0'));
}

StompFrame::StompFrame(string command, map<string, string> headers, string body): mCommand{command}, mHeaders{headers}, mBody{body} {};

const string& StompFrame::getCommand() const{
    return mCommand;
}

const string& StompFrame::getHeaderValue(const string& header) const{
    return mHeaders.at(header);
}

const string& StompFrame::getBody() const{
    return mBody;
}

void StompFrame::printFrame(bool includeHeaders) const {
    std::cout << mCommand << std::endl;
    if(includeHeaders)
        for (auto const &pair: mHeaders)
            std::cout << pair.first << ":" << pair.second << std::endl;
    std::cout << mBody << std::endl;
}

