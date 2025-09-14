cmake_minimum_required(VERSION 4.0)
project(ghettp)

set(CMAKE_CXX_STANDARD 23)

add_library(ghettp SHARED source/ghettp.cpp
        source/socket.cpp
        include/socket.hpp)
