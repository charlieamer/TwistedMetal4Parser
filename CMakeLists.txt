cmake_minimum_required(VERSION 3.14)

project(TwistedMetal4Parser)

add_subdirectory(lib)

add_executable(mr-info src/mr-info.cpp)
target_link_libraries(mr-info mr_parser)
# set_property(TARGET mr-info PROPERTY CXX_STANDARD 17)

add_executable(mr-to-obj src/mr-to-obj.cpp)
target_link_libraries(mr-to-obj mr_parser)