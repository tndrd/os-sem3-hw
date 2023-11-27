cmake_minimum_required(VERSION 3.0)
project(6-backup-daemon)

include_directories(Inc/)

add_library(Logger Src/Logger.c)
target_link_libraries(Logger PUBLIC TnStatus)

add_library(Stage Src/Stage.c)
target_link_libraries(Stage PUBLIC TnStatus)

add_library(StageQueue Src/StageQueue.c)
target_link_libraries(StageQueue PUBLIC TnStatus Logger Stage)

add_library(WDList Src/WatchDescriptorList.c)
target_link_libraries(WDList PUBLIC TnStatus Logger)

add_library(WDMap Src/WatchDescriptorMap.c)
target_link_libraries(WDMap PUBLIC WDList)

add_library(DWatcher Src/DirectoryWatcher.c)
target_link_libraries(DWatcher PUBLIC WDMap StageQueue pthread)

add_executable(DWTest Tests/DWTest.c)
target_link_libraries(DWTest PRIVATE DWatcher)