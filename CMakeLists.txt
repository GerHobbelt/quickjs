file(STRINGS VERSION QUICK_JS_VERSION)
add_library(quickjs STATIC cutils.c libbf.c libregexp.c libunicode.c quickjs.c
    quickjs-libc.c)
target_compile_definitions(quickjs PRIVATE
    CONFIG_VERSION="${QUICK_JS_VERSION}"
    CONFIG_BIGNUM=1
    _GNU_SOURCE)
target_link_libraries(quickjs Threads::Threads ${CMAKE_DL_LIBS})
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})