cmake_minimum_required(VERSION 3.16)
project(ghettp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_library(ghettp SHARED
    source/socket.cpp
    source/ghettp.cpp
)

target_include_directories(ghettp PUBLIC include)
target_link_libraries(ghettp Threads::Threads)

add_executable(ghettp_example example/main.cpp)
target_link_libraries(ghettp_example ghettp)

set_target_properties(ghettp PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
)
