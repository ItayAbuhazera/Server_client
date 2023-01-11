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
 //   switch (tokens[0]) {
	//	case "connect":
	//		out = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:"+tokens[1]+"\npasscode:"+tokens[2]+"\n\n\0";
	//		break;
	//	case "subscribe":
	//		out = "SUBSCRIBE\nid:"+to_string(subId)+"\ndestination:"+tokens[1]+"\n\n\0";
	//		subId++;
	//		break;
	//	case "unsubscribe":
	//		out = "UNSUBSCRIBE\nid:"+tokens[1]+"\n\n\0";
	//		break;
	//	case "send":
	//		out = "SEND\ndestination:"+tokens[1]+"\n\n"+tokens[2]+"\n\0";
	//	    break;
	//	case "disconnect":
	//		out = "DISCONNECT\nreceipt:"+to_string(receiptCounter)+"\n\n\0";
	//		receiptCounter++;
	//	    break;
	//	default:
	//		break;
	//}
    if(tokens[0] == "login"){
        out = login(tokens)
    }
    else{
        if(ch->isLoggedIn()){
            switch (tokens[0]){
                case "join":
                out = join(tokens);
                break;
                case "add":
                out = add(tokens);
                break;
                case "exit":
                out = exit(tokens);
                break;
                case "logout":
                out = logout(tokens);
                break;
                case "report"
                out = report(tokens);
                break;
                case "summary"
                out = summary(tokens);
                break;
                default:
                out = error(tokens);
                break;
        }
    }
    }
    return out;
}


string StompProtocol::processFrame(string msg) {
    vector<string> tokens;
    std::istringstream stream(msg);
    std::string line;
    while(std::getline(stream, line)) {
        tokens.push_back(line);
    }
    string out = "";
    switch(tokens[0]){
        case "MESSAGE":
        out = message(tokens);
        break;
        case "RECEIPT":
        out = receipt(tokens);
        break;
        case "CONNECTED":
        out = connected(tokens);
        break;
        case "ERROR":
        out = error(tokens);
        break;
        default:
        out = error(tokens);
        break;
    }
    return out;
}

void StompProtocol::error(vector<string> tokens) {
    cout<<"Error"<<endl;
    cout<<tokens[tokens.size()-1]<<endl;
    ch->disconnecting();
}

string StompProtocol::connected() {
    cout<<"login successful"<<endl;
    return "";
}

string StompProtocol::login(vector<string> msg) {
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

void StompProtocol::receipt(vector<string> tokens) {
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

string StompProtocol::logout(vector<string> msg) {
    string out = "DISCONNECT";
    out=out+"\n"+"receipt:"+to_string(receiptCounter)+"\n";
    disconnectRec = receiptCounter;
    receiptCounter++;
    return out;
}
string StompProtocol::status(vector<string> msg) {
    string out ="SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+"book status"+"\n";
    return out;
}
string StompProtocol::addBook(vector<string> msg) {
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
string StompProtocol::borrow(vector<string> msg) {
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
string StompProtocol::returnBook(vector<string> msg) {
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
string StompProtocol::join(vector<string> msg) {
    string out = "SUBSCRIBE";
    subId++;
    ch->addToReceipts(receiptCounter,msg[1],true);
    ch->addToSubscription(msg[1],subId);
    out=out+"\n"+"destination:"+msg[1]+"\n"+"id:"+to_string(subId)+"\n"+"receipt:"+to_string(receiptCounter)+"\n"+"\n";
    receiptCounter++;
    return out;
}
string StompProtocol::exit(vector<string> msg) {
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

string StompProtocol::message(vector<string> msg) {
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

    if((tokens[0]!=ch->getName())&((tokens.size()>1)&&(tokens[1]=="wish"))){
            int bookSize=tokens.size()-4;
            string book="";
            for(int i=0;i<bookSize;i++)
                book=book+" "+tokens[4+i];
            book=book.substr(1,book.length());
            if (ch->hasBook(book,topic)){
                string message=ch->getName()+" has "+book;
                out=sendFrame(message,topic);
                 return out;
            }
        }


    if((tokens[0]=="Returning")&((tokens[tokens.size()-1]==ch->getName()))) {
        cout<< ch->getName() + " in returning action"<<endl;
        int bookSize=tokens.size()-3;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[1+i];
        book=book.substr(1,book.length());
        if(ch->isBorrowed(book)){
            cout<<ch->getName() + " is in borrowed need to return to " + ch->getLender(book)<<endl;
            string out = "SEND";
            out=out+"\n"+"destination:"+topic+"\n"+"\n"+"Returning  "+book+ " to " +ch->getLender(book) + "\n";
            ch->addAndRemoveBorrowed(book,"",1);
            return  out;
        }
        ch->addAndRemoveInventory(book, topic, 0);
        ch->addAndRemoveBorrowed(book, "", 1);
    }
    if(tokens[0]=="book"){
        string message=ch->getName()+": ";
        vector <string> inv = ch->getBooks(topic);
        for(string book:inv) {
            message = message + book + ",";
        }
        message=message.substr(0,message.length()-1);
        out=sendFrame(message,topic);
        return out;
    }

    if(tokens[0]=="Taking"&&(tokens[tokens.size()-1]==ch->getName())){
        int bookSize=tokens.size()-3;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[1+i];
        book=book.substr(1,book.length());
        ch->addAndRemoveInventory(book,topic,1);
    }
    if(tokens.size()>1 &&(tokens[1]=="has")) {
        int bookSize=tokens.size()-2;
        string book="";
        for(int i=0;i<bookSize;i++)
            book=book+" "+tokens[2+i];
        book=book.substr(1,book.length());
        if (ch->wished(book)) {
            string message = "Taking " + book + " from " + tokens[0];
            ch->addAndRemoveBorrowed(book, tokens[0], 0);
            ch->addAndRemoveToWish(book,1);
            ch->addAndRemoveInventory(book,topic,0);
            out = sendFrame(message, topic);
            return out;
        }
    }

    return out;
}

string StompProtocol::sendFrame(string msg, string topic) {
    string out="SEND";
    out=out+"\n"+"destination:"+topic+"\n"+"\n"+msg+"\n";
    return out;
}

string StompProtocol::findHeader(string head,vector<string> msg) {
    for (string s:msg){
        int ndx =s.find(":");
        if (s.substr(0,ndx)==head)
            return s.substr(ndx+1, s.size()-1);
    }
    return "";
}