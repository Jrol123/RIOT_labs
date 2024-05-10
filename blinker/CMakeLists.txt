cmake_minimum_required(VERSION 3.28)
project(blinker C)

set(CMAKE_C_STANDARD 11)

add_executable(blinker main.c)
include_directories(\\wsl.localhost/Ubuntu/home/artem/RIOT)