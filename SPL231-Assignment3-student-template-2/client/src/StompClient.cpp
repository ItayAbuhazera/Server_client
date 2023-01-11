#include <stdlib.h>
#include "../include/StompProtocol.h"
#include "../include/KeyboardThread.h"
#include <thread>
#include <string>

#include <iostream>;

int main(int argc, char *argv[]) {
	ConnectionHandler* ch = new ConnectionHandler();

    StompProtocol* stompProtocol  = new StompProtocol(*ch);
    KeyboardThread  kbThread(*ch, *stompProtocol);
    std::thread thread(&KeyboardThread::run,&kbThread);
    while(1){
        if(ch -> isLoggedIn()) {
            std::string ans;
            if (!ch -> getFrameAscii(ans, '\0')) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::string out = stompProtocol->processFrame(ans);
            if (out != "")
                if (!ch -> sendFrameAscii(out,'\0')) {
                    std::cout << "Disconnected. Exiting...\n" << std::endl;
                    break;
                }
        }
    }
    thread.join();
    delete(ch);
    delete(stompProtocol);
	return 0;
}