cmake_minimum_required(VERSION 3.14)

add_executable(mr-diff mr-diff.cpp)
target_link_libraries(mr-diff mr_parser -static)