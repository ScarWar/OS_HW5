cmake_minimum_required(VERSION 3.7)
project(OS_HW5)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c linked_list.c linked_list.h msg_slot.c msg_slot.h)
add_executable(OS_HW5 ${SOURCE_FILES})