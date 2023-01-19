#include <sstream>
#include <vector>
#include <map>
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch):    mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), mConnectionHandler(&ch), commands(), subscriptions()
{
    initCommands();
}

StompProtocol::StompProtocol(const StompProtocol& protocol): mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), mConnectionHandler(), commands(), subscriptions()
{
    initCommands();
}

const StompProtocol& StompProtocol::operator=(const StompProtocol& protocol)
{
    initCommands();
    return *(this);
}

void StompProtocol::initCommands(){
    //Client
    commands["login"] = 0;
    //commands["connect"] = 1;
    commands["join"] = 2;
    commands["exit"] = 3;
    commands["logout"] = 4;
    commands["send"] = 5;

    //Server
    commands["MESSAGE"] = -1;
    commands["RECEIPT"] = -2;
    commands["CONNECTED"] = -3;
    commands["ERROR"] = -4;
}

vector<string> StompProtocol::tokenize(string source, char delimiter){
    vector<string> tokens;
    string token;
    stringstream stream(source);

    while(getline(stream, token, delimiter)){
        tokens.push_back(token);
    }

    return tokens;
}

bool StompProtocol::validateCommand(vector<string> command){
    int unsigned expectedSize = 1;
    string structure;
    int type = -1;
    try{
        type = commands.at(command[0]);
    } catch(std::out_of_range& e){
        cout << "Invalid command type" << endl;
        return false;
    }
    
    switch (type) {

        case 0:
            expectedSize = 4;
            structure = "login {host:port} {username} {password}";
            break;

        case 2:
            expectedSize = 2;
            structure = "join {game_name}";
            if(subscriptions.count(command[1]) > 0){
                cout << "Client is already subscribed to " << command[1]  << " with id " << subscriptions[command[1]] << endl;
                return false;
            }
            break;
        case 3:
            expectedSize = 2;
            structure = "exit {game_name}";
            try{
                subscriptions.at(command[1]);
            } catch(std::out_of_range& e){
                cout << "Invalid command. Client is not subscribed to " << command[1] << endl;
                return false;
            }
            break;

        case 5:
            expectedSize = 3;
            structure = "send {destination} {message}";
            if(command.size() < expectedSize){
                cout << "Invalid arguments. Expected: " << structure << endl;
                return false;
            } else 
                return true;
            break;
    }

        if(command.size() != expectedSize){
            cout << "Invalid arguments. Expected: " << structure << endl;
            return false;
        }
    return true;
}

string StompProtocol::processKeyboard(string msg) {
    if(msg == "")
        return "";

    vector<string> tokens = tokenize(msg, ' ');

    if(!mConnectionHandler->isLoggedIn() && tokens[0] != "login"){
        std::cout << "login first" << std::endl;
        return "";
    }

    if(!validateCommand(tokens))
        return "";

    string out;
    switch (commands.at(tokens[0])) {
        //Login
        case 0:
            out = login(tokens);
            break;

        //Subscribe
        case 2:
            out = "SUBSCRIBE\nid:" + to_string(mSubId) + "\ndestination:" + tokens[1] + "\n\n\0";
            subscriptions[tokens[1]] = mSubId;
            mSubId++;
            break;

        //Unsubscribe
        case 3:
            out = "UNSUBSCRIBE\nid:" + to_string(subscriptions.at(tokens[1])) + "\n\n\0";
            subscriptions.erase(tokens[1]);
            break;

        //Send
        case 5:
            out = tokens[2] + " ";
            for (int unsigned i=3; i<tokens.size(); i++)
                out += " " + tokens[i];
            out = "SEND\ndestination:" + tokens[1] + "\n\n" + out + "\n\0";
            break;

        //Disconnect
        case 4:
            out = logout();
            break;

        //Defualt - Invalid
        default:
            std::cout << "Invalid frame code" << std::endl;
            break;
    }

    if(out != ""){
        std::cout << "=== SENT ===" << std::endl;
        std::cout << out << std::endl;
    }
    return out;
}

void StompProtocol::processFrame(StompFrame newFrame) {
    std::cout << "=== RECEIVED ===" << std::endl;
    newFrame.printFrame(1);
    int recId;
    switch(commands[newFrame.getCommand()]){

        //Message
        case -1:
        break;

        //Receipt
        case -2:
        recId = std::stoi(newFrame.getHeaderValue("receipt-id"));
        if(recId == mDisconnectRec){
            mConnectionHandler->disconnect();
            std::cout << "Disconnected" << std::endl;
        }
        break;

        //Connected
        case -3:
        break;

        //Error
        default:
        break;

    }
}

string StompProtocol::login(vector<string> msg) {
    if(!mConnectionHandler -> isLoggedIn()) {
        string host = msg[1].substr(0, msg[1].find(":"));
        mConnectionHandler -> setHost(host);
        short port = std::stoi(msg[1].substr(msg[1].find(":") + 1, msg[1].length()));
        mConnectionHandler -> setPort(port);
        if (!mConnectionHandler -> connect()) {
            std::cerr << "Unable to connect " << host << ":" << port << std::endl;
            return "";
        } else {
            mConnectionHandler -> setName(msg[2]);
            string out = "CONNECT";
            out = out + "\n" + "accept-version:1.2" + "\n" + "host:stomp.cs.bgu.ac.il" + "\n" + "login:" + msg[2] + "\n" +
                "passcode:" + msg[3] + "\n" + "\n";
            return out;
        }
    }
    std::cout << "The client is already logged in, log out before trying again" << std::endl;
    return "";
}

string StompProtocol::logout() {
    string out = "DISCONNECT\nreceipt:" + to_string(mReceiptCounter) + "\n\n\0";
    mDisconnectRec = mReceiptCounter;
    mReceiptCounter++;
    return out;
}