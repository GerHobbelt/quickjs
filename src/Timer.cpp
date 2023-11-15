#include "Timer.hpp"
Timer DeltaTime;
int8_t Timer::init()
{
    dtime = 0.0;
#if IS_LINUX_OS
    start_timespec={0,0},end_timespec={0,0};
#else
    start_counter = {0,0};end_counter={0,0};
    return QueryPerformanceFrequency(&frequency);
#endif
    return 0;
}

int8_t Timer::start_point()
{
#if IS_LINUX_OS
    // Setup time step
    clock_gettime(CLOCK_MONOTONIC, &start_timespec);
    return 1;
#else
    return QueryPerformanceCounter(&start_counter);
#endif
}

int8_t Timer::end_point()
{
#if IS_LINUX_OS
    clock_gettime(CLOCK_MONOTONIC, &end_timespec);
    dtime = (double)(
        (double)(end_timespec.tv_sec - start_timespec.tv_sec) +
        (double)(end_timespec.tv_nsec - start_timespec.tv_nsec) * 1e-9
    );
    //may happen
    if(dtime < 0.0)
        dtime+=( 1000000000.0 * 60);
    return 1;
#else
    int8_t buffer = QueryPerformanceCounter(&end_counter);
    dtime = ((double)(end_counter.QuadPart - start_counter.QuadPart)) / ((double)frequency.QuadPart);
    return buffer;
#endif
}
