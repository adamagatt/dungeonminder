cmake_minimum_required(VERSION 3.24)

project(DungeonMinder)

find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 20)

add_executable(DungeonMinder
    src/DungeonMinder.cpp
    src/game_state.cpp
    src/hero.cpp
    src/utils.cpp
)

target_link_libraries(DungeonMinder tcod ${SDL2_LIBRARIES})

configure_file(terminal.png terminal.png COPYONLY)