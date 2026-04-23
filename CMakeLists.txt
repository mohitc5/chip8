cmake_minimum_required(VERSION 3.16)
project(chip8 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# SDL2 — uncomment for Phase 4. On WSL/Ubuntu: sudo apt install libsdl2-dev
find_package(SDL2 CONFIG REQUIRED)

add_executable(chip8
    src/main.cpp
    src/chip8.cpp
    src/platform.cpp  # uncomment for Phase 4
)

target_link_libraries(chip8 PRIVATE SDL2::SDL2 SDL2::SDL2main)
