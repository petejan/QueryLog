/*
 * serial Port interface
 *
 * Peter Jansen
 * Australian Antarctic Divison 2006
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __WIN32__
#include <windows.h>
#endif
#ifdef __linux__
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#include "port.hpp"
#include "common.h"
#include "timeStamp.h"

serialPort::serialPort(char *device) : port(device)
{
    strcpy( config, "19200,N,8,1");
    timeout = 1000000; // 1 sec
	pace = 0;
    rxdChars = 0;
    type = SERIAL;
}

void *serialPort::portOpen(void)
{
    char portName[256];
#if __linux__
    /* map the windows name to unix names */

    if ( strcmp( "COM1", deviceString) == 0 )
    {
        strcpy( portName, "/dev/ttyS0" );
    }
    else if ( strcmp( "COM2", deviceString) == 0 )
    {
        strcpy( portName, "/dev/ttyS1" );
    }
    else if ( strcmp( "COM3", deviceString) == 0 )
    {
        strcpy( portName, "/dev/ttyS2" );
    }
    else if ( strcmp( "COM4", deviceString) == 0 )
    {
        strcpy( portName, "/dev/ttyS3" );
    }
    else
    {
        strcpy( portName, deviceString);
    }
#endif
#ifdef __WIN32__
	// printf("WIN32 serialPort::portOpen \n");
    strcpy( portName, deviceString);
#endif

#if __linux__
    handle = open( portName, O_RDWR | O_NONBLOCK );

    if ( handle == -1 )
    {
        fprintf( stderr, "Could not open com port : %s\n", portName );
        exit( -1 );
    }

    tcgetattr( handle, & term_save );
    cfmakeraw( & tios );

    /* some things we want to set arbitrarily */
    tios.c_lflag &= ~ICANON;
    tios.c_lflag &= ~(ECHO | ECHOCTL | ECHONL);
    tios.c_cflag &= ~HUPCL;
    tios.c_cflag |= CREAD;
    tios.c_cc[VMIN] = 1;
    tios.c_cc[VTIME] = 0;

    /* Standard CR/LF handling: this is a dumb terminal.
     *  Do no translation:
     *   no NL -> CR/NL mapping on output, and
     *   no CR -> NL mapping on input.
     */
    tios.c_oflag &= ~ONLCR;
    tios.c_iflag &= ~ICRNL;

    /* no flow control */
    tios.c_cflag &= ~CRTSCTS;
    tios.c_cflag |= CLOCAL;
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    tcsetattr( handle, TCSANOW, & tios );
#endif
#ifdef __WIN32__
	char pn[256] = "\\\\.\\";
	
	strcat(pn, deviceString);
	
    handle = CreateFile( pn, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

    if ( handle == INVALID_HANDLE_VALUE )
    {
        fprintf( stderr, "Could not open com port : %s\n", deviceString );
        exit( -1 );
    }
#endif

#if __linux__
    char * speed;
    int baud;

    speed = strtok( config, "," );

    switch ( atoi( speed ) )
    {
        case 300:
            baud = B300;
            break;
        case 1200:
            baud = B1200;
            break;
        case 2400:
            baud = B2400;
            break;
        case 4800:
            baud = B4800;
            break;
        case 9600:
            baud = B9600;
            break;
        case 19200:
            baud = B19200;
            break;
        case 38400:
            baud = B38400;
            break;
        default:
            baud = 0;
            fprintf( stderr, "%s is an unsupported baud\n", config );
            exit( 1 );
    }
    cfsetispeed( & tios, baud );
    cfsetospeed( & tios, baud );

    tcsetattr( handle, TCSANOW, & tios );
#endif
#ifdef __WIN32__
    // Set the comm timeouts

    GetCommTimeouts( handle, &OldTimeOut );
    PortTimeOut.ReadTotalTimeoutConstant = 10; // 10 ms timeout between characters
    PortTimeOut.ReadTotalTimeoutMultiplier = 0;
    PortTimeOut.WriteTotalTimeoutMultiplier = 0;
    PortTimeOut.WriteTotalTimeoutConstant = 0;
    SetCommTimeouts( handle, &PortTimeOut );

    // SET BAUD RATE, PARITY, WORD SIZE, AND STOP BITS.
    // THERE ARE OTHER WAYS OF DOING SETTING THESE BUT THIS IS THE EASIEST.
    // IF YOU WANT TO LATER ADD CODE FOR OTHER BAUD RATES, REMEMBER
    // THAT THE ARGUMENT FOR BuildCommDCB MUST BE A POINTER TO A STRING.
    // ALSO NOTE THAT BuildCommDCB() DEFAULTS TO NO HANDSHAKING.

    dcbCommPort.DCBlength = sizeof( DCB );
    GetCommState( handle, & dcbCommPort );
    BuildCommDCB( config, & dcbCommPort );
    dcbCommPort.fRtsControl = RTS_CONTROL_ENABLE;
    dcbCommPort.fDtrControl = DTR_CONTROL_ENABLE;
    dcbCommPort.fNull = 0; // disable NULL discard

    if ( !SetCommState( handle, & dcbCommPort ) )
    {
        fprintf( stderr, "Could Not set comm port parameters : %s\n" , config );
        exit( -1 );
    }
#endif
    Dev = &handle;

    return Dev;
}

void *serialPort::reOpen(void *newDev)
{
    Dev = newDev;

#ifdef __linux__
    handle = *(int *)Dev;
#endif
#ifdef __WIN32__
    handle = *(HANDLE *)Dev;
#endif

    return Dev;
}

int serialPort::portClose(void)
{
    int ret;
#ifdef __WIN32__
    SetCommTimeouts( handle, &OldTimeOut );
    ret = CloseHandle( handle );
#endif
#if __linux__
    tcsetattr( handle, TCSANOW, &term_save );
    ret = close( handle );
#endif
    return ret;
}

int serialPort::portRead(char *buffer, int len)
{
    timeT now;
    timeT timeoutTime;
    int ret = -1;
    struct timeval tv;
    char rxChar;
#ifdef __linux__
    int i;
#endif
#ifdef __WIN32__
    unsigned long i;
#endif
    int bytes;

    len = len; // compiler warning

    bytes = 0;
    buffer[0] = '\0';

    now = getTime();
    timeoutTime = now + timeout;

    i = 0;
    while (rxdChars > 0)
    {
        buffer[bytes++] = tempBuffer[i++];
        rxdChars--;
    }

    while ((now < timeoutTime) && (ret == -1))
    {
#ifdef __linux__
        i = read( handle, &rxChar, 1 );
#endif
#ifdef __WIN32__
         ReadFile( handle, &rxChar, 1, &i, NULL );
#endif
        if (i > 0)
        {
            buffer[bytes++] = rxChar;
            if (rxChar == eosMode)
            {
                ret = bytes;
            }
        }

        now = getTime();
    }
    buffer[bytes] = '\0';

    return ret;
}

int serialPort::portWrite(char *buffer, int len)
{
    int ret = 0;
	timeT now;
    timeT timeoutTime;


    if (len <= 0)
    {
	return ret;
    }
#ifdef __WIN32__
    unsigned long Errors;
    COMSTAT Status;
    DWORD Bytes;
    unsigned int i;

    CancelIo( handle ); // Cancel any pending TX characters

    Errors = ~0; // Clear All errors
    ClearCommError( handle, & Errors, & Status );
    PurgeComm( handle, PURGE_TXCLEAR | PURGE_RXCLEAR );

    now = getTime();

	for(i=0;i<len;i++)
	{
		WriteFile( handle, &buffer[i] , 1, & Bytes, NULL );
		//printf("%c", buffer[i]);
		if (pace != 0)
		{
			timeoutTime = now + pace;
			while (now < timeoutTime)
			{
				now = getTime();
				//printf(".");
			}
		}
	}

    ret = Bytes;
#endif
#if __linux__
    tcflush( handle, TCIOFLUSH );
    ret = write( handle, buffer, len );

    if (ret == -1)
    {
        perror("serialPort::portWrite");
    }

    tcdrain( handle );
#endif

    return ret;
}

int serialPort::portMode(setType setting, char *set)
{
    switch (setting)
    {
        case CONFIG:
            strcpy(config, set);
            break;
        case EOSMODE:
            eosMode = *set;
            break;
        case CLEAR:
#ifdef __WIN32__
			SetCommBreak( handle );
			Sleep ( 300 ); // sleep 300 ms
			ClearCommBreak( handle );
			Sleep ( 50 ); // sleep 50 ms
#endif
#ifdef __linux__
			printf("serialPort::portMode break 300\n");
			tcsendbreak( handle, 300); // should send 300 ms break
			usleep(100000); // a bit of a guard after
			tcdrain( handle );
#endif
            break;
        case TIMEOUT:
            timeout = strtoul(set, NULL, 10) * 1000L;
            break;
        case PACING:
            pace = strtoul(set, NULL, 10) * 1000L;
            break;
    }

    return 0;
}

int serialPort::dataAvaliable(void)
{
    char rxChar;

#ifdef __linux__
    int i;
#endif
#ifdef __WIN32__
    unsigned long i;
#endif

#ifdef __linux__
    i = read( handle, &rxChar, 1 );
#endif
#ifdef __WIN32__
    ReadFile( handle, &rxChar, 1, &i, NULL );
#endif

    if (i > 0)
    {
      tempBuffer[rxdChars] = rxChar;
      rxdChars++;
    }

    return rxdChars;
}


int serialPort::portService(void)
{
    return 0;
}
