project(quickjs)

add_library(quickjs STATIC
		config.h
	  cutils.c
		cutils.h
	  libbf.c
		libbf.h
		libregexp-opcode.h
		libregexp.c
		libregexp.h
		libunicode-table.h
	  libunicode.c
		libunicode.h
		list.h
		quickjs-atom.h
		quickjs-debugger.c
		quickjs-debugger.h
	  quickjs-libc.c
		quickjs-libc.h
		quickjs-opcode.h
	  quickjs.c
    quickjs.h
  )

if(WIN32)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(quickjs PRIVATE -Wno-microsoft-enum-forward-reference -Wno-implicit-const-int-float-conversion)
	endif()
	target_compile_definitions(quickjs PRIVATE alloca=_alloca _WINSOCK_DEPRECATED_NO_WARNINGS)
	target_sources(quickjs PRIVATE quickjs-debugger-transport-win.c)
else()
	target_sources(quickjs PRIVATE quickjs-debugger-transport-unix.c)
endif()

file(STRINGS "VERSION" QUICKJS_VERSION)
target_compile_definitions(quickjs PUBLIC CONFIG_VERSION="${QUICKJS_VERSION}")
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(quickjs PUBLIC OS)

add_executable(qjs qjs.c)
target_link_libraries(qjs PRIVATE quickjs)

add_executable(qjsc qjsc.c)
target_link_libraries(qjsc PRIVATE quickjs)
