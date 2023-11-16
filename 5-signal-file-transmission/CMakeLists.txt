cmake_minimum_required(VERSION 3.0)
project(5-signal-file-transmission)

include_directories(Inc/)

add_executable(Rx Src/RxMain.c)
target_link_libraries(Rx PRIVATE TnStatus pthread)

add_executable(Tx Src/TxMain.c)
target_link_libraries(Tx PRIVATE TnStatus pthread)

# Copy benchmarks to build directory
configure_file(Tests/GenTestData.sh ${CMAKE_CURRENT_BINARY_DIR}/GenTestData.sh COPYONLY)