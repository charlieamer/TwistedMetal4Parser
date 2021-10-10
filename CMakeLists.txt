cmake_minimum_required(VERSION 3.14)

project(TwistedMetal4Parser)

add_subdirectory(lib)

add_executable(mr-unzip src/mr-info.cpp)
target_link_libraries(mr-unzip mr_parser)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)
add_executable(mr-to-obj src/mr-to-obj.cpp ${lodepng_SOURCE_DIR}/lodepng.cpp)
target_link_libraries(mr-to-obj mr_parser)
target_include_directories(mr-to-obj PRIVATE ${lodepng_SOURCE_DIR})