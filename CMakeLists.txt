cmake_minimum_required(VERSION 3.1)

project(quickjs C CXX)

set(QUICKJS_SOURCES
    quickjs.c
    libregexp.c
    libunicode.c
    libbf.c
    cutils.c
    quickjs-libc.c
)

file(STRINGS "VERSION" QUICKJS_VERSION)

message(STATUS "Identified QuickJS ${QUICKJS_VERSION}")

add_library(${PROJECT_NAME} ${QUICKJS_SOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_VERSION="${QUICKJS_VERSION}")
target_compile_definitions(${PROJECT_NAME} PUBLIC JS_STRICT_NAN_BOXING=true)
target_compile_definitions(${PROJECT_NAME} PUBLIC CONFIG_BIGNUM=true)
target_include_directories(${PROJECT_NAME} PUBLIC .)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_executable(qjsc ${QUICKJS_SOURCES} qjsc.c)
    target_link_libraries(qjsc PRIVATE ${PROJECT_NAME})
    target_link_libraries(qjsc PRIVATE m)

    add_executable(qjs ${QUICKJS_SOURCES} qjs.c repl.c qjscalc.c)
    target_link_libraries(qjs PRIVATE ${PROJECT_NAME})
    target_link_libraries(qjs PRIVATE m)
endif()
