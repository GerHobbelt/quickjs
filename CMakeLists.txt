project(quickjs)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(quickjs PUBLIC OS)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(quickjs PRIVATE DUMP_LEAKS=1)
	#target_compile_definitions(quickjs PUBLIC CONFIG_DEBUGGER)
endif()

add_library(quickjs-libc STATIC
	  quickjs-libc.c
		quickjs-libc.h
  )
target_link_libraries(quickjs-libc PUBLIC quickjs)

add_executable(qjsc qjsc.c)
target_link_libraries(qjsc PRIVATE quickjs-libc)

add_custom_command(
		COMMAND qjsc -c -o repl.c -m ${CMAKE_CURRENT_SOURCE_DIR}/repl.js
		OUTPUT repl.c
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/repl.js
	)

add_executable(qjs qjs.c repl.c)
target_link_libraries(qjs PRIVATE quickjs-libc)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # https://bugs.llvm.org/show_bug.cgi?id=25305
	target_compile_definitions(quickjs PUBLIC CONFIG_BIGNUM)
	target_sources(qjs PRIVATE qjscalc.c)
	add_custom_command(
			COMMAND qjsc -fbignum -c -o qjscalc.c -m ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js
			OUTPUT qjscalc.c
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qjscalc.js
		)
endif()
