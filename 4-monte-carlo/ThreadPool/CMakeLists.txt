cmake_minimum_required(VERSION 3.0)
project(ThreadPool C CXX)

include_directories(Inc/)

add_library(Worker Src/Worker.c)
target_link_libraries(Worker PUBLIC pthread)

add_library(WorkerArray Src/WorkerArray.c)
target_link_libraries(WorkerArray PUBLIC Worker)

add_library(WorkerQueue Src/WorkerQueue.c)
target_link_libraries(WorkerQueue PUBLIC Worker)

add_library(WQMonitor Src/WQMonitor.c)
target_link_libraries(WQMonitor PUBLIC WorkerQueue pthread)

add_library(TaskQueue Src/TaskQueue.c)
target_link_libraries(TaskQueue PUBLIC Worker)

add_library(TQMonitor Src/TQMonitor.c)
target_link_libraries(TQMonitor PUBLIC TaskQueue pthread)

add_library(ThreadPool Src/ThreadPool.c)
target_link_libraries(ThreadPool PUBLIC TQMonitor WQMonitor WorkerArray)

set(TEST_EXECUTABLE ${PROJECT_NAME}_RunTests)

add_executable(${TEST_EXECUTABLE} Tests/RunTests.cpp)
target_link_libraries(${TEST_EXECUTABLE} PRIVATE ThreadPool GTest::gtest_main)