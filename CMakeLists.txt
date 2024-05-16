cmake_minimum_required(VERSION 2.8.12...4.0)
project(quickjs C)
include(${CMAKE_CURRENT_LIST_DIR}/quickjs.cmake)
add_subdirectory(${QUICKJS_SOURCE_DIR} ${QUICKJS_BINARY_DIR})
