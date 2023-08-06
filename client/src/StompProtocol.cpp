#include "../include/StompProtocol.h"
#include "../include/KeyboardThread.h"
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch): 
    mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), user(), mConnectionHandler(&ch),
    commands(), subscriptions(), excpectedReciepts(), allReports() {}

StompProtocol::StompProtocol(const StompProtocol& protocol): 
    mDisconnectRec(-1), mReceiptCounter(1), mSubId(0), user(), mConnectionHandler(),
    commands(), subscriptions(), excpectedReciepts(), allReports() {}

const StompProtocol& StompProtocol::operator=(const StompProtocol& protocol) { return *(this); }

vector<string> StompProtocol::tokenize(string source, char delimiter){
    vector<string> tokens;
    string token;
    stringstream stream(source);

    while(getline(stream, token, delimiter)){
        tokens.push_back(token);
    }

    return tokens;
}

ClientCommand StompProtocol::stringToCommand(string str){
    static const std::unordered_map<std::string, ClientCommand> commandsMap = {
            {"login", ClientCommand::login},
            {"join", ClientCommand::join},
            {"exit", ClientCommand::unjoin},
            {"logout", ClientCommand::logout},
            {"send", ClientCommand::sendMsg},
            {"report", ClientCommand::report},
            {"summary", ClientCommand::summary},
        };

    try{
        return commandsMap.at(str);
    } catch (const std::out_of_range& e) {
        return ClientCommand::invalid;
    }
}

bool StompProtocol::validate(vector<string> command, std::size_t expected, string structure){
    if(command.size() != expected){
        cout << "Invalid arguments. Expected: " << structure << endl;
        return false;
    }
    return true;
}

bool StompProtocol::validateCommand(vector<string> command){
    ClientCommand commandType = stringToCommand(command[0]);
    switch (commandType){
        case ClientCommand::login:
            return validate(command, 4, "login {host:port} {username} {password}");
        case ClientCommand::join:
            return validate(command, 2, "join {game_name}");
        case ClientCommand::unjoin:
            return validate(command, 2, "exit {game_name}");
        case ClientCommand::logout:
            return validate(command, 1, "logout");
        case ClientCommand::sendMsg:
            return validate(command, 3, "send {destination} {message}");
        case ClientCommand::report:
            return validate(command, 2, "report {file}");
        case ClientCommand::summary:
            return validate(command, 4, "summary {game_name} {user} {file}");
        default:
            cout << "Invalid command type" << endl;
            return false;
    }
}

StompFrame* StompProtocol::processKeyboard(string input) { 
    //Empty command
    if(input == "")
        return nullptr;

    vector<string> tokens = tokenize(input, ' ');

    //Not logged in
    if(mConnectionHandler->isLoggedIn() == false && tokens[0] != "login"){
        std::cout << mConnectionHandler->isLoggedIn() << std::endl;
        std::cout << "log in first" << std::endl;
        return nullptr;
    }

    //Invalid command
    if(!validateCommand(tokens))
        return nullptr;

    switch (stringToCommand(tokens[0])) {
        
        case ClientCommand::login:
            return login(tokens);
        
        case ClientCommand::join:
            return subscribe(tokens[1]);
        
        case ClientCommand::unjoin:
            return unsubscribe(tokens[1], subscriptions[tokens[1]]);
        
        case ClientCommand::logout:
            return logout();
        
        case ClientCommand::sendMsg:
            return send(tokens[1], tokens[2]);

        case ClientCommand::report:
            for (StompFrame* frame : report(tokens[1])){
                KeyboardThread::sendFrame(frame);
            }
            return nullptr;

        case ClientCommand::summary:
            if(allReports.count(make_tuple(tokens[1], tokens[2]))){
                parseToFile(allReports[make_tuple(tokens[1], tokens[2])], tokens[3]);
                std::cout << "Game reports summary saved to file" << std::endl;
            }
            else{
                std::cout << "No relevant reports found" << std::endl;
            }
            return nullptr;
            
        default:
            std::cout << "Invalid command" << std::endl;
            return nullptr;
    }
}

void StompProtocol::processFrame(StompFrame newFrame) {
    /*  DEBUG : print received frame
    */
    std::cout << "=== RECEIVED ===" << std::endl << newFrame.toString() << std::endl << "================" << std::endl;

    switch(newFrame.getCommand()){

        case FrameCommand::MESSAGE:
            message(newFrame);
        break;

        case FrameCommand::RECEIPT:
            handleReceipt(newFrame);
        break;

        case FrameCommand::CONNECTED:
            mConnectionHandler->setLoggedIn(1);
            std::cout << "Login successful" << std::endl;
        break;

        //Error
        default:
            std::cout << newFrame.toString() << std::endl;
            disconnect();
        break;

    }
}

StompFrame* StompProtocol::subscribe(string destination){
    //Create SUBSCRIBE frame
    StompFrame* newFrame = new StompFrame(FrameCommand::SUBSCRIBE);
    newFrame->addHeader("destination", destination.data());
    newFrame->addHeader("id", std::to_string(mSubId).data());
    newFrame->addHeader("receipt", std::to_string(mReceiptCounter).data());

    //Update data
    mSubId++;
    mReceiptCounter++;
    excpectedReciepts[mReceiptCounter] = make_tuple(ReceiptType::subscribe, mSubId, destination);
    
    return newFrame;
}

StompFrame* StompProtocol::unsubscribe(string destination, int subId){
    //Create UNSUBSCRIBE frame
    StompFrame* newFrame = new StompFrame(FrameCommand::UNSUBSCRIBE);
    newFrame->addHeader("receipt", std::to_string(mReceiptCounter).data());
    newFrame->addHeader("id", std::to_string(subId).data());

    mReceiptCounter++;
    return newFrame;
}

string StompProtocol::extractUser(const string& frameBody){
    string body(frameBody);
    int idx;

    idx = body.find("user:") + 6;
    body = body.substr(idx);
    idx = body.find('\n');
    
    return body.substr(0, idx);
}

void StompProtocol::parseToFile(const map<int, Event*>& reports, const string& file){
    std::map<string, string> updates; //a map that contain only the latest update of each game
    Event* event;
    
    std::ofstream out_file(file, std::ios::trunc | std::ios::out);

    if (!out_file.is_open()) {
        std::cerr << "Failed to open file for writing" << std::endl;
        return;
    }

    //Title
    out_file << reports.begin()->second->get_team_a_name() + " vs " + reports.begin()->second->get_team_b_name() << "\n\n" << "Game stats:" << "\n";

    //General
    string header;
    out_file << "General stats:" << '\n';
    for (auto const &pair: reports) {
        event = pair.second;
        for (auto const &update: event->get_game_updates()) {
            header = update.first;
            if (updates.count(update.first) == 0) {
                updates[header] = update.second;
            } else {
                if (header == "active") {
                    if (update.second != updates[header])
                        updates[header] = update.second;
                } else if (header == "before halftime") {
                    if (update.second != updates[header])
                        updates[header] = update.second;
                } else {
                    updates[header] = update.second;
                }
            }
        }
    }
    for(auto const &update: updates){
        out_file << update.first + ": " + update.second << '\n';
    }
    updates.clear();

    //Team A
    out_file << "\n" << event->get_team_a_name() << " stats:" << '\n';
    for (auto const &pair: reports) {
        event = pair.second;
        for (auto const &update: event->get_team_a_updates()) {
            header = update.first;
            if (updates.count(header) == 0) {
                updates[header] = update.second;
            } else {
                if(update.second != updates[header])
                    updates[header] = update.second;
            }
        }
    }
    for(auto const &update: updates){
        out_file << update.first + ": " + update.second << '\n';
    }
    updates.clear();
    //Team B
    out_file << "\n" << event->get_team_b_name() << " stats:" << '\n';
    for (auto const &pair: reports){
        event = pair.second;
        for (auto const &update: event->get_team_b_updates()){
            header = update.first;
            if (updates.count(header) == 0) {
                updates[header] = update.second;
            } else {
                if(update.second != updates[header])
                    updates[header] = update.second;
            }
        }
    }
    for(auto const &update: updates){
        out_file << update.first + ": " + update.second << '\n';
    }
    //Events
    out_file << "\n" << "Game event reports:" << '\n';
    for (auto const &pair: reports){
        event = pair.second;
        out_file << pair.first << " - " << event->get_name() << '\n' << event->get_discription() << "\n\n";
    }
    out_file.close();
}

StompFrame* StompProtocol::login(vector<string> msg) {
    //init connection
    if(!mConnectionHandler -> isLoggedIn()) {
        string host = msg[1].substr(0, msg[1].find(":"));
        mConnectionHandler -> setHost(host);
        short port = std::stoi(msg[1].substr(msg[1].find(":") + 1, msg[1].length()));
        mConnectionHandler -> setPort(port);

        //Error connecting
        if (!mConnectionHandler -> connect()) {
            std::cerr << "Unable to connect " << host << ":" << port << std::endl;
            return nullptr;

        //Connected
        } else {
            std::cout << "Logging in" << std::endl;
            mConnectionHandler -> setName(msg[2]);

            // create CONNECT frame
            StompFrame* newFrame = new StompFrame(FrameCommand::CONNECT);
            newFrame -> addHeader("accept-version", "1.2");
            newFrame -> addHeader("host", "stomp.cs.bgu.ac.il");
            newFrame -> addHeader("login", msg[2].data());
            newFrame -> addHeader("passcode", msg[3].data());
            return newFrame;
        }
    }
    std::cout << "The client is already logged in, log out before trying again" << std::endl;
    return nullptr;
}

StompFrame* StompProtocol::logout() {
    StompFrame* newFrame = new StompFrame(FrameCommand::DISCONNECT);
    newFrame -> addHeader("receipt", std::to_string(mReceiptCounter).data());
    mDisconnectRec = mReceiptCounter;
    mReceiptCounter++;
    return newFrame;
}

void StompProtocol::disconnect(){
    mConnectionHandler->disconnect();
    subscriptions.clear();
    allReports.clear();
    std::cout << "Disconnected" << std::endl;
}

StompFrame* StompProtocol::send(const string& destination, const string& body){
    StompFrame* newFrame = new StompFrame(FrameCommand::SEND);
    newFrame->addHeader("destination", destination.data());
    newFrame->addHeader("user", mConnectionHandler->getName().data());
    newFrame->setBody(body.data());
    return newFrame;
}

void StompProtocol::message(const StompFrame& frame){
    //Save event
    Event* event = new Event(frame.getBody());
    string dest = frame.getHeaderValue("destination");
    string user = extractUser(frame.getBody());
    allReports[make_tuple(dest, user)][event->get_time()] = event;

    //Print event
    std::cout << std::endl << "destination: " << dest << std::endl << "user: " << user << std::endl << std::endl;
    event->printEvent();
}

void StompProtocol::handleReceipt(const StompFrame& frame){
    int recId = std::stoi(frame.getHeaderValue("receipt-id"));
        if (excpectedReciepts.count(recId)){
            ReceiptType t = get<0>(excpectedReciepts[recId]);
            int id = get<1>(excpectedReciepts[recId]);
            string d = get<2>(excpectedReciepts[recId]);
            switch(t){
                case ReceiptType::disconnect:
                    disconnect();
                    break;
                
                case ReceiptType::subscribe:
                    subscriptions[d] = id;
                    std::cout << "Subscribed to " + d << std::endl;
                    break;
                
                case ReceiptType::unsubscribe:
                    subscriptions.erase(d);
                    std::cout << "Unsubscribed from " + d << std::endl;
                    break;

                default:
                    std::cout << "Unknown receipt received" + d << std::endl;
                    break;
            }
            excpectedReciepts.erase(recId);
        }
}

vector<StompFrame*> StompProtocol::report(const string& file){
    vector<StompFrame*> out;
    names_and_events allEvents = parseEventsFile(file);

    string dest = '/' + allEvents.team_a_name + '_' + allEvents.team_b_name;

    for (Event event : allEvents.events){
        //Create SEND frame
        StompFrame* frame = new StompFrame(FrameCommand::SEND);
        frame->addHeader("destination", dest.data());
        frame->addHeader("user", mConnectionHandler->getName().data());
        frame->addHeader("team a", allEvents.team_a_name.data());
        frame->addHeader("team b", allEvents.team_b_name.data());

        //Create body
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

        frame->setBody(body.data());
        out.push_back(frame);
    }
    return out;
}
