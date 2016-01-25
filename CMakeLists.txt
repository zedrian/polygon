cmake_minimum_required(VERSION 3.2)
project(polygon)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static")


find_package(mbedTLS REQUIRED)
include_directories(${mbedTLS_INCLUDE_DIR})


set(NETWORK_SOURCE
        source/acceptor.h
        source/acceptor.cpp
        source/error.h
        source/error.cpp
        source/header.h
        source/header.cpp
        source/message.h
        source/message.cpp
        source/socket.h
        source/socket.cpp)

set(CLIENT_SOURCE_FILES
        source/client-main.cpp
        source/utilities.h
        source/utilities.cpp)

set(SERVER_SOURCE_FILES
        source/server-main.cpp
        source/utilities.h
        source/utilities.cpp)


add_library(network ${NETWORK_SOURCE})

add_executable(client ${CLIENT_SOURCE_FILES})
target_link_libraries(client network ${mbedTLS_LIBRARIES})

add_executable(server ${SERVER_SOURCE_FILES})
target_link_libraries(server network ${mbedTLS_LIBRARIES})