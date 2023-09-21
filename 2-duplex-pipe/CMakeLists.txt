cmake_minimum_required(VERSION 3.0)
project(2-duplex-pipe C CXX)

include_directories(Inc/)

add_library(DuplexPipe Src/DuplexPipe.c)

add_executable(${PROJECT_NAME} Src/Main.c)
target_link_libraries(${PROJECT_NAME} PRIVATE DuplexPipe)