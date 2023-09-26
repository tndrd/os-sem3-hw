cmake_minimum_required(VERSION 3.0)
project(3-ipc-comparison C CXX)

include_directories(Inc/)

add_library(Helpers Src/IPC/Helpers.c)
add_library(IPCStatus Src/IPC/IPCStatus.c)

add_library(FIFOTx Src/IPC/FIFOTx.c)
target_link_libraries(FIFOTx PRIVATE Helpers)

add_library(FIFORx Src/IPC/FIFORx.c)
target_link_libraries(FIFORx PRIVATE Helpers)

add_library(ShMemTx Src/IPC/ShMemTx.c)
target_link_libraries(ShMemTx PRIVATE Helpers)

add_library(ShMemRx Src/IPC/ShMemRx.c)
target_link_libraries(ShMemRx PRIVATE Helpers)

add_library(MQTx Src/IPC/MQTx.c)
target_link_libraries(MQTx PRIVATE Helpers)

add_library(MQRx Src/IPC/MQRx.c)
target_link_libraries(MQRx PRIVATE Helpers)

add_executable(Fifo Src/Fifo.cpp)
target_link_libraries(Fifo PRIVATE FIFOTx FIFORx IPCStatus)

add_executable(Shm Src/Shm.cpp)
target_link_libraries(Shm PRIVATE ShMemTx ShMemRx IPCStatus)

add_executable(Msg Src/Msg.cpp)
target_link_libraries(Msg PRIVATE MQTx MQRx IPCStatus)