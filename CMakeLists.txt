cmake_minimum_required(VERSION 3.3)
project(polygon)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static")


set(SOURCE_FILES
        source/main.cpp)

add_executable(polygon ${SOURCE_FILES})