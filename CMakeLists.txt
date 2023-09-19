cmake_minimum_required(VERSION 3.25)

project(quickjs C)

file(STRINGS VERSION quickjs_version)

add_library(quickjs OBJECT)

set_target_properties(
  quickjs
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  quickjs
  INTERFACE
    quickjs.h
  PRIVATE
    quickjs.c
    libregexp.c
    libunicode.c
    libbf.c
    cutils.c
)

target_include_directories(
  quickjs
  PUBLIC
    .
)

target_compile_definitions(
  quickjs
  PUBLIC
    CONFIG_BIGNUM
    CONFIG_VERSION="${quickjs_version}"
)

add_library(quickjs_shared SHARED $<TARGET_OBJECTS:quickjs>)

set_target_properties(
  quickjs_shared
  PROPERTIES
  OUTPUT_NAME quickjs
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_library(quickjs_static STATIC $<TARGET_OBJECTS:quickjs>)

set_target_properties(
  quickjs_static
  PROPERTIES
  OUTPUT_NAME quickjs
  PREFIX lib
)

install(TARGETS quickjs_shared quickjs_static)
