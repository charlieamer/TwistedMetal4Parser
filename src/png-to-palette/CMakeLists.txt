cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)

add_executable(png-to-palette png-to-palette.cpp)
target_link_libraries(png-to-palette mr_parser mr_graphics -static)
target_include_directories(png-to-palette PRIVATE ${lodepng_SOURCE_DIR})