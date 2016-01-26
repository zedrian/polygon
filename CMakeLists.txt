cmake_minimum_required(VERSION 3.3)
project(polygon)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


set(MATH_SOURCE
        source/stupid.h
        source/stupid.cpp)

add_library(math SHARED ${MATH_SOURCE})


set(NETWORK_SOURCE
        source/socket.h
        source/socket.cpp source/socket-implementation.cpp source/socket-implementation.h)

add_library(network SHARED ${NETWORK_SOURCE})


set(SOURCE_FILES
        source/main.cpp)

add_executable(polygon ${SOURCE_FILES})
target_link_libraries(polygon math network)