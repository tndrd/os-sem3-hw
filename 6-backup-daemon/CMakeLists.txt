cmake_minimum_required(VERSION 3.0)
project(6-backup-daemon CXX)

include_directories(Inc/)

add_library(HwBackupException Src/HwBackupException.cpp)
add_library(DescriptorWrapper Src/DescriptorWrapper.cpp)

add_library(WDCache Src/WDCache.cpp)
target_link_libraries(WDCache PUBLIC HwBackupException)

add_library(PThreadWrapper Src/PThreadWrapper.cpp)
target_link_libraries(PThreadWrapper PUBLIC pthread HwBackupException)

add_library(Logger Src/Logger.cpp)
target_link_libraries(Logger PUBLIC PThreadWrapper)

add_library(Stage Src/Stage.cpp)

add_library(FSMonitor Src/FSMonitor.cpp)
target_link_libraries(FSMonitor PUBLIC WDCache DescriptorWrapper Logger Stage)

add_executable(FSMTest Test/FSMTest.cpp)
target_link_libraries(FSMTest PUBLIC FSMonitor)


