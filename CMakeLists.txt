cmake_minimum_required(VERSION 3.11)

project(utils CXX)
enable_testing()

find_package(Threads)
find_package(GTest)
include(GoogleTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# config parser
add_library(config_parser_lib config_parser.cpp)
add_executable(config_parser_example config_parser_example.cpp)
target_link_libraries(config_parser_example config_parser_lib)

add_executable(config_parser_test gtest_main.cpp config_parser_test.cpp)
target_link_libraries(config_parser_test config_parser_lib ${CMAKE_THREAD_LIBS_INIT} ${GTEST_BOTH_LIBRARIES})
add_test(NAME run_config_parser_test COMMAND config_parser_test)

# argument parser
add_library(argument_parser_lib argument_parser.cpp)
add_executable(argument_parser_example argument_parser_example.cpp)
target_link_libraries(argument_parser_example argument_parser_lib)

add_executable(argument_parser_test gtest_main.cpp argument_parser_test.cpp)
target_link_libraries(argument_parser_test argument_parser_lib ${CMAKE_THREAD_LIBS_INIT} ${GTEST_BOTH_LIBRARIES})
add_test(NAME run_argument_parser_test COMMAND argument_parser_test)