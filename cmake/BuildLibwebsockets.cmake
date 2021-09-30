macro(build_libwebsockets)
  message("-- Building LIBWEBSOCKETS from source")
  set(LWS_WITHOUT_TESTAPPS TRUE)
  set(LWS_WITHOUT_TEST_SERVER TRUE)
  set(LWS_WITHOUT_TEST_PING TRUE)
  set(LWS_WITHOUT_TEST_CLIENT TRUE)
  set(LWS_LINK_TESTAPPS_DYNAMIC OFF CACHE BOOL "link test apps dynamic")
  set(LWS_WITH_STATIC ON CACHE BOOL "build libwebsockets static library")
  set(LWS_HAVE_LIBCAP FALSE CACHE BOOL "have libcap")

  # include: libwebsockets find_package(libwebsockets)
  set(LIBWEBSOCKETS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libwebsockets/include ${CMAKE_CURRENT_BINARY_DIR}/libwebsockets ${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/include)
  set(LIBWEBSOCKETS_FOUND ON CACHE BOOL "found libwebsockets")
  set(LIBWEBSOCKETS_LIBRARIES "${MBEDTLS_LIBRARIES};brotlienc;brotlidec;cap" CACHE STRING "libwebsockets libraries")
  if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib/libwebsockets.a")
    set(LIBWEBSOCKETS_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib/libwebsockets.a" ${LIBWEBSOCKETS_LIBRARIES})
  else(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib/libwebsockets.a")
    set(LIBWEBSOCKETS_LIBRARIES "websockets" ${LIBWEBSOCKETS_LIBRARIES})
  endif(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib/libwebsockets.a")

  set(LIBWEBSOCKETS_INCLUDE_DIRS "${LIBWEBSOCKETS_INCLUDE_DIRS}" CACHE PATH "libwebsockets include directory")
  set(LIBWEBSOCKETS_LIBRARY_DIRS ${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib CACHE PATH "libwebsockets library directory")
  # add_subdirectory(libwebsockets ${CMAKE_CURRENT_BINARY_DIR}/libwebsockets)
  include(ExternalProject)
  ExternalProject_Add(
    libwebsockets
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libwebsockets
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libwebsockets
    CMAKE_CACHE_ARGS
      "-DCMAKE_SYSTEM_INCLUDE_PATH:STRING=${CMAKE_SYSTEM_INCLUDE_PATH}"
      "-DCMAKE_SYSTEM_LIBRARY_PATH:STRING=${CMAKE_SYSTEM_LIBRARY_PATH}"
      "-DCMAKE_SYSTEM_PROGRAM_PATH:STRING=${CMAKE_SYSTEM_PROGRAM_PATH}"
      "-DCMAKE_SYSTEM_IGNORE_PATH:STRING=${CMAKE_SYSTEM_IGNORE_PATH}"
      "-DCMAKE_INCLUDE_PATH:STRING=${CMAKE_INCLUDE_PATH}"
      "-DCMAKE_LIBRARY_PATH:STRING=${CMAKE_LIBRARY_PATH}"
      "-DCMAKE_PROGRAM_PATH:STRING=${CMAKE_PROGRAM_PATH}"
      "-DCMAKE_IGNORE_PATH:STRING=${CMAKE_IGNORE_PATH}"
      "-DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH}"
      "-DCMAKE_MODULE_PATH:STRING=${CMAKE_MODULE_PATH}"
      "-DPKG_CONFIG_EXECUTABLE:FILEPATH=${PKG_CONFIG_EXECUTABLE}"
      "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
      "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}"
      "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
      "-DCMAKE_INSTALL_RPATH:STRING=${MBEDTLS_LIBRARY_DIR}"
      -DCOMPILER_IS_CLANG:BOOL=OFF
      "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
      #"-DLWS_MBEDTLS_LIBRARIES:STRING=${MBEDTLS_LIBRARIES}"
      #"-DLWS_MBEDTLS_INCLUDE_DIRS:STRING=${MBEDTLS_INCLUDE_DIRS}"
      -DLWS_WITH_DIR:BOOL=OFF
      -DLWS_WITH_LEJP:BOOL=OFF
      -DLWS_WITH_LEJP_CONF:BOOL=OFF
    CMAKE_CACHE_DEFAULT_ARGS
      -DDISABLE_WERROR:BOOL=ON
      -DLWS_HAVE_LIBCAP:BOOL=FALSE
      -DLWS_ROLE_RAW_PROXY:BOOL=ON
      -DLWS_STATIC_PIC:BOOL=ON
      -DLWS_UNIX_SOCK:BOOL=ON
      -DLWS_WITH_ACCESS_LOG:BOOL=ON
      -DLWS_WITH_CGI:BOOL=OFF
      -DLWS_WITH_DISKCACHE:BOOL=ON
      -DLWS_WITH_EVLIB_PLUGINS:BOOL=OFF
      -DLWS_WITH_EXTERNAL_POLL:BOOL=ON
      -DLWS_WITH_FILE_OPS:BOOL=ON
      -DLWS_WITH_FSMOUNT:BOOL=ON
      -DLWS_WITH_HTTP2:BOOL=ON
      -DLWS_WITH_HTTP_BROTLI:BOOL=ON
      -DLWS_WITH_HTTP_PROXY:BOOL=ON
      -DLWS_WITH_HTTP_STREAM_COMPRESSION:BOOL=ON
      -DLWS_WITH_LIBUV:BOOL=OFF
      -DLWS_WITH_MBEDTLS:BOOL=ON
      -DLWS_WITH_MINIMAL_EXAMPLES:BOOL=OFF
      -DLWS_WITH_NO_LOGS:BOOL=OFF
      -DLWS_WITHOUT_EXTENSIONS:BOOL=OFF
      -DLWS_WITHOUT_TESTAPPS:BOOL=ON
      -DLWS_WITH_PLUGINS_API:BOOL=${LWS_WITH_PLUGINS_API}
      -DLWS_WITH_PLUGINS:BOOL=${LWS_WITH_PLUGINS}
      -DLWS_WITH_PLUGINS_BUILTIN:BOOL=${LWS_WITH_PLUGINS_BUILTIN}
      -DLWS_WITH_RANGES:BOOL=ON
      -DLWS_WITH_SERVER:BOOL=ON
      -DLWS_WITH_SHARED:BOOL=OFF
      -DLWS_WITH_SOCKS5:BOOL=ON
      -DLWS_WITH_STATIC:BOOL=ON
      -DLWS_WITH_SYS_ASYNC_DNS:BOOL=ON
      -DLWS_WITH_THREADPOOL:BOOL=ON
      -DLWS_WITH_UNIX_SOCK:BOOL=ON
      -DLWS_WITH_ZIP_FOPS:BOOL=ON
      -DLWS_WITH_ZLIB:BOOL=ON

      #"-DLWS_MBEDTLS_LIBRARIES:STRING=${MBEDTLS_LIBRARIES}"
      #"-DLWS_MBEDTLS_INCLUDE_DIRS:STRING=${MBEDTLS_INCLUDE_DIRS}"
  )
  ExternalProject_Get_Property(libwebsockets CMAKE_CACHE_DEFAULT_ARGS)
  #message("CMAKE_CACHE_DEFAULT_ARGS of libwebsockets = ${CMAKE_CACHE_DEFAULT_ARGS}")
  link_directories("${CMAKE_CURRENT_BINARY_DIR}/libwebsockets/lib")

endmacro(build_libwebsockets)
