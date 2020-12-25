cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
)
FetchContent_MakeAvailable(zlib)

project(TwistedMetal4Parser)

# message(WARNING ${zlib_SOURCE_DIR})
# message(WARNING ${zlib_BINARY_DIR})
# add_subdirectory(${zlib_SOURCE_DIR})
# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
    # message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

add_executable(mr-info src/main.cpp)
target_link_libraries(mr-info zlibstatic)
target_include_directories(mr-info PRIVATE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})