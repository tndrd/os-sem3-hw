cmake_minimum_required(VERSION 3.0)
project(4-monte-carlo C CXX)

include_directories(Inc/)

add_library(TnStatus Src/TnStatus.c)
add_library(MCAlgorithm Src/MCAlgorithm.c)

add_library(WorkerContent Src/WorkerContent.c)
target_link_libraries(WorkerContent PUBLIC TnStatus)

add_library(Worker Src/Worker.c)
target_link_libraries(Worker PUBLIC WorkerContent pthread)

add_library(Driver Src/Driver.c)
target_link_libraries(Driver PUBLIC Worker MCAlgorithm)

add_executable(${PROJECT_NAME} Src/Main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE Driver)

add_executable(Test Tests/Test.cpp)
target_link_libraries(Test PRIVATE Driver GTest::gtest_main)