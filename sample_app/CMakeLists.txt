cmake_minimum_required(VERSION 3.9)
project(QuickJS-Experiment)

#========== Global Configurations =============#
#----------------------------------------------#

set( CMAKE_CXX_STANDARD     17 )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_CXX_EXTENSIONS   OFF)

# ------------ Download CPM CMake Script ----------------#

## Automatically donwload and use module CPM.cmake
file(DOWNLOAD https://raw.githubusercontent.com/TheLartians/CPM.cmake/v0.26.2/cmake/CPM.cmake
                 "${CMAKE_BINARY_DIR}/CPM.cmake")
include("${CMAKE_BINARY_DIR}/CPM.cmake")

#----------- Add dependencies --------------------------#

CPMAddPackage(
    NAME               quickjs 
    GITHUB_REPOSITORY  bellard/quickjs
    GIT_TAG            204682fb87ab9312f0cf81f959ecd181180457bc
    # DOWNLOAD_ONLY YES
    )


# Add this directory where is this file (CMakeLists.txt) to include path. 
include_directories( ${CMAKE_CURRENT_LIST_DIR} )

# =============== QuickJS settings ====================================#

include_directories( ${quickjs_SOURCE_DIR}/ )
message([TRACE] " quickjs source = ${quickjs_SOURCE_DIR} ")

file(GLOB quickjs_hpp ${quickjs_SOURCE_DIR}/*.h )

file(GLOB quickjs_src ${quickjs_SOURCE_DIR}/quickjs.c 
                      ${quickjs_SOURCE_DIR}/libregexp.c 
                      ${quickjs_SOURCE_DIR}/libunicode.c  
                      ${quickjs_SOURCE_DIR}/cutils.c 
                      ${quickjs_SOURCE_DIR}/quickjs-libc.c 
                      ${quickjs_SOURCE_DIR}/libbf.c 
                      )


               add_library( qjs-engine ${quickjs_src} ${quickjs_hpp} )
    target_compile_options( qjs-engine PRIVATE
                                -MMD -MF
                                -Wno-sign-compare 
                                -Wno-missing-field-initializers 
                                -Wundef -Wuninitialized 
                                -Wundef -Wuninitialized -Wwrite-strings -Wchar-subscripts
                          )
target_compile_definitions( qjs-engine PUBLIC 
                                       CONFIG_BIGNUM=y
                                       CONFIG_VERSION="2020-11-08"
                                       _GNU_SOURCE
                           )

if(UNIX)
    target_link_libraries( qjs-engine PRIVATE m pthread dl)
endif()

# =========== Target Settings =========================================#

            # QuickJS compiler. 
            add_executable( qjsc ${quickjs_SOURCE_DIR}/qjsc.c )
target_compile_definitions( qjsc  PUBLIC  CONFIG_BIGNUM=y  CONFIG_VERSION="2020-11-08"  _GNU_SOURCE )            
     target_link_libraries( qjsc  qjs-engine )

            # Sample application that embeds the quickJS Javascript engine. 
       add_executable( main main.cpp   )
target_link_libraries( main qjs-engine )
