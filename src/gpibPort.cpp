/*
 * gpib Port interface
 *
 * Peter Jansen
 * Australian Antarctic Divison 2006
 */

#ifdef ENABLE_GPIB
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __linux__
#include <unistd.h>
#include <gpib/ib.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#include "decl-32.h"
#endif

#include "common.h"
#include "port.hpp"

#define BDINDEX 0
#define NO_SECONDARY_ADDR 0

const static char ErrorMnemonic[21] [5] =
{
    "EDVR", "ECIC", "ENOL", "EADR", "EARG", "ESAC", "EABO", "ENEB", "EDMA", "",
    "EOIP", "ECAP", "EFSO", "", "EBUS", "ESTB", "ESRQ", "", "", "", "ETAB"
};

const static unsigned long int TimeOut[13] =
{
    1, 3, 10, 30, 100, 300, 1000, 3000, 10000, 30000, 100000, 300000, 1000000
};

const static int TimeTS[13] =
{
    T1ms, T3ms, T10ms, T30ms, T100ms, T300ms, T1s, T3s, T10s, T30s, T100s, T300s, T1000s
};


/* After each GPIB call, the application checks whether the call
 *  succeeded. If an NI-488.2 call fails, the GPIB driver sets the
 *  corresponding bit in the global status variable. If the call
 *  failed, this procedure prints an error message, takes the device offline
 *  and exits.
 */

void gpibPort::GPIBCleanup(char * ErrorMsg )
{
    fprintf( stderr, " GPIB %s Error: <%s> : ibsta = 0x%x iberr = %d (%s) ", deviceString, ErrorMsg, ibsta, iberr, ErrorMnemonic[iberr] );
}

gpibPort::gpibPort(char *device): port(device)
{
    type = GPIB;
    timeout = T1s;
}

void *gpibPort::portOpen(void)
{
    int ret;
    int gpibAddress;
    int sta;

    gpibAddress = atoi(deviceString);

    handle = ibdev( BDINDEX, gpibAddress, NO_SECONDARY_ADDR, T1s, 1, 0 );
    if ( ibsta & ERR )
    {
        GPIBCleanup( (char*)"Unable to open device" );
        exit( -1 );
    }

    sta = ibconfig( BDINDEX, IbcAUTOPOLL, 0 ); // enable auto serial polling
    if ( sta & ERR )
    {
        GPIBCleanup( (char*)"Problem setting Autopoll" );
        exit( -1 );
    }

    if (eosMode)
    {
        ibeos( handle, eosMode);
    }

    if (eotMode)
    {
        ibeot( handle, eotMode);
    }

    ibtmo( handle, timeout);

    portService();

    Dev = &handle;

    return Dev;
}

void *gpibPort::reOpen(void *newDev)
{
    Dev = newDev;

    handle = *(int *)Dev;

    return Dev;
}

int gpibPort::portClose(void)
{
    int ret;

    ret = ibonl( handle, 0 );

    return ret;
}

int gpibPort::portRead(char *buffer, int len)
{
    ibrd( handle, buffer , len );
    // printf("GPIB: read 0x%04x\n", ibsta);
    if ( ibsta & ERR )
    {
        buffer[0] = '\0';

        return -1;
    }
    else if ( ibsta & TIMO )
    {
        buffer[0] = '\0';

        return -1;
    }

    /* Assume that the returned string contains ASCII data. NULL terminate
     * the string using the value in ibcntl which is the number of bytes
     * read in.
     */

    buffer[ibcntl] = '\0';

    return ibcntl;
}

int gpibPort::portWrite(char *buffer, int len)
{
    int ret;

    // printf("GPIB: write\n");
    ret = ibwrt( handle, buffer, len );

    if ( ibsta & ERR )
    {
        GPIBCleanup( (char*)"Unable to write to device" );
        return -1;
    }

    return ret;
}

int gpibPort::portService(void)
{
    int sta;
    char spr;
    int ret = 0;

    sta = ibwait( BDINDEX, 0);
    if (sta & SRQI)
    {
        // printf( "Board Service Request\n" );
        sta = ibrsp( handle, & spr );

        if ( ( spr & 0x40 ) != 0 )
        {
            // printf( "Instrument Service Request %s\n", deviceString);
            ret = 1;
        }
    }

    return ret;
}

int gpibPort::portMode(setType setting, char *set)
{
    int ret = -1;
    char spr;
    int i = 0;
    unsigned long newTimeout;

    switch (setting)
    {
        case EOSMODE:
            eosMode = BIN | REOS | set[0];
            break;
        case EOTMODE:
            eotMode = 1;
            break;
        case TIMEOUT:
            // find the timeout to set
            newTimeout = strtoul(set, NULL, 10);

            while ( ( i < 13 ) && ( newTimeout > TimeOut[i] ) )
            {
                i++;
            }
            if (i < 13)
            {
                timeout = TimeTS[i];
            }

            break;
        case CLEAR:
            // printf("GPIB: send CLEAR\n");
            /* Clear the internal or device functions of the device.  If the error
             *  bit ERR is set in ibsta, call GPIBCleanup with an error message. */

            ret = ibclr( handle );
            if ( ibsta & ERR )
            {
                GPIBCleanup( (char*)"Unable to clear device" );
                exit( -1 );
            }
            break;
    }

    return ret;
}

int gpibPort::dataAvaliable(void)
{
    return 1;
}


void gpibPort::print(int to)
{
    lprintf(to, "GPIB(%s)", deviceString);
}
#endif
