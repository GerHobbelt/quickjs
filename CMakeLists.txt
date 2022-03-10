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
	target_compile_definitions(quickjs PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
	target_sources(quickjs PRIVATE quickjs-debugger-transport-win.c)
else()
	target_sources(quickjs PRIVATE quickjs-debugger-transport-unix.c)
endif()

target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(qjs qjs.c)
target_link_libraries(qjs PRIVATE quickjs)

add_executable(qjsc qjsc.c)
target_link_libraries(qjsc PRIVATE quickjs)
