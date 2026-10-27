cmake_minimum_required(VERSION 3.16)

project(Ejemplo00)

find_package(fmt CONFIG REQUIRED)

add_executable(ejemplo00 main.cpp)
target_link_libraries(ejemplo00 PRIVATE fmt::fmt)