cmake_minimum_required(VERSION 3.2)
project(polygon)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${polygon_SOURCE_DIR}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static")


find_package(FreeType REQUIRED)
include_directories(${FreeType_INCLUDE_DIR})


set(SOURCE_FILES
        source/main.cpp)

add_executable(polygon ${SOURCE_FILES})
target_link_libraries(polygon ${FreeType_LIBRARY})