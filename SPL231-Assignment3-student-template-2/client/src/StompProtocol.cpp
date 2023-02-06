#include <sstream>
#include <vector>
#include <map>
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompFrame.h"
#include "../include/event.h"

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch): 
mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), user(), mConnectionHandler(&ch), commands(), subscriptions(), excpectedReciepts(), allReports()
{
    initCommands();
}

StompProtocol::StompProtocol(const StompProtocol& protocol): 
mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), user(), mConnectionHandler(), commands(), subscriptions(), excpectedReciepts(), allReports()
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
    commands["login"] = Command::login;
    //commands["connect"] = 1;
    commands["join"] = Command::join;
    commands["exit"] = Command::unjoin;
    commands["logout"] = Command::logout;
    commands["send"] = Command::sendMessage;
    commands["report"] = Command::report;
    commands["summary"] = Command::summary;

    //Server
    commands["MESSAGE"] = Frame::message;
    commands["RECEIPT"] = Frame::receipt;
    commands["CONNECTED"] = Frame::connected;
    commands["ERROR"] = Frame::error;
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

        case Command::login:
            expectedSize = 4;
            structure = "login {host:port} {username} {password}";
            break;

        case Command::join:
            expectedSize = 2;
            structure = "join {game_name}";
            if(subscriptions.count(command[1]) > 0){
                cout << "Client is already subscribed to " << command[1]  << " with id " << subscriptions[command[1]] << endl;
                return false;
            }
            break;
        case Command::unjoin:
            expectedSize = 2;
            structure = "exit {game_name}";
            try{
                subscriptions.at(command[1]);
            } catch(std::out_of_range& e){
                cout << "Invalid command. Client is not subscribed to " << command[1] << endl;
                return false;
            }
            break;

        case Command::sendMessage:
            expectedSize = 3;
            structure = "send {destination} {message}";
            if(command.size() < expectedSize){
                cout << "Invalid arguments. Expected: " << structure << endl;
                return false;
            } else 
                return true;
            break;

        case Command::report:
            expectedSize = 2;
            structure = "report {file}";
            break;

        case Command::summary:
            expectedSize = 4;
            structure = "summary {game_name} {user} {file}";
            break;
    }

        if(command.size() != expectedSize){
            cout << "Invalid arguments. Expected: " << structure << endl;
            return false;
        }
    return true;
}

vector<string> StompProtocol::processKeyboard(string msg) {
    vector<string> out;
    string s;

    if(msg == "")
        return out;

    vector<string> tokens = tokenize(msg, ' ');

    if(!mConnectionHandler->isLoggedIn() && tokens[0] != "login"){
        std::cout << "login first" << std::endl;
        return out;
    }

    if(!validateCommand(tokens))
        return out;

    switch (commands.at(tokens[0])) {
        
        case Command::login:
            out.push_back(login(tokens));
            break;
        
        case Command::join:
            out.push_back("SUBSCRIBE\nreceipt:" + to_string(mReceiptCounter) + "\nid:" + to_string(mSubId) + "\ndestination:" + tokens[1] + "\n\n\0");
            excpectedReciepts[mReceiptCounter] = make_tuple(subscribe, mSubId, tokens[1]);
            mSubId++;
            mReceiptCounter++;
            break;
        
        case Command::unjoin:
            out.push_back("UNSUBSCRIBE\nreceipt:" + to_string(mReceiptCounter) + "\nid:" + to_string(subscriptions.at(tokens[1])) + "\n\n\0");
            excpectedReciepts[mReceiptCounter] = make_tuple(unsubscribe, subscriptions.at(tokens[1]), tokens[1]);
            mReceiptCounter++;
            break;
        
        case Command::logout:
            excpectedReciepts[mReceiptCounter] = make_tuple(disconnect, 0, "");
            out.push_back(logout());
            break;
        
        case Command::sendMessage:
            s = tokens[2];
            for (int unsigned i=3; i<tokens.size(); i++)
                s += " " + tokens[i];
            out.push_back(send(tokens[1], s));
            break;

        case Command::report:
            out = report(tokens[1]);
            break;

        //Just a debug for now
        //TODO
        case Command::summary:
            for (auto const &pair1: allReports){
                for (auto const &pair2: allReports[make_tuple(get<0>(pair1.first),  get<1>(pair1.first))]){
                    string k = "(" + get<0>(pair1.first) + ", " + get<1>(pair1.first) + ")";
                    std::cout << k << " : (" << pair2.first << " : event )" << std::endl;
                }
            }
            break;

        default:
            std::cout << "Invalid frame code" << std::endl;
            break;
    }

    /* if(out != ""){
        std::cout << "=== SENT ===" << std::endl;
        std::cout << out << std::endl;
    } */
    return out;
}

void StompProtocol::processFrame(StompFrame newFrame) {
    //std::cout << "=== RECEIVED ===" << std::endl;
    //newFrame.printFrame(1);
    int recId;
    Event* event;
    string dest, user;
    switch(commands[newFrame.getCommand()]){

        case Frame::message:
            //newFrame.printFrame(1);
            //std::cout << std::endl << newFrame.getBody() << std::endl;
            event = new Event(newFrame.getBody());
            dest = newFrame.getHeaderValue("destination");
            user = extractUser(newFrame.getBody());

            std::cout << std::endl << "destination: " << dest << std::endl << "user: " << user << std::endl << std::endl;
            event->printEvent();
            //TODO: insert new event to allReports
            allReports[make_tuple(dest, user)][event->get_time()] = event;
        break;

        case Frame::receipt:
        recId = std::stoi(newFrame.getHeaderValue("receipt-id"));

        if (excpectedReciepts.count(recId)){
            Type t = get<0>(excpectedReciepts[recId]);
            int id = get<1>(excpectedReciepts[recId]);
            string d = get<2>(excpectedReciepts[recId]);
            switch(t){

                case disconnect:
                    mConnectionHandler->disconnect();
                    subscriptions.clear();
                    std::cout << "Disconnected" << std::endl;
                    break;
                
                case subscribe:
                    subscriptions[d] = id;
                    std::cout << "Subscribed to " + d << std::endl;
                    break;
                
                case unsubscribe:
                    subscriptions.erase(d);
                    std::cout << "Unsubscribed from " + d << std::endl;
                    break;

                default:
                    std::cout << "Unknown receipt received" + d << std::endl;
                    break;
            }

            excpectedReciepts.erase(recId);
        }
        break;

        case Frame::connected:
            mConnectionHandler->setLoggedIn(1);
            std::cout << "Login successful" << std::endl;
        break;

        //Error
        default:
            newFrame.printFrame(1);
        break;

    }
}

string StompProtocol::extractUser(const string& frameBody){
    string body(frameBody);
    int idx;

    idx = body.find("user:") + 6;
    body = body.substr(idx);
    idx = body.find('\n');
    
    return body.substr(0, idx);
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

string StompProtocol::send(const string& destination, const string& body){
    return "SEND\ndestination:" + destination + "\n\n" + body + "\n\0";
}

vector<string> StompProtocol::report(const string& file){
    vector<string> out;
    names_and_events allEvents = parseEventsFile(file);

    string dest = '/' + allEvents.team_a_name + '_' + allEvents.team_b_name;

    string head = "";
    head += "user: " + mConnectionHandler->getName() + '\n';
    head += "team a: " + allEvents.team_a_name + '\n';
    head += "team b: " + allEvents.team_b_name + '\n';

    for (Event event : allEvents.events){
        string body = "";
        body += "event name: " + event.get_name() + '\n';
        body += "time: " + to_string(event.get_time()) + '\n';

        body += "general game updates:\n";
        for(auto update : event.get_game_updates()){
            body += update.first + ": " + update.second + '\n';
        }

        body += "team a updates:\n";
        for(auto update : event.get_team_a_updates()){
            body += update.first + ": " + update.second + '\n';
        }

        body += "team b updates:\n";
        for(auto update : event.get_team_b_updates()){
            body += update.first + ": " + update.second + '\n';
        }

        body += "description:\n";
        body += event.get_discription();

        out.push_back(send(dest, head + body));
    }
    return out;
}