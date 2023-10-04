cmake_minimum_required(VERSION 3.0)
project(4-monte-carlo C)

include_directories(Inc/)

add_library(TnStatus Src/TnStatus.c)
add_library(MCAlgorithm Src/MCAlgorithm.c)

add_library(WorkerContent Src/WorkerContent.c)
target_link_libraries(WorkerContent PUBLIC TnStatus)

add_library(Worker Src/Worker.c)
target_link_libraries(Worker PUBLIC WorkerContent pthread)

add_executable(${PROJECT_NAME} Src/Main.c)
target_link_libraries(${PROJECT_NAME} PUBLIC Worker MCAlgorithm)