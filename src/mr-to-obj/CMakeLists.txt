cmake_minimum_required(VERSION 3.14)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)
add_executable(mr-to-obj
  mr-to-obj.cpp geometry.cpp colors.cpp geometry.cpp textures.cpp
  output.cpp ${lodepng_SOURCE_DIR}/lodepng.cpp)
target_link_libraries(mr-to-obj mr_parser -static)
target_include_directories(mr-to-obj PRIVATE ${lodepng_SOURCE_DIR})