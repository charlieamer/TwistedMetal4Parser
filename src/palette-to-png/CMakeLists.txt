cmake_minimum_required(VERSION 3.14)
add_executable(palette-to-png palette-to-png.cpp)
target_link_libraries(palette-to-png mr_parser mr_graphics -static)