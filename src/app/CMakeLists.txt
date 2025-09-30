add_executable(alpha_wrap2_app main.cpp)
target_link_libraries(alpha_wrap2_app PRIVATE alpha_wrap2)
target_include_directories(alpha_wrap2_app PRIVATE ${CMAKE_SOURCE_DIR}/include)
