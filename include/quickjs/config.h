#ifndef JS_CONFIG_H
#define JS_CONFIG_H 1



#if defined(_WIN32)
#include <windows.h>
#endif

#define JS_VERSION "0.15"

#if defined(JS_STATIC_LIBRARY)
#define JS_MODULE
// if the C file that include this was for library code that define this function
// to be exported
#elif defined(JS_SHARED_LIBRARY)
    #if defined(_WIN32)
        #define JS_MODULE  __declspec(dllexport)
    #else
        #define JS_MODULE  extern
    #endif
// if the C file that include this was the interepter that need to call this function
// so need to be imported from library
#else
    #if defined(_WIN32)
        #define JS_MODULE  __declspec(dllimport)
    #else
        #define JS_MODULE  extern
    #endif
#endif



#endif /*JS_CONFIG_H*/
