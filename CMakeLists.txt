cmake_minimum_required(VERSION 3.14)

project(TwistedMetal4Parser)

add_subdirectory(lib)

add_executable(mr-unpackager src/mr-unpackager.cpp)
target_link_libraries(mr-unpackager mr_parser -static)

add_executable(mr-packager src/mr-packager.cpp)
target_link_libraries(mr-packager mr_parser -static)

add_executable(mr-diff src/mr-diff.cpp)
target_link_libraries(mr-diff mr_parser -static)

include(FetchContent)
FetchContent_Declare(
  lodepng
  GIT_REPOSITORY https://github.com/lvandeve/lodepng.git
)
FetchContent_MakeAvailable(lodepng)
add_executable(mr-to-obj src/mr-to-obj.cpp ${lodepng_SOURCE_DIR}/lodepng.cpp)
target_link_libraries(mr-to-obj mr_parser -static)
target_include_directories(mr-to-obj PRIVATE ${lodepng_SOURCE_DIR})