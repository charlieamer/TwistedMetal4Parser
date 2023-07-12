cmake_minimum_required(VERSION 3.14)
add_executable(mr-to-obj mr-to-obj.cpp output.cpp)
target_link_libraries(mr-to-obj mr_parser mr_graphics -static)