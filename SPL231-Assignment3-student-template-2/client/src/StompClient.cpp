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

    while(1){
        //Receive
        if(ch->isConnected()){
            std::string msg = "";
            if(ch->getFrameAscii(msg, '\0')){
                StompFrame recFrame(msg);
                stompProtocol->processFrame(recFrame);
                msg.clear();
            }
        }
    }
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

login 192.168.48.1:7777 or or
join /Germany_Japan
report data/events1.json
summary /Germany_Japan or test
*/