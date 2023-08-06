#include <stdlib.h>
#include "../include/StompProtocol.h"
#include "../include/KeyboardThread.h"
#include <thread>
#include <string>

#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Client Starting" << endl;
	ConnectionHandler* ch = new ConnectionHandler();
    StompProtocol* stompProtocol  = new StompProtocol(*ch);
    KeyboardThread  kbThread(*ch, *stompProtocol);
    std::thread thread(&KeyboardThread::run, &kbThread);

    //Receive
    while(1){
        if(ch->isConnected()){
            std::string msg = "";
            if(ch->getFrameAscii(msg, '\0')){
                StompFrame recFrame(msg);
                stompProtocol->processFrame(recFrame);
                msg.clear();
            }
        }
    }

    //Exit Client
    ch->setLoggedIn(false);
    thread.join();
    ch->close();
    std::cout << "Disconnected" << std::endl;
    delete(ch);
    delete(stompProtocol);
	return 0;
}

/*
ip route show default

login 172.28.0.1:7777 or or
join /Germany_Japan
report data/events1.json
summary /Germany_Japan or test
*/