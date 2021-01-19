cmake_minimum_required(VERSION 3.14)

project(TwistedMetal4Parser)

add_subdirectory(lib)

add_executable(mr-info src/mr-info.cpp)
target_link_libraries(mr-info mr_parser)
# set_property(TARGET mr-info PROPERTY CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)
add_executable(mr-to-obj src/mr-to-obj.cpp ${lodepng_SOURCE_DIR}/lodepng.cpp)
target_link_libraries(mr-to-obj mr_parser)
target_include_directories(mr-to-obj PRIVATE ${lodepng_SOURCE_DIR})