cmake_minimum_required(VERSION 3.14)

add_executable(mr-packager mr-packager.cpp)
target_link_libraries(mr-packager mr_parser -static)