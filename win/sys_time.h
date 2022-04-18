#ifndef SYS_TIME_H
#define SYS_TIME_H

#if defined(_MSC_VER)
#include "WinSock2.h"
#endif

#ifndef _WINSOCKAPI_ // struct timeval already defined in winsock.h

typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

#endif

struct timezone {
	int tz_minuteswest;     /* minutes west of Greenwich */
	int tz_dsttime;         /* type of DST correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz);

#endif
