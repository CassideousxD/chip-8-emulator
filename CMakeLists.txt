cmake_minimum_required(VERSION 3.20)

project(CHIP8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_ttf REQUIRED)

add_executable(CHIP8
    "source code/main.cpp"
    "source code/chip8.cpp"
)

target_link_libraries(
    CHIP8
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
)