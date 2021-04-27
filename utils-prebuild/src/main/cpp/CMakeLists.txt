
cmake_minimum_required(VERSION 3.10.2)

project("cmake-utils-lib")

add_library(cmake-utils-lib SHARED utils.cpp)

target_link_libraries(cmake-utils-lib)
