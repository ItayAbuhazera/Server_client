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
        if(ch->isLoggedIn()){
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
login 172.24.144.1:7777 or or
*/