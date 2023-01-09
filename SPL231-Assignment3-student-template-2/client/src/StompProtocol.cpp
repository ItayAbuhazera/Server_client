#include <sstream>
#include <vector>
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch): disconnectRec(0),receiptCounter(1),subId(0), ch(&ch){}


string StompProtocol::processKeyboard(string msg) {
    vector<string> tokens;
    while(msg!=""){
        string tmp;
        if(msg.find(" ")==std::string::npos) {
            tmp = msg;
            tokens.push_back(tmp);
            break;
        }
        tmp=msg.substr(0,msg.find(" "));
        tokens.push_back(tmp);
        msg=msg.substr(msg.find(" ")+1,msg.length());

    }
    string out = "";
    if(tokens[0] == "login")
        out = login(tokens);
    if(ch -> isConnected()) {
        if (tokens[0] == "join")
            out = join(tokens);
        if (tokens[0] == "borrow")
            out = borrow(tokens);
        if (tokens[0] == "add")
            out = addBook(tokens);
        if (tokens[0] == "exit")
            out = exit(tokens);
        if (tokens[0] == "logout")
            out = logout(tokens);
        if (tokens[0] == "return")
            out = returnBook(tokens);
        if (tokens[0] == "status")
            out = status(tokens);
    }
    return out;
}


string StompMessageProtocol::processFrame(string msg) {
    vector<string> tokens;
    std::istringstream stream(msg);
    std::string line;
    while(std::getline(stream, line)) {
        tokens.push_back(line);
    }
    string out = "";
    if(tokens[0]=="MESSAGE")
        out=message(tokens);
    if(tokens[0]=="CONNECTED")
        connected();
    if(tokens[0]=="RECEIPT")
        receipt(tokens);
    if(tokens[0]=="ERROR")
        error(tokens);
    return  out;
}

void StompMessageProtocol::error(vector<string> tokens) {
    cout<<"Error"<<endl;
    cout<<tokens[tokens.size()-1]<<endl;
    ch->disconnecting();
}

string StompMessageProtocol::connected() {
    cout<<"login successful"<<endl;
    return "";
}

string StompMessageProtocol::login(vector<string> msg) {
    if(!ch->isConnected()) {
        string host = msg[1].substr(0, msg[1].find(":"));
        ch->setHost(host);
        short port = std::stoi(msg[1].substr(msg[1].find(":") + 1, msg[1].length()));
        ch->setPort(port);
        if (!ch->connect()) {
            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
            return "";
        }
        ch->connecting();
        ch->setName(msg[2]);
        string out = "CONNECT";
        out = out + "\n" + "accept-version:1.2" + "\n" + "host:stomp.cs.bgu.ac.il" + "\n" + "login:" + msg[2] + "\n" +
              "passcode:" + msg[3] + "\n" + "\n";
        return out;
    }
    else {
        string out = "CONNECT";
        out = out + "\n" + "accept-version:1.2" + "\n" + "host:stomp.cs.bgu.ac.il" + "\n" + "login:" + msg[2] + "\n" +
              "passcode:" + msg[3] + "\n" + "\n";
        return out;
    }
}

void StompMessageProtocol::receipt(vector<string> tokens) {
    const string receiptId = tokens[1].substr(tokens[1].find(":")+1, tokens[1].length());
    int reccc=std::stoi(receiptId);
    if (reccc==disconnectRec){
        ch->disconnecting();
        return;
    }
    if(ch->getREceipts().count(reccc)!=0){
        if(ch->getREceipts().at(reccc).second)
            cout<<"Joined club "+ ch->getREceipts().at(reccc).first<<endl;
        else
            cout<<"Exited club "+ch->getREceipts().at(reccc).first<<endl;
    }
}

string StompMessageProtocol::logout(vector<string> msg) {
    string out = "DISCONNECT";
    out=out+"\n"+"receipt:"+to_string(receiptCounter)+"\n";
    disconnectRec = receiptCounter;
    receiptCounter++;

    return out;
}
string StompMessageProtocol::status(vector<string> msg) {
    string out ="SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+"book status"+"\n";
    return out;
}
string StompMessageProtocol::addBook(vector<string> msg) {
    int bookSize=msg.size()-2;
    string book="";
    for(int i=0;i<bookSize;i++)
        book=book+" "+msg[2+i];
    book=book.substr(1,book.length());
    string out = "SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+ch->getName()+" has added the book "+book+"\n";
    ch->addAndRemoveInventory(book,msg[1],0);
    return out;
}
string StompMessageProtocol::borrow(vector<string> msg) {
    int bookSize=msg.size()-2;
    string book="";
    for(int i=0;i<bookSize;i++)
        book=book+" "+msg[2+i];
    book=book.substr(1,book.length());
    string out = "SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+ch->getName()+" wish to borrow "+book+"\n";
    ch->addAndRemoveToWish(book,0);
    return  out;
}
string StompMessageProtocol::returnBook(vector<string> msg) {
    int bookSize=msg.size()-2;
    string book="";
    for(int i=0;i<bookSize;i++)
        book=book+" "+msg[2+i];
    book=book.substr(1,book.length());
    string out = "SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+"Returning "+book+ " to " +ch->getLender(book) + "\n";
    ch->addAndRemoveBorrowed(book,"",1);
    ch->addAndRemoveInventory(book,msg[1],1);
    return  out;
}
string StompMessageProtocol::join(vector<string> msg) {
    string out = "SUBSCRIBE";
    subId++;
    ch->addToReceipts(receiptCounter,msg[1],true);
    ch->addToSubscription(msg[1],subId);
    out=out+"\n"+"destination:"+msg[1]+"\n"+"id:"+to_string(subId)+"\n"+"receipt:"+to_string(receiptCounter)+"\n"+"\n";
    receiptCounter++;
    return out;
}
string StompMessageProtocol::exit(vector<string> msg) {
    string out = "";
    string topic = msg[1];
    if(ch->getSubId(topic)!=-1) {
         out = "UNSUBSCRIBE";
        int id = ch->getSubId(topic);
        ch->removeSubscription(topic);
        ch->addToReceipts(receiptCounter, topic, false);
        out = out + "\n" + "id:" + to_string(id) + "\n" + "receipt:" + to_string(receiptCounter) + "\n" + "\n";
        receiptCounter++;
    }
    return out;
}

string StompMessageProtocol::message(vector<string> msg) {
    vector<string> tokens;
    string out="";
    string topic = findHeader("destination", msg);
    string message = msg[msg.size()-1];
    cout<<topic+":"+message<<endl;
    while(message!=""){
        string tmp;
        if(message.find(" ")==std::string::npos) {
            tmp = message;
            tokens.push_back(tmp);
            break;
        }
        tmp=message.substr(0,message.find(" "));
        tokens.push_back(tmp);
        message=message.substr(message.find(" ")+1,message.length());
    }

    if((tokens[0]!=hand->getName())&((tokens.size()>1)&&(tokens[1]=="wish"))){
            int bookSize=tokens.size()-4;
            string book="";
            for(int i=0;i<bookSize;i++)
                book=book+" "+tokens[4+i];
            book=book.substr(1,book.length());
            if (ch->hasBook(book,topic)){
                string message=hand->getName()+" has "+book;
                out=sendFrame(message,topic);
                 return out;
            }
        }


    if((tokens[0]=="Returning")&((tokens[tokens.size()-1]==hand->getName()))) {
        cout<< hand->getName() + " in returning action"<<endl;
        int bookSize=tokens.size()-3;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[1+i];
        book=book.substr(1,book.length());
        if(hand->isBorrowed(book)){
            cout<<hand->getName() + " is in borrowed need to return to " + hand->getLender(book)<<endl;
            string out = "SEND";
            out=out+"\n"+"destination:"+topic+"\n"+"\n"+"Returning  "+book+ " to " +hand->getLender(book) + "\n";
            hand->addAndRemoveBorrowed(book,"",1);
            return  out;
        }
        hand->addAndRemoveInventory(book, topic, 0);
        hand->addAndRemoveBorrowed(book, "", 1);
    }
    if(tokens[0]=="book"){
        string message=hand->getName()+": ";
        vector <string> inv = hand->getBooks(topic);
        for(string book:inv) {
            message = message + book + ",";
        }
        message=message.substr(0,message.length()-1);
        out=sendFrame(message,topic);
        return out;
    }

    if(tokens[0]=="Taking"&&(tokens[tokens.size()-1]==hand->getName())){
        int bookSize=tokens.size()-3;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[1+i];
        book=book.substr(1,book.length());
        hand->addAndRemoveInventory(book,topic,1);
    }
    if(tokens.size()>1 &&(tokens[1]=="has")) {
        int bookSize=tokens.size()-2;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[2+i];
        book=book.substr(1,book.length());
        if (hand->wished(book)) {
            string message = "Taking " + book + " from " + tokens[0];
            hand->addAndRemoveBorrowed(book, tokens[0], 0);
            hand->addAndRemoveToWish(book,1);
            hand->addAndRemoveInventory(book,topic,0);
            out = sendFrame(message, topic);
            return out;
        }
    }

    return out;
}

string StompMessageProtocol::sendFrame(string msg, string topic) {
    string out="SEND";
    out=out+"\n"+"destination:"+topic+"\n"+"\n"+msg+"\n";
    return out;
}

string StompMessageProtocol::findHeader(string head,vector<string> msg) {
    for (string s:msg){
        int ndx =s.find(":");
        if (s.substr(0,ndx)==head)
            return s.substr(ndx+1, s.size()-1);
    }
    return "";
}