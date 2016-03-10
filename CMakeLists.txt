cmake_minimum_required(VERSION 3.2)
project(polygon)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(SFML REQUIRED)
include_directories(${SFML_INCLUDE_DIR})


set(POLYGON_SOURCE
        source/main.cpp
        source/game-state.h source/game.cpp source/game.h source/game-state-start.h source/game-state-editor.h source/game-state-start.cpp source/game-state-editor.cpp source/texture-manager.cpp source/texture-manager.h source/animation.cpp source/animation.h source/animation-handler.cpp source/animation-handler.h source/tile-type.h source/tile-type.cpp source/tile.h source/tile.cpp source/map.cpp source/map.h source/action-state.h source/gui-style.cpp source/gui-style.h source/gui-entry.cpp source/gui-entry.h source/gui.cpp source/gui.h source/city.cpp source/city.h)

add_executable(polygon ${POLYGON_SOURCE})
target_link_libraries(polygon ${SFML_LIBRARIES})