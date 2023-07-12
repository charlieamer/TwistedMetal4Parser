cmake_minimum_required(VERSION 3.14)

add_executable(util-explore-types main.cpp)
target_link_libraries(util-explore-types mr_parser -static)