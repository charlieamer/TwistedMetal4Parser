cmake_minimum_required(VERSION 3.14)

project(TwistedMetal4Parser)

add_subdirectory(lib)

add_executable(mr-info src/main.cpp)

target_link_libraries(mr-info mr_parser)