/*
 * TimeStamps and things
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if __linux__
#include <time.h>
#include <sys/time.h>
#endif

#include "timeStamp.h"

/* return the time now in usec accuracy */

timeT getTime( void )
{
#ifdef __WIN32__
    FILETIME ft;
    ULARGE_INTEGER ui;
    SYSTEMTIME stTimeNow;
    unsigned long long int timeNow;

    GetLocalTime(& stTimeNow);
    SystemTimeToFileTime( & stTimeNow, & ft );

    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;

    GetLocalTime( & stTimeNow );
    timeNow = ui.QuadPart;

    return timeNow / 10LL;
#endif
#if __linux__
    struct timeval tv;

    gettimeofday( & tv, NULL );

    return ( tv.tv_sec * 1000000LL ) + tv.tv_usec;
#endif
}

void timeToSysTime( timeT a, SYSTEMTIME * st )
{
#ifdef __WIN32__
    FILETIME ft;
    ULARGE_INTEGER ui;

    ui.QuadPart = a * 10LL;

    ft.dwLowDateTime = ui.LowPart;
    ft.dwHighDateTime = ui.HighPart;

    FileTimeToSystemTime( & ft, st );
#endif
#if __linux__
    struct tm * t;
    time_t tsecs;

    tsecs = a / 1000000LL;
    t = localtime( & tsecs );

    st->wYear = t->tm_year + 1900;
    st->wMonth = t->tm_mon + 1;
    st->wDay = t->tm_mday;
    st->wHour = t->tm_hour;
    st->wMinute = t->tm_min;
    st->wSecond = t->tm_sec;
    st->wMilliseconds = ( a / 1000LL ) % 1000LL;
#endif
}

void timeToStr( timeT a, char * string )
{
    SYSTEMTIME st;

    timeToSysTime( a, & st );

    sprintf( string, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute,
            st.wSecond, st.wMilliseconds );
}

void sysTimeToTime( SYSTEMTIME st, timeT *ret )
{
#if __linux__
    time_t tt;
    struct tm t;

    t.tm_year = st.wYear - 1900;
    t.tm_mon = st.wMonth - 1;
    t.tm_mday = st.wDay;
    t.tm_hour = st.wHour;
    t.tm_min = st.wMinute;
    t.tm_sec = st.wSecond;

    tt = mktime( & t );

    *ret = ( tt * 1000000LL ) + st.wMilliseconds * 1000LL;
#endif
#ifdef __WIN32__
    FILETIME ft;
    ULARGE_INTEGER ui;

    SystemTimeToFileTime( & st, & ft );

    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;

    *ret = ui.QuadPart / 10LL;
#endif
}

timeT strToAbsTime( char * string, timeT start )
{
    int hrs = -1;
    int mins = -1;
    int secs = 0;
    char * param;

    if ( strlen( string ) == 0 ) return 0;

    param = strtok( string, " :" );
    if ( param != NULL )
    {
        secs = strtoul( param, NULL, 10 );
    }
    param = strtok( NULL, " :" );
    if ( param != NULL )
    {
        mins = secs;
        secs = strtoul( param, NULL, 10 );
    }
    param = strtok( NULL, " :" );
    if ( param != NULL )
    {
        hrs = mins;
        mins = secs;
        secs = strtoul( param, NULL, 10 );
    }
    param = strtok( NULL, " :" );
    if ( param != NULL )
    {
        hrs = hrs * 24 + mins;
        mins = secs;
        secs = strtoul( param, NULL, 10 );
    }

    /* have the time now (add it) to the old time */
    if ( ( string[0] == '+' ) || ( start == 0 ) )
    {
        if ( hrs < 0 ) hrs = 0;
        if ( mins < 0 ) mins = 0;
        start = start + ( timeT )( secs + ( mins * 60L ) + ( hrs * 3600L ) ) * 1000000LL;
    }
    else
    {
        SYSTEMTIME stStart;

        timeToSysTime( start, & stStart );

        if ( hrs >= 0 )
        {
            if ( stStart.wHour > hrs ) stStart.wDay++;
            stStart.wHour = hrs;
        }

        if ( mins >= 0 )
        {
            if ( stStart.wMinute > mins ) stStart.wHour++;
            stStart.wMinute = mins;
        }

        if ( stStart.wSecond > secs ) stStart.wMinute++;
        stStart.wSecond = secs;
        stStart.wMilliseconds = 0;

        sysTimeToTime( stStart, &start );
    }

    return start;
}
