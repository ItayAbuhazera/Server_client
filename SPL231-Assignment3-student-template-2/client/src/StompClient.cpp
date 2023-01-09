#include <stdlib.h>
#include "../include/StompProtocol.h"
//#include "../include/KeyboardThread.h"
#include <thread>
#include <string>


#include <iostream>;

int main(int argc, char *argv[]) {
	ConnectionHandler* ch = new ConnectionHandler();

    StompProtocol* stompProtocol  = new StompProtocol(*ch);
    KeyboardThread  keyThread(*ch, *stompProtocol);
    std::thread th(&KeyboardThread::run,&keyThread);
    while(1){
        if(ch->isConnected()) {
            std::string ans;
            if (!ch->getFrameAscii(ans, '\0')) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::string out = stompProtocol->processFrame(ans);
            if (out != "")
                if (!ch->sendFrameAscii(out,'\0')) {
                    std::cout << "Disconnected. Exiting...\n" << std::endl;
                    break;
                }
        }
    }
    th.join();
    delete(ch);
    delete(stompProtocol);
	return 0;
}