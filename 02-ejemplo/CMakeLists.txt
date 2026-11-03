cmake_minimum_required(VERSION 3.16)

project(Ejemplo02)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)

add_executable(ejemplo02 main.cpp)
target_link_libraries(ejemplo02 PRIVATE 
    fmt::fmt
    SFML::Graphics SFML::Window SFML::System

)
