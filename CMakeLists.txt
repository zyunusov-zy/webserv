cmake_minimum_required(VERSION 3.25)
project(our_webserv)

set(CMAKE_CXX_STANDARD 14)

include_directories(Config)

add_executable(our_webserv
        Config/Client.cpp
        Config/Client.hpp
        Config/Config.cpp
        Config/Config.hpp
        Config/ErrorCodes.cpp
        Config/ErrorCodes.hpp
        Config/Location.cpp
        Config/Location.hpp
        Config/main.cpp
        Config/mainIn.hpp
        Config/Request.cpp
        Config/Request.hpp
        Config/Response.cpp
        Config/Response.hpp
        Config/Server.cpp
        Config/Server.hpp
        Config/Server2.cpp)
