cmake_minimum_required(VERSION 3.0)
project(os-sem3-hw C CXX)

# C compile flags
set(CMAKE_C_FLAGS "-Werror -g -O0")
set(CMAKE_CXX_FLAGS "-Werror -g -O0")

# Dependencies
add_subdirectory(deps/googletest)

# Projects
add_subdirectory(1-shell-emulator)
add_subdirectory(2-duplex-pipe)
add_subdirectory(3-ipc-comparison)
add_subdirectory(4-monte-carlo)