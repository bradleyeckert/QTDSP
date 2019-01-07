// Get time in usec

#if defined(WIN32) || defined(__WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN32_)

#include <windows.h>
double now()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return 1e6*(double)t.QuadPart/(double)f.QuadPart;
}

#else

#include <sys/time.h>
#include <sys/resource.h>

double now()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return 1e6*t.tv_sec + t.tv_usec;
}

#endif

