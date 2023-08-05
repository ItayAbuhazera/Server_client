#include "../include/KeyboardThread.h"
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <vector>
class ConnectionHandler;

std::queue<StompFrame*> KeyboardThread::sendQueue;

KeyboardThread::KeyboardThread(ConnectionHandler &ch, StompProtocol &protocol):
    mConnectionHandler(&ch), mProtocol(&protocol) {}

KeyboardThread::KeyboardThread(const KeyboardThread &kt):
    mConnectionHandler(kt.mConnectionHandler), mProtocol(kt.mProtocol) {}

KeyboardThread& KeyboardThread::operator=(const KeyboardThread &kt){
    mConnectionHandler = kt.mConnectionHandler;
    mProtocol = kt.mProtocol;
    sendQueue = kt.sendQueue;
    return  *this;
}

KeyboardThread::~KeyboardThread() {
    delete(mConnectionHandler);
    delete(mProtocol);
    clearSendQueue();
}

void KeyboardThread::clearSendQueue(){
    while (!sendQueue.empty()) {
        if (sendQueue.front() != nullptr)
            delete(sendQueue.front());
        sendQueue.pop();
    }
}

void KeyboardThread::sendFrame(StompFrame* frame){
    if (frame != nullptr)
        sendQueue.push(frame);
}

void KeyboardThread::run() {
    while(1) {
        const short bufferSize = 1024;
        char buffer[bufferSize];
        std::cin.getline(buffer, bufferSize);
        std::string line(buffer);
        //StompFrame* frame = mProtocol->processKeyboard(line);

        //Send all frames in queue
        while(!sendQueue.empty() && mConnectionHandler->isConnected()) {
            StompFrame* frame = sendQueue.front();
            if (frame != nullptr) {
                mConnectionHandler->sendFrame(*frame);
                delete(frame);
            }
            sendQueue.pop();
        }

        //Proccess keyboard
        StompFrame* frame = mProtocol->processKeyboard(line);
        if (frame != nullptr && mConnectionHandler->isConnected()) {
            sendQueue.push(frame);
        }
    }
}