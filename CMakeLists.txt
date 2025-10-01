cmake_minimum_required(VERSION 3.16)
project(alpha_wrap_2 LANGUAGES CXX)

# Default to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_PEDANTIC "Enable extra warnings" ON)
if(ENABLE_PEDANTIC)
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

option(USE_STACK_QUEUE "Use stack as queue instead of priority queue" OFF)

if(USE_STACK_QUEUE)
    target_compile_definitions(alpha_wrap_2 PRIVATE USE_STACK_QUEUE)
    message(STATUS "Using stack as queue")
else()
    message(STATUS "Using priority queue")
endif()

add_subdirectory(src)
