/**
 * @file CrossPlatform.h
 * @author ZWArrior
 * @brief A compile-time c header to help to write cross platform codes
 * @version 1.1
 * @date 2022-06-11
 *
 */
#ifndef CROSSPLAT_HPP
#define CROSSPLAT_HPP 1

#include <stdint.h>
//#include <endian.h>

#if ( defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) )
    #define IS_WINDOWS_OS 1
    #define IS_ANDROID_OS 0
    #define IS_MAC_OS 0
    #define IS_LINUX_OS 0
#elif  defined(__ANDROID__)
    #define IS_WINDOWS_OS 1
    #define IS_ANDROID_OS 1
    #define IS_MAC_OS 0
    #define IS_LINUX_OS 0
#elif defined(__APPLE__)
    #define IS_WINDOWS_OS 0
    #define IS_ANDROID_OS 0
    #define IS_MAC_OS 1
    #define IS_LINUX_OS 0
#else //defined(_X11) || defined(X_PROTOCOL)
    #define IS_WINDOWS_OS 0
    #define IS_ANDROID_OS 0
    #define IS_MAC_OS 0
    #define IS_LINUX_OS 1
#endif

#endif
