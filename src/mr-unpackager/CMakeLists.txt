cmake_minimum_required(VERSION 3.14)

add_executable(mr-unpackager mr-unpackager.cpp)
target_link_libraries(mr-unpackager mr_parser -static)