if(UNIX AND NOT APPLE)
  include(GNUInstallDirs)
elseif(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR ""
      CACHE PATH "Specify the output directory for libraries (default is lib)")
endif()
