cmake_minimum_required(VERSION 3.11)

project(utils CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


add_library(utils config_parser.cpp)
add_executable(config_parser_example config_parser_example.cpp)
target_link_libraries(config_parser_example utils)

include(GoogleTest)
add_executable(runUnitTests
    gtest_main.cpp
    config_parser_test.cpp
)

find_package(Threads)
find_package(GTest)
target_link_libraries(runUnitTests
    utils
    ${CMAKE_THREAD_LIBS_INIT}
    ${GTEST_BOTH_LIBRARIES}
)

enable_testing()
include(GoogleTest)
add_test(NAME runAllUnitTests COMMAND runUnitTests)
