cmake_minimum_required(VERSION 3.29)
project(SO2 C)

set(CMAKE_C_STANDARD 90)

add_executable(SO2 src/main.c
        src/includes/unicode8.h
        src/modules/unicode8.c
        src/includes/formatting.h
        src/modules/formatting.c
        src/includes/output.h
        src/modules/output.c)
