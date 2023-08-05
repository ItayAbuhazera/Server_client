#include "../include/StompFrame.h"

using namespace std;

StompFrame::StompFrame(string msg): mCommand(stringToCommand(msg.substr(0, msg.find('\n')))), mHeaders(), mBody("") {
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

StompFrame::StompFrame(FrameCommand command, map<string, string> headers, string body): mCommand{command}, mHeaders{headers}, mBody{body} {};

StompFrame::StompFrame(FrameCommand command): mCommand{command}, mHeaders{}, mBody{} {};

void StompFrame::addHeader(const string& header, const string& value){
    mHeaders[header] = value;
}

void StompFrame::setBody(const string& body){
    mBody = body;
}

const FrameCommand& StompFrame::getCommand() const{
    return mCommand;
}

const string& StompFrame::getHeaderValue(const string& header) const{
    return mHeaders.at(header);
}

const string& StompFrame::getBody() const{
    return mBody;
}

string StompFrame::toString() const {
    string str = commandToString(mCommand) + "\n";
    for (auto const &pair: mHeaders)
        str += pair.first + ":" + pair.second + "\n";
    str += "\n" + mBody + "\n" + '\0';
    return str;
}

FrameCommand StompFrame::stringToCommand(string str) {
    static const std::unordered_map<std::string, FrameCommand> commandsMap = {
        {"CONNECT", FrameCommand::CONNECT},
        {"CONNECTED", FrameCommand::CONNECTED},
        {"SUBSCRIBE", FrameCommand::SUBSCRIBE},
        {"UNSUBSCRIBE", FrameCommand::UNSUBSCRIBE},
        {"SEND", FrameCommand::SEND},
        {"MESSAGE", FrameCommand::MESSAGE},
        {"ERROR", FrameCommand::ERROR},
        {"DISCONNECT", FrameCommand::DISCONNECT},
    };

    if (commandsMap.find(str) == commandsMap.end())
        throw std::invalid_argument("Invalid FrameCommand string");
    return commandsMap.at(str);
}

string StompFrame::commandToString(FrameCommand command) {
    switch (command) {
        case FrameCommand::CONNECT:
            return "CONNECT";
        case FrameCommand::CONNECTED:
            return "CONNECTED";
        case FrameCommand::SUBSCRIBE:
            return "SUBSCRIBE";
        case FrameCommand::UNSUBSCRIBE:
            return "UNSUBSCRIBE";
        case FrameCommand::SEND:
            return "SEND";
        case FrameCommand::MESSAGE:
            return "MESSAGE";
        case FrameCommand::ERROR:
            return "ERROR";
        case FrameCommand::DISCONNECT:
            return "DISCONNECT";
        default:
            throw std::invalid_argument("Invalid FrameCommand");
    }
}
