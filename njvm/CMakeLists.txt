cmake_minimum_required(VERSION 3.25)
project(njvm C)

set(CMAKE_C_STANDARD 99)
add_compile_options(-g -Wall -pedantic -o)
add_executable(njvm njvm.c)
