#include "../include/KeyboardThread.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
class ConnectionHandler;

KeyboardThread::KeyboardThread(ConnectionHandler &ch, StompProtocol &protocol): mConnectionHandler(&ch),mProtocol(&protocol), shouldTerminate(false) {}

KeyboardThread::KeyboardThread(const KeyboardThread &kt): mConnectionHandler(kt.mConnectionHandler), mProtocol(kt.mProtocol), shouldTerminate(false) {}

KeyboardThread& KeyboardThread::operator=(const KeyboardThread &kt){
    mConnectionHandler = kt.mConnectionHandler;
    mProtocol = kt.mProtocol;
    return  *this;
}
KeyboardThread::~KeyboardThread() {
    delete(mConnectionHandler);
    delete(mProtocol);
}

void KeyboardThread::terminate(){
    shouldTerminate = true;
}

void KeyboardThread::run() {
    while(!shouldTerminate) {
        const short bufferSize = 1024;
        char buffer[bufferSize];
        std::cin.getline(buffer, bufferSize);
        std::string line(buffer);
        const std::string out = mProtocol -> processKeyboard(line);
        if(out != "" && mConnectionHandler->isLoggedIn() && !shouldTerminate) {
            mConnectionHandler -> sendFrameAscii(out, '\0');
        }
    }
}