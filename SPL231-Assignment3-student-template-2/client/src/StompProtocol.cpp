#include <sstream>
#include <vector>
#include <map>
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch):    mDisconnectRec(0), mReceiptCounter(1), mSubId(0), mConnectionHandler(&ch),
                                                        subscriptionsByTopic(unordered_map<string,int>()),
                                                        subscriptionsById(unordered_map<int,string>()),
                                                        receipts(unordered_map<int,pair<string,bool>>()),
                                                        commands()
{
    initCommands();
}

StompProtocol::StompProtocol(const StompProtocol& protocol): mDisconnectRec(0), mReceiptCounter(1), mSubId(0), mConnectionHandler(),
                                                        subscriptionsByTopic(unordered_map<string,int>()),
                                                        subscriptionsById(unordered_map<int,string>()),
                                                        receipts(unordered_map<int,pair<string,bool>>()),
                                                        commands()
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
    commands["subscribe"] = 2;
    commands["unsubscribe"] = 3;
    commands["send"] = 5;
    commands["disconnect"] = 4;

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

string StompProtocol::processKeyboard(string msg) {
    vector<string> tokens = tokenize(msg, ' ');
    string out;
    switch (commands[tokens[0]]) {
        //Login
        case 0:
            out = login(tokens);
            break;

        //Subscribe
        case 2:
            out = "SUBSCRIBE\nid:" + to_string(mSubId) + "\ndestination:" + tokens[1] + "\n\n\0";
            mSubId++;
            break;

            //Unsubscribe
        case 3:
            out = "UNSUBSCRIBE\nid:" + tokens[1] + "\n\n\0";
            break;

            //Send
        case 5:
            out = "SEND\ndestination:" + tokens[1] + "\n\n" + tokens[2] + "\n\0";
            break;

            //Disconnect
        case 4:
            out = "DISCONNECT\nreceipt:" + to_string(mReceiptCounter) + "\n\n\0";
            mReceiptCounter++;
            break;

            //Defualt - Invalid
        default:
            std::cout << "Invalid frame code" << std::endl;
            break;
    }
    std::cout << std::endl << out << std::endl;
    return out;
}

string StompProtocol::processFrame(string msg) {
    StompFrame newFrame(msg);
    //newFrame.printFrame(1);
    string out = "";
    switch(commands[newFrame.getCommand()]){

        //Message
        case -1:
        out = message(newFrame.getBody());
        break;

        //Receipt
        case -2:
        receipt(newFrame);
        break;

        //Connected
        case -3:
        std::cout << "Connected" << std::endl;
        newFrame.printFrame(1);
        break;

        //Error
        case -4:
        error(newFrame);
        break;

        //Defualt - Invalid (Error)
        default:
        error(newFrame);
        break;
    }
    return out;
}

void StompProtocol::error(StompFrame frame) {
    frame.printFrame(0);
    disconnect();
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
    std::cout << "Already logged in" << std::endl;
    return "";
}

void StompProtocol::receipt(StompFrame frame) {
    cout << "Receipt recieved with id: " << frame.getHeaderValue("receipt-id") << endl;
}

string StompProtocol::logout(vector<string> msg) {
    string out = "DISCONNECT";
    out=out+"\n"+"receipt:"+to_string(mReceiptCounter)+"\n";
    mDisconnectRec = mReceiptCounter;
    mReceiptCounter++;
    return out;
}

string StompProtocol::report(vector<string> msg) {
    string out = "SEND";
    out = out+"\n"+"destination:" + msg[1] + "\n" + "\n" + "book status" + "\n";
    return out;
}

string StompProtocol::join(vector<string> msg) {
    string out = "SUBSCRIBE";
    mSubId++;
    addToReceipts(mReceiptCounter,msg[1],true);
    addToSubscription(msg[1],mSubId);
    out=out+"\n"+"destination:"+msg[1]+"\n"+"id:"+to_string(mSubId)+"\n"+"receipt:"+to_string(mReceiptCounter)+"\n"+"\n" + "\0";
    mReceiptCounter++;
    return out;
}

string StompProtocol::exit(vector<string> msg) {
    string out = "";
    string topic = msg[1];
    if(getSubId(topic)!=-1) {
         out = "UNSUBSCRIBE";
        int id = getSubId(topic);
        removeSubscription(topic);
        addToReceipts(mReceiptCounter, topic, false);
        out = out + "\n" + "id:" + to_string(id) + "\n" + "receipt:" + to_string(mReceiptCounter) + "\n" + "\n"+ "\0";
        mReceiptCounter++;
    }
    return out;
}

string StompProtocol::message(string body){
    cout << "Message Received:" << endl;
    cout << body << endl;
    return body;
}

string StompProtocol::send(string msg, string topic) {
    string out="SEND";
    out=out+"\n"+"destination:"+topic+"\n"+"\n"+msg+"\n";
    return out;
}

string StompProtocol::look4Header(string header,vector<string> msg) {
    for (string s:msg){
        int ndx =s.find(":");
        if (s.substr(0,ndx)==header)
            return s.substr(ndx+1, s.size()-1);
    }
    return "";
}

  int StompProtocol::getSubId(string topic) {
    if (subscriptionsByTopic.count(topic)==0)
        return -1;
    return subscriptionsByTopic.at(topic);
  }

  void StompProtocol::addToReceipts(int recId, string topic, bool action) { //action true for subscribe false for unsubscribe
    receipts.insert(pair<int, pair<string,bool>>(recId,pair<string, bool >(topic,action)));
}
unordered_map<int,pair<string,bool>> StompProtocol::getReceipts() { 
    return receipts;
}
  void StompProtocol::addToSubscription(string topic, int id) {
      subscriptionsByTopic.insert(pair<string,int>(topic,id));
      subscriptionsById.insert(pair<int,string>(id,topic));

  }
  void StompProtocol::removeSubscription(string topic) {
      int id = subscriptionsByTopic.at(topic);
      subscriptionsByTopic.erase(topic);
      subscriptionsById.erase(id);
  }

  void StompProtocol::disconnect(){
    this->subscriptionsById.clear();
    this->subscriptionsByTopic.clear();
    this->receipts.clear();
    mConnectionHandler->setLoggedIn(false);
    mConnectionHandler->close();



}