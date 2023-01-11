#include <sstream>
#include <vector>
#include <map>
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
using namespace std;

StompProtocol::StompProtocol(ConnectionHandler& ch): mDisconnectRec(0), mReceiptCounter(1), mSubId(0), mConnectionHandler(&ch), 
                                        subscriptionsByTopic(unordered_map<string,int>()),
                                        subscriptionsById(unordered_map<int,string>()),
                                        receipts(unordered_map<int,pair<string,bool>>())
{
    initCommands();
}

void StompProtocol::initCommands(){
    //Client
    commands["login"] = 0;
    commands["connect"] = 1;
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

string StompProtocol::processKeyboard(string msg) {
    vector<string> tokens;
    while(msg != ""){
        string nextToken;
        if(msg.find(" ") == std::string::npos) {
            nextToken = msg;
            tokens.push_back(nextToken);
            break;
        }

        nextToken = msg.substr(0, msg.find(" "));
        tokens.push_back(nextToken);
        msg = msg.substr(msg.find(" ") + 1 , msg.length());
    }
    string out = "";
    switch (commands[tokens[0]]) {

        //Login
        case 0:
        for(string token : tokens){
            out += token + " ";
        }
        out.pop_back(); //remove last whitespace
        break;

        //Connecet
		case 1:
			out = "CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + tokens[1] + "\npasscode:" + tokens[2] + "\n\n\0";
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
			out = "SEND\ndestination:" + tokens[1] + "\n\n"+tokens[2] + "\n\0";
		    break;

        //Disconnect
		case 4:
			out = "DISCONNECT\nreceipt:" + to_string(mReceiptCounter) + "\n\n\0";
			mReceiptCounter++;
		    break;

        //Defualt - Invalid
		default:
			break;

        return out;
	}
}

string StompProtocol::processFrame(string msg) {
    string command;
    map<string, string> headers;
    string body;

    //Command
    int idx = msg.find("\n");
    command = msg.substr(0, idx);
    msg = msg.erase(0, idx);

    //Headers
    while(msg.substr(0, 2) != "\n\n"){
        idx = msg.find("\n");
        string line = msg.substr(0, idx);
        headers[line.substr(0, line.find(":"))] = line.substr(line.find(":") + 1);
        msg = msg.erase(0, idx);
    }

    //Body
    body = msg.substr(0, msg.length() - 1);

    string out = "";
    switch(commands[command]){

        //Message
        case -1:
        out = message(body);
        break;

        //Receipt
        case -2:
        receipt(tokens);
        break;

        //Connected
        case -3:
        std::cout << "Connected" << std::endl;
        break;

        //Error
        case -4:
        error(tokens);
        break;

        //Defualt - Invalid (Error)
        default:
        error(tokens);
        break;
    }
    return out;
}

void StompProtocol::error(vector<string> tokens) {
    cout<<"Error"<<endl;
    cout<<tokens[tokens.size()-1]<<endl;
    mConnectionHandler -> disconnecting();
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
        }
        mConnectionHandler -> connecting();
        mConnectionHandler -> setName(msg[2]);
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
    if (reccc==mDisconnectRec){
        mConnectionHandler -> disconnecting();
        return;
    }
    if(getReceipts().count(reccc) != 0){
        if(getReceipts().at(reccc).second)
            cout<<"Joined club "+ getReceipts().at(reccc).first<<endl;
        else
            cout<<"Exited club "+getReceipts().at(reccc).first<<endl;
    }
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

/* string StompProtocol::addBook(vector<string> msg) {
    int bookSize=msg.size()-2;
    string book="";
    for(int i=0;i<bookSize;i++)
        book=book+" "+msg[2+i];
    book=book.substr(1,book.length());
    string out = "SEND";
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+ch->getName()+" has added the book "+book+"\n"+ "\0";
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
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+ch->getName()+" wish to borrow "+book+"\n"+ "\0";
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
    out=out+"\n"+"destination:"+msg[1]+"\n"+"\n"+"Returning "+book+ " to " +ch->getLender(book) + "\n"+ "\0";
    ch->addAndRemoveBorrowed(book,"",1);
    ch->addAndRemoveInventory(book,msg[1],1);
    return  out;
} */

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

// string StompProtocol::message(vector<string> msg) {
//     vector<string> tokens;
//     string out="";
//     string topic = look4Header("destination", msg);
//     string message = msg[msg.size()-1];
//     cout<<topic+":"+message<<endl;
//     while(message!=""){
//         string tmp;
//         if(message.find(" ")==std::string::npos) {
//             tmp = message;
//             tokens.push_back(tmp);
//             break;
//         }
//         tmp=message.substr(0,message.find(" "));
//         tokens.push_back(tmp);
//         message=message.substr(message.find(" ")+1,message.length());
//     }

    // if((tokens[0]!=mConnectionHandler->getName())&((tokens.size()>1)&&(tokens[1]=="wish"))){
    //         int bookSize=tokens.size()-4;
    //         string book="";
    //         for(int i=0;i<bookSize;i++)
    //             book=book+" "+tokens[4+i];
    //         book=book.substr(1,book.length());
    //         if (mConnectionHandler->hasBook(book,topic)){
    //             string message=mConnectionHandler->getName()+" has "+book;
    //             out=sendFrame(message,topic);
    //              return out;
    //         }
    //     }


    // if((tokens[0]=="Returning")&((tokens[tokens.size()-1]==mConnectionHandler->getName()))) {
    //     cout<< mConnectionHandler->getName() + " in returning action"<<endl;
    //     int bookSize=tokens.size()-3;
    //     string book="";
    //     for(int i=0;i<bookSize;i++)
    //         book=book+" "+tokens[1+i];
    //     book=book.substr(1,book.length());
    //     if(mConnectionHandler->isBorrowed(book)){
    //         cout<<mConnectionHandler->getName() + " is in borrowed need to return to " + mConnectionHandler->getLender(book)<<endl;
    //         string out = "SEND";
    //         out=out+"\n"+"destination:"+topic+"\n"+"\n"+"Returning  "+book+ " to " +mConnectionHandler->getLender(book) + "\n";
    //         mConnectionHandler->addAndRemoveBorrowed(book,"",1);
    //         return  out;
    //     }
    //     mConnectionHandler->addAndRemoveInventory(book, topic, 0);
    //     mConnectionHandler->addAndRemoveBorrowed(book, "", 1);
    // }
    // if(tokens[0]=="book"){
    //     string message=mConnectionHandler->getName()+": ";
    //     vector <string> inv = mConnectionHandler->getBooks(topic);
    //     for(string book:inv) {
    //         message = message + book + ",";
    //     }
    //     message=message.substr(0,message.length()-1);
    //     out=send(message,topic);
    //     return out;
    // }

    // if(tokens[0]=="Taking"&&(tokens[tokens.size()-1]==mConnectionHandler->getName())){
    //     int bookSize=tokens.size()-3;
    //     string book="";
    //     for(int i=0;i<bookSize;i++)
    //         book=book+" "+tokens[1+i];
    //     book=book.substr(1,book.length());
    //     mConnectionHandler->addAndRemoveInventory(book,topic,1);
    // }
    // if(tokens.size()>1 &&(tokens[1]=="has")) {
    //     int bookSize=tokens.size()-2;
    //     string book="";
    //     for(int i=0;i<bookSize;i++)
    //         book=book+" "+tokens[2+i];
    //     book=book.substr(1,book.length());
    //     if (mConnectionHandler->wished(book)) {
    //         string message = "Taking " + book + " from " + tokens[0];
    //         mConnectionHandler->addAndRemoveBorrowed(book, tokens[0], 0);
    //         mConnectionHandler->addAndRemoveToWish(book,1);
    //         mConnectionHandler->addAndRemoveInventory(book,topic,0);
    //         out = send(message, topic);
    //         return out;
    //     }
//     }

//     return out;
// }

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