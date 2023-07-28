cmake_minimum_required(VERSION 3.24)
project(Server_client)

set(CMAKE_CXX_STANDARD 14)
include_directories(SPL231-Assignment3-student-template-2/client/include)
include_directories(D:/boost_1_81_0/include)
link_directories(D:/boost_1_81_0/lib)


add_executable(Server_client
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
