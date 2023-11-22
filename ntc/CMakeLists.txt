cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(NTC_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(source)
add_subdirectory(exc)
