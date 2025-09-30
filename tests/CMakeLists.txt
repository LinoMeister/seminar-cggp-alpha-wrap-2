include(FetchContent)
FetchContent_Declare(
  catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(catch2)

add_executable(test_alpha_wrap tests_unit.cpp)
target_link_libraries(test_alpha_wrap PRIVATE alpha_wrap2 Catch2::Catch2WithMain)
target_include_directories(test_alpha_wrap PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_test(NAME alpha_wrap_tests COMMAND test_alpha_wrap)
