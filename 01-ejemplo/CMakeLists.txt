cmake_minimum_required(VERSION 3.16)

project(Ejemplo01)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)

add_executable(ejemplo01 main.cpp)
target_link_libraries(ejemplo01 PRIVATE
fmt::fmt
SFML::Network SFML::Graphics SFML::Window SFML::Audio SFML::System
)