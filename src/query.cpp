/*
 * query.cpp - a class for a single query record
 *
 * Peter Jansen
 * Australian Antarctic Division
 */

#include <string.h>

#include "port.hpp"
#include "query.hpp"
#include "timeStamp.h"
#include "common.h"

query::query(void)
{
    receiveWait = 0;
    send = 0;
    sendClear = 0;
    nmea = 0;
    timeout = 10000;
    period = 0;
    first = 0;
    last = 0;
    nextRuntime = 0;
    number = -1;
    memset(message, 0, sizeof(message));
    messageLength = 0;
    memset(header, 0 , sizeof(header));
    sent = 0;
    received = 0;
    sumErrors = 0;
    checked = 0;
    timeouts = 0;
    memset(args, 0, sizeof(args));
    srq = 0;
    srqReq = 0;
    device = NULL;
}

void query::setPeriod(char *p)
{
    period = strToAbsTime(p, 0);
}

void query::setFirstTime(char *p, timeT time)
{
    first = strToAbsTime(p, time);
}

void query::setLastTime(char *p, timeT time)
{
    last = strToAbsTime(p, time);
}

int query::finished(timeT time)
{
    int cont = 0;
    char sTime[80];

    timeToStr( time, sTime );

    if ( ( last != 0l ) )
    {
    	if  ( (last > time) )
		{
			cont = 1;
		}
    }
	else if ( ( number == -1 ) || ( number > 0 ) )
    {
        cont = 1;
    }

    //printf("%d time %s time to go %ld\n", cont, sTime, last-time);

    return cont;
}

void query::print(int to)
{
    char sTime[100];

    if ( period != 0 )
    {
        lprintf( to, " : Period %-6Lu sec", ( period / 1000000LL ) );
    }

    if ( first != 0 )
    {
        timeToStr( first, sTime );
        lprintf( to, " : First %s", sTime );
    }
    if ( last != 0 )
    {
        timeToStr( last, sTime );
        lprintf( to, " : Last %s", sTime );
    }

    if ( number > 0 )
    {
        lprintf( to, " : Send %-3d", number );
    }

    if ( sendClear != 0 )
    {
        lprintf( to, " : (CLEAR)" );

    }
    if ( send != 0 )
    {
        lprintf( to, " : TX " );

        cleanPrint( to, message, messageLength);
    }

    lprintf( to, "\n" );

}

void query::timeStamp(int to, timeT time)
{
    char sTime[100];

    timeToStr( time, sTime );
    lprintf( to, "%s, ", sTime );

    if ( header[0] != '\0' )
    {
        lprintf( to, "%s ", header );
    }

    device->print(to);
}

int query::thisRecord(timeT timeNow)
{
    int thisRecord = 0;
    int ret = 0;

    // if first time is enabled or passed
    if ( ( nextRuntime == 0 ) && ( first < timeNow ) )
    {
        // printf("First Time %lld %lld %d %lld\n", timeNow - first, first, period);
        if ( first == 0 )
        {
            nextRuntime = timeNow;
        }
        else
        {
            nextRuntime = first;
        }
    }
    else if ( ( nextRuntime > 0 ) && ( nextRuntime < timeNow ) )
    {
        // if we're not waiting for a service request, then do this one now
        if ( srq == 0 )
        {
            thisRecord = 1;
        }

        if ( last > 0 )
        {
            if ( timeNow > last )
            {
                thisRecord = 0;
            }
        }

    }

    // check is instrument is waiting for a service
    if ( srqReq != 0 )
    {
        thisRecord = 1; // go and send/read this record
    }

    /* Deal with the number to send */
    if (( number >= 0 ) && (thisRecord != 0))
    {
        if ( number == 0 )
        {
            thisRecord = 0; /* no more for this record */
        }
        else
        {
            number--;
        }
    }

    if (thisRecord != 0)
    {
        /* deal with the period, by setting the first time to now + period */
        if ( period > 0 )
        {
            if ( nextRuntime == 0 )
            {
                nextRuntime = timeNow + period;
            }
            else
            {
                nextRuntime += period;
            }
        }

        if (send != 0)
        {
            ret |= 0x01;
        }

        if (sendClear != 0)
        {
            ret |= 0x02;
        }

        if ( receiveWait != 0 )
        {
            if (send == 0)
            {
                if (device->dataAvaliable() != 0)
                {
                    ret |= 0x04;
                }
            }
            else
            {
                ret |= 0x04;
            }
        }
    }

    return ret;
}
