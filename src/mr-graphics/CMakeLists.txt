cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)

add_library(mr_graphics colors.cpp textures.cpp geometry.cpp  ${lodepng_SOURCE_DIR}/lodepng.cpp)
target_link_libraries(mr_graphics mr_parser)
target_include_directories(mr_graphics PUBLIC .)
target_include_directories(mr_graphics PRIVATE ${lodepng_SOURCE_DIR})