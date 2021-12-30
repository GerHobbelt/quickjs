macro(find_libwebsockets)

  include(FindPkgConfig)

  unset(LIBWEBSOCKETS_INCLUDE_DIRS CACHE)
  unset(LIBWEBSOCKETS_LIBRARY_DIR CACHE)
  unset(LIBWEBSOCKETS_LIBRARIES CACHE)
  unset(LIBWEBSOCKETS_FOUND CACHE)

  pkg_check_modules(LIBWEBSOCKETS libwebsockets)
  pkg_search_module(OPENSSL openssl)

  if(NOT OPENSSL_SSL_LIBRARY AND NOT OPENSSL_CRYPTO_LIBRARY)
    if(pkgcfg_lib_OPENSSL_ssl)
      set(OPENSSL_SSL_LIBRARY "${pkgcfg_lib_OPENSSL_ssl}" CACHE PATH "OpenSSL ssl library")
    endif(pkgcfg_lib_OPENSSL_ssl)
    if(pkgcfg_lib_OPENSSL_crypto)
      set(OPENSSL_CRYPTO_LIBRARY "${pkgcfg_lib_OPENSSL_crypto}" CACHE PATH "OpenSSL crypto library")
    endif(pkgcfg_lib_OPENSSL_crypto)
  endif(NOT OPENSSL_SSL_LIBRARY AND NOT OPENSSL_CRYPTO_LIBRARY)

  if(OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
    set(OPENSSL_LIBRARIES "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}")
  endif(OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)

  if(pkgcfg_lib_LIBWEBSOCKETS_websockets AND EXISTS "${pkgcfg_lib_LIBWEBSOCKETS_websockets}")
    set(LIBWEBSOCKETS_LIBRARIES "${pkgcfg_lib_LIBWEBSOCKETS_websockets}")
  endif(pkgcfg_lib_LIBWEBSOCKETS_websockets AND EXISTS "${pkgcfg_lib_LIBWEBSOCKETS_websockets}")
  set(LIBWEBSOCKETS_LIBRARIES "${LIBWEBSOCKETS_LIBRARIES}" CACHE FILEPATH "libwebsockets library")

  if(LIBWEBSOCKETS_LIBRARIES AND LIBWEBSOCKETS_LIBRARIES MATCHES ".*/.*")
    string(REGEX REPLACE "/[^/]*$" "" LIBWEBSOCKETS_LIBRARY_DIR "${LIBWEBSOCKETS_LIBRARIES}")
    string(REGEX REPLACE "/lib/.*$" "/include" LIBWEBSOCKETS_INCLUDE_DIRS "${LIBWEBSOCKETS_LIBRARIES}")
  endif(LIBWEBSOCKETS_LIBRARIES AND LIBWEBSOCKETS_LIBRARIES MATCHES ".*/.*")

  if(CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH "${LIBWEBSOCKETS_LIBRARY_DIR}:${CMAKE_INSTALL_RPATH}" CACHE PATH "Install runtime path")
  else(CMAKE_INSTALL_RPATH)
    set(CMAKE_INSTALL_RPATH "${LIBWEBSOCKETS_LIBRARY_DIR}" CACHE PATH "Install runtime path")
  endif(CMAKE_INSTALL_RPATH)

  set(LIBWEBSOCKETS_LIBRARY_DIR "${LIBWEBSOCKETS_LIBRARY_DIR}" CACHE PATH "libwebsockets library directory")
  set(LIBWEBSOCKETS_INCLUDE_DIRS "${LIBWEBSOCKETS_INCLUDE_DIRS};${OPENSSL_INCLUDE_DIRS}"
      CACHE PATH "libwebsockets include directory")

endmacro(find_libwebsockets)
