cmake_minimum_required(VERSION 3.9)
project(Semaphores C)

set(CMAKE_C_STANDARD 99)

add_executable(Semaphores main.c semaphore.h semaphore.c)