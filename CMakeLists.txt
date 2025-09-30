cmake_minimum_required(VERSION 3.16)
project(alpha_wrap_2 LANGUAGES CXX)

# Default to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_PEDANTIC "Enable extra warnings" ON)
if(ENABLE_PEDANTIC)
	add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_subdirectory(src)
