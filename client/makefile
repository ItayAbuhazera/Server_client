CFLAGS:=-c -Wall -Weffc++ -g -std=c++11 -Iinclude
LDFLAGS:=-lboost_system -lpthread

all: clean Client

Client: bin/ConnectionHandler.o bin/KeyboardThread.o bin/StompClient.o bin/StompFrame.o bin/StompProtocol.o bin/event.o
	g++ -o bin/Client bin/ConnectionHandler.o bin/KeyboardThread.o bin/StompClient.o bin/StompFrame.o bin/StompProtocol.o bin/event.o $(LDFLAGS)

bin/StompClient.o: src/StompClient.cpp
	g++ $(CFLAGS) -o bin/StompClient.o src/StompClient.cpp

bin/StompProtocol.o: src/StompProtocol.cpp
	g++ $(CFLAGS) -o bin/StompProtocol.o src/StompProtocol.cpp

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/KeyboardThread.o: src/KeyboardThread.cpp
	g++ $(CFLAGS) -o bin/KeyboardThread.o src/KeyboardThread.cpp

bin/event.o: src/event.cpp
	g++ $(CFLAGS) -o bin/event.o src/event.cpp

bin/StompFrame.o: src/StompFrame.cpp
	g++ $(CFLAGS) -o bin/StompFrame.o src/StompFrame.cpp

.PHONY: clean
clean:
	rm -f bin/*
	