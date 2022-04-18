#ifndef SYS_TIME_H
#define SYS_TIME_H


#ifndef _WINSOCKAPI_ // struct timeval may already be defined in winsock.h

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
