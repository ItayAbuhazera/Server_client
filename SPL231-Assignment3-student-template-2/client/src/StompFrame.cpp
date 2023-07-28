#include "../include/StompFrame.h"

using namespace std;


StompFrame::StompFrame(string msg): mCommand(msg.substr(0, msg.find('\n'))), mHeaders(), mBody() {
    msg = msg.substr(msg.find('\n') + 1);

    //headers
    string::size_type pos = 0;
    string headersString = msg.substr(0, msg.find("\n\n"));
    while ((pos = headersString.find(":")) != string::npos) {
        string key = headersString.substr(0, pos);
        headersString = headersString.substr(pos + 1);
        pos = headersString.find("\n");
        string value = headersString.substr(0, pos);
        headersString = headersString.substr(pos + 1);
        mHeaders[key] = value;
    }

    //body
    mBody = msg.substr(msg.find("\n\n") + 2);
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
    std::cout << std::endl << mCommand << std::endl;
    if(includeHeaders)
        for (auto const &pair: mHeaders)
            std::cout << pair.first << " : " << pair.second << std::endl;
    std::cout << std::endl << mBody << std::endl << "\0" << std::endl;
}

