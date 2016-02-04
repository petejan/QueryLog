/*
 * TimeStamps and things
 */

#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#ifdef __WIN32__
#include <windows.h>
#endif

#ifdef __linux__
typedef struct _SYSTEMTIME
{
    int wYear;
    int wMonth;
    int wDayOfWeek;
    int wDay;
    int wHour;
    int wMinute;
    int wSecond;
    int wMilliseconds;
}
SYSTEMTIME;
#endif

typedef unsigned long long int timeT;

/* return the time now in usec accuracy */

extern timeT getTime( void );
extern void timeToSysTime( timeT a, SYSTEMTIME * st );
extern void timeToStr( timeT a, char * string );
extern void sysTimeToTime( SYSTEMTIME st, timeT *ret );
extern timeT strToAbsTime( char * string, timeT start );
#endif
