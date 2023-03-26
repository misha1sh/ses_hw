cmake_minimum_required(VERSION 3.16)
project(ses_hw)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(hw1)
add_subdirectory(hw2)
add_subdirectory(hw3)
add_subdirectory(hw4)

add_subdirectory(libs/big-number/deps/googletest deps/googletest)
add_subdirectory(libs/big-number/src/big_number)
add_subdirectory(hw5)