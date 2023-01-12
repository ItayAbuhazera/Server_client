cmake_minimum_required(VERSION 3.24)
project(Server_client)

set(CMAKE_CXX_STANDARD 14)

include_directories(__MACOSX/SPL231-Assignment3-student-template-2/client/include)
include_directories(SPL231-Assignment3-student-template-2/client/include)

add_executable(Server_client
        __MACOSX/SPL231-Assignment3-student-template-2/client/include/._ConnectionHandler.h
        __MACOSX/SPL231-Assignment3-student-template-2/client/include/._event.h
        __MACOSX/SPL231-Assignment3-student-template-2/client/include/._json.hpp
        __MACOSX/SPL231-Assignment3-student-template-2/client/include/._StompProtocol.h
        __MACOSX/SPL231-Assignment3-student-template-2/client/src/._ConnectionHandler.cpp
        __MACOSX/SPL231-Assignment3-student-template-2/client/src/._echoClient.cpp
        __MACOSX/SPL231-Assignment3-student-template-2/client/src/._event.cpp
        __MACOSX/SPL231-Assignment3-student-template-2/client/src/._StompClient.cpp
        SPL231-Assignment3-student-template-2/client/include/ConnectionHandler.h
        SPL231-Assignment3-student-template-2/client/include/event.h
        SPL231-Assignment3-student-template-2/client/include/json.hpp
        SPL231-Assignment3-student-template-2/client/include/KeyboardThread.h
        SPL231-Assignment3-student-template-2/client/include/StompProtocol.h
        SPL231-Assignment3-student-template-2/client/src/ConnectionHandler.cpp
        SPL231-Assignment3-student-template-2/client/src/echoClient.cpp
        SPL231-Assignment3-student-template-2/client/src/event.cpp
        SPL231-Assignment3-student-template-2/client/src/KeyboardThread.cpp
        SPL231-Assignment3-student-template-2/client/src/StompClient.cpp
        SPL231-Assignment3-student-template-2/client/src/StompProtocol.cpp SPL231-Assignment3-student-template-2/client/src/StompFrame.cpp)
