/*
 * NMEA check sum checker
 *
 * Peter Jansen
 * Australian Antarctic Division
 */

#include <string.h>
#include <stdio.h>

int NmeaCheck( char * buffer )
{
    size_t startCh;
    size_t starCh;
    int length;
    unsigned int i;
    unsigned int sum;
    int ret;

    ret = -1; /* No Check sum */

    length = strlen( buffer );

    startCh = strcspn( buffer, "$" );
    starCh = strcspn( buffer, "*" );

    if ( (( length - startCh ) > 4 ) && (starCh != 0))
    {
        ret = 0; /* Check sum found */
        sum = 0;
        for ( i = startCh + 1; i < starCh; i++ )
        {
            sum ^= buffer[i];
        }
        sum = sum & 0xff;
        sscanf( & buffer[starCh + 1], "%x", & i );
        if ( i != sum )
        {
            ret = 1; /* Check sum ok */
        }
    }

    return ret;
}


