/**
 * @file Timer.hpp
 * @author your name (you@domain.com)
 * @brief A interface
 * it is OS dependent, means must write low level access code for each OS
 * @version 0.1
 * @date 2022-06-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef TIMER_HPP
#define TIMER_HPP 1

#include "CrossPlatform.h"
#if IS_WINDOWS_OS
    #ifndef WIN32_LEAN_AND_MEAN
	    #define WIN32_LEAN_AND_MEAN
	#endif
    #include <windows.h>
#else
    #include <time.h>
    #include <sys/times.h>
#endif


    class Timer
    {
    private:
        #if IS_WINDOWS_OS
        LARGE_INTEGER start_counter,end_counter ;
        LARGE_INTEGER frequency;
        #else
        timespec start_timespec,end_timespec;
        #endif
        /// in second
    public:
        double dtime;
        inline operator double() const { return this->dtime; }
        /// all returns 0 in failed
        int8_t init();
        /// all returns 0 in failed
        int8_t start_point();
        /// all returns 0 in failed
        int8_t end_point();
    };
    extern Timer DeltaTime;

#endif
