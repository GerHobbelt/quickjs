cmake_minimum_required(VERSION 3.8)
project(quickjs)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_COMPILER_IS_GNUCC)
    add_compile_options(-Wall -Wno-unused-parameter)
endif()

add_subdirectory(quickjs)
add_executable(qjs main.c)
target_link_libraries(qjs quickjs)
