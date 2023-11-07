cmake_minimum_required(VERSION 3.0)
project(4-monte-carlo C CXX)

add_subdirectory(ThreadPool/)
include_directories(Inc/)

add_library(MonteCarlo Src/MonteCarlo.c)

add_library(MonteCarloMT Src/MonteCarloMT.c)
target_link_libraries(MonteCarloMT PUBLIC MonteCarlo ThreadPool)

add_library(Driver Src/Driver.c)
target_link_libraries(Driver PUBLIC MonteCarloMT)

add_executable(${PROJECT_NAME} Src/Main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE Driver)

set(TEST_EXECUTABLE ${PROJECT_NAME}_RunTests)

add_executable(${TEST_EXECUTABLE} Tests/RunTests.cpp)
target_link_libraries(${TEST_EXECUTABLE} PRIVATE GTest::gtest_main Driver)