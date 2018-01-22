cmake_minimum_required(VERSION 3.6)
project(csv2xml)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
add_executable(csv2xml ${SOURCE_FILES})
target_link_libraries(csv2xml popt)
