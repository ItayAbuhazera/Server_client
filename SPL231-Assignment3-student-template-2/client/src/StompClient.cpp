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
    bool shouldTerminate = false;
    
    while(!shouldTerminate){
        //Receive
        if(ch->isLoggedIn()){
            std::string msg = "";
            if(ch->getFrameAscii(msg, '\0')){
                StompFrame recFrame(msg);
                shouldTerminate = stompProtocol->processFrame(recFrame);
                msg.clear();
            } else {
                shouldTerminate = true;
            }
        }
    }
    kbThread.terminate();
    thread.join();
    delete(ch);
    delete(stompProtocol);
	return 0;
}