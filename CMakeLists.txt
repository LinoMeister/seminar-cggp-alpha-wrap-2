cmake_minimum_required(VERSION 3.16)
project(alpha_wrap_2)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CGAL REQUIRED)


file(GLOB SRC_FILES src/alpha_wrap_2/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(alpha_wrap_2 PRIVATE Eigen3::Eigen)
target_link_libraries(alpha_wrap_2 PRIVATE CGAL::CGAL)
