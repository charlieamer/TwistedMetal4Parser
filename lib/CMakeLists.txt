cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
)
FetchContent_MakeAvailable(zlib)

add_library(mr_parser src/namable.cpp src/component.cpp src/node.cpp src/utils.cpp)
target_include_directories(mr_parser PUBLIC include)
target_include_directories(mr_parser PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
target_link_libraries(mr_parser zlibstatic)
target_link_directories(mr_parser PRIVATE ${zlib_BINARY_DIR})
target_include_directories(example PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
target_include_directories(example64 PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
target_include_directories(minigzip PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
target_include_directories(minigzip64 PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})