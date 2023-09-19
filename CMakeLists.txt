cmake_minimum_required(VERSION 3.0)
project(os-sem3-hw C CXX)

# Dependencies
add_subdirectory(deps/googletest)

# C compile flags
set(CMAKE_C_FLAGS "-Werror -g")

# Projects
add_subdirectory(1-shell-emulator)