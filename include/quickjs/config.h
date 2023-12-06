#ifndef JS_CONFIG_H
#define JS_CONFIG_H 1



#if defined(_WIN32)
#include <windows.h>
#endif

#define JS_VERSION "0.15"


#if defined(_WIN32)
    #if defined(JS_STATIC_LIBRARY)
        #define JS_MODULE
    #elif defined(JS_SHARED_LIBRARY)
        // if the C file that include this was for library code that define this function
        // to be exported
        #define JS_MODULE  __declspec(dllexport)
    #else
        // if the C file that include this was the interepter that need to call this function
        // so need to be imported from library
        #define JS_MODULE  __declspec(dllimport)
    #endif
#else
    #if defined(JS_STATIC_LIBRARY)
        #define JS_MODULE
    #elif __GNUC__ >= 4
        # define JS_MODULE __attribute__((visibility("default")))
    #elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550) /* Sun Studio >= 8 */
        # define JS_MODULE __global
    #else
        #define JS_MODULE  extern
    #endif
#endif



#endif /*JS_CONFIG_H*/
