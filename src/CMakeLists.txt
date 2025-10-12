cmake_minimum_required(VERSION 3.16)

# Library target
file(GLOB AW2_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/alpha_wrap_2/*.cpp
)

# Exclude main.cpp from the library sources (app will build it)
list(FILTER AW2_SOURCES EXCLUDE REGEX ".*main.cpp$")

add_library(alpha_wrap2 STATIC ${AW2_SOURCES})

target_include_directories(alpha_wrap2
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/alpha_wrap_2
)

target_compile_features(alpha_wrap2 PUBLIC cxx_std_17)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CGAL REQUIRED)

target_link_libraries(alpha_wrap2 PRIVATE Eigen3::Eigen CGAL::CGAL)

# Apply preprocessor definitions based on options
if(USE_STACK_QUEUE)
    target_compile_definitions(alpha_wrap2 PUBLIC USE_STACK_QUEUE)
    message(STATUS "Using stack as queue")
else()
    message(STATUS "Using priority queue")
endif()

if(MODIFIED_ALPHA_TRAVERSABILITY)
    target_compile_definitions(alpha_wrap2 PUBLIC MODIFIED_ALPHA_TRAVERSABILITY)
    message(STATUS "Modified alpha traversability algorithm enabled")
else()
    message(STATUS "Standard alpha traversability algorithm")
endif()

add_subdirectory(app)
