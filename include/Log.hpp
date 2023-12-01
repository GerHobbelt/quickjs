#ifndef LOG_H
#define LOG_H 1

#include <stdio.h>


enum priority
{
    // print developer information (trace stack)
    priority_developer=0,
    // debug information (not product usage)
    priority_debug,
    // detailed program information
    priority_info,
    // an under the control problem
    priority_notice,
    // considerable problem (eg: network problem,bad configuration), may be terminated
    priority_warn,
    // handleable unexpected error in codes, will be terminated gradually
    priority_error,
    // unhandleable unexpected error, will be terminated
    priority_crit,
    // error occured from low-level (eg:OS, Hardware), terminate imediately
    priority_emerg,
    // print the error any way
    priority_any=99,
};

#define Console(PERIORITY,TAG,...) do{printf("[%s] <%s> %s:%d >", "PERIORITY" , TAG , __FILE__ , __LINE__ );printf(__VA_ARGS__);}while(0);


#endif
