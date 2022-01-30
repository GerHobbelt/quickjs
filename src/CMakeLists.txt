cmake_minimum_required(VERSION 3.12.0)

set(QUICKJS_SOURCES cutils.h
        cutils.c
        libregexp.c
        libunicode.c
        quickjs.c
        quickjs-libc.c
        libbf.c
        libregexp.h
        libregexp-opcode.h
        libunicode.h
        libunicode-table.h
        list.h
        quickjs.h
        quickjs-atom.h
        quickjs-libc.h
        quickjs-opcode.h
        quickjs-jsx.h)

add_library(quickjs STATIC ${QUICKJS_SOURCES})
target_compile_definitions(quickjs PRIVATE
 JS_STRICT_NAN_BOXING
 CONFIG_BIGNUM
 CONFIG_JSX
 _CRT_SECURE_NO_WARNINGS
)

target_include_directories(quickjs PUBLIC .)

set_target_properties(quickjs PROPERTIES
 C_STANDARD 11
)

add_executable(qjsc qjsc.c)
target_link_libraries(qjsc quickjs pthread)