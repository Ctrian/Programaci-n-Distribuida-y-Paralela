cmake_minimum_required(VERSION 3.16)

project(FractalJulia)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)

add_executable(fractal-julia
main.cpp
fractal-serial.cpp
palette.cpp)

target_link_libraries(fractal-julia PRIVATE 
    fmt::fmt
    SFML::Graphics SFML::Window SFML::System
)