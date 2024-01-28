#ifndef __NODECPP_EXCEPTION_H
#define __NODECPP_EXCEPTION_H 1

#pragma GCC visibility push(default)

#include <bits/c++config.h>
#include "Log.hpp"
extern "C++" {

/**
 *  @brief Base class for this library exceptions.
 *
 *  You are free to derive
 *  your own %exception classes, or use a different hierarchy, or to
 *  throw non-class data (e.g., fundamental types).
 */
class Exception
{
protected:
    char const * const reason = "(Null)";
    const int32_t code = -1;
public:
    Exception() _GLIBCXX_NOTHROW = default;
    Exception(const char * _res, const int32_t _code=-1) _GLIBCXX_NOTHROW :reason(_res),code(_code) {}
    virtual ~Exception() _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW = default;
#if __cplusplus >= 201103L
    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;
    Exception(Exception&&) = default;
    Exception& operator=(Exception&&) = default;
#endif
    /** Returns a C-style character string describing the general cause
     *  of the current error.
     */
    const char*
    what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
        {return reason;}
};

}

#pragma GCC visibility pop

#endif