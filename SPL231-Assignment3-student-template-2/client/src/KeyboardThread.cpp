#include "../include/KeyboardThread.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
class ConnectionHandler;

KeyboardThread::KeyboardThread(ConnectionHandler &ch, StompProtocol &protocol): mConnectionHandler(&ch), mProtocol(&protocol) {}

KeyboardThread::KeyboardThread(const KeyboardThread &kt): mConnectionHandler(kt.mConnectionHandler), mProtocol(kt.mProtocol) {}

KeyboardThread& KeyboardThread::operator=(const KeyboardThread &kt){
    mConnectionHandler = kt.mConnectionHandler;
    mProtocol = kt.mProtocol;
    return  *this;
}
KeyboardThread::~KeyboardThread() {
    delete(mConnectionHandler);
    delete(mProtocol);
}

void KeyboardThread::run() {
    while(1) {
        const short bufferSize = 1024;
        char buffer[bufferSize];
        std::cin.getline(buffer, bufferSize);
        std::string line(buffer);
        const std::string out = mProtocol -> processKeyboard(line);
        if(out != "") {
            mConnectionHandler -> sendFrameAscii(out, '\0');
        }
    }
}