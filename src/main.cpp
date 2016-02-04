/*
 * Program to Query or Send command to the serial or GPIB port
 *
 * Peter Jansen
 * Australian Antarctic Division
 * 20 April 2005
 */

/* MINGWIN compile : How to you setup the serial port when using mingwin */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "port.hpp"
#include "query.hpp"

#include "timeStamp.h"
#include "nmea.h"
#include "common.h"

#if __linux__
#include <sys/time.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <signal.h>
#endif

#ifdef __WIN32__
//#include <conio.h>
#include <windows.h>
#endif

#define READ_BUFFER_SIZE 10000            // Size of read buffer

#define CMD_PARAMS 20
#define CMD_PARAM_LEN 128

#define NO_RECORDS 2000

char ReadBuffer[READ_BUFFER_SIZE + 1]; // Read data buffer

int exitSig;

timeT timeNow;

int records;
int record;
char * logFileName;
int printStat = 0;

query *q[NO_RECORDS];

#ifdef __WIN32__
BOOL CtrlHandler( DWORD fdwCtrlType )
{
	printf("\nCtrlHandler\n");

    switch ( fdwCtrlType )
    {
        // Handle the CTRL-C signal.
        case CTRL_C_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_LOGOFF_EVENT:
        case CTRL_SHUTDOWN_EVENT:
            break;

        default:
            return FALSE;
    }

    if (exitSig == -1)
    {
    	printf("CtrlHandler: already set\n");
    	exit(-1);
    }

    exitSig = -1;

    return TRUE;
}
#endif

#if __linux__
void Exit( int sig )
{
    exitSig = sig;
    signal( SIGHUP, SIG_DFL );
    signal( SIGINT, SIG_DFL );
    signal( SIGQUIT, SIG_DFL );
    signal( SIGTERM, SIG_DFL );
}
#endif

/* print usage */

void usage( void )
{
    fprintf( stderr, "Usage : Query [OPTIONS] <String> <args>\n" );
    fprintf( stderr, " <string> is the sent message, can be strftime chars, \\n \\r \\xHH\n" );
#if ENABLE_GPIB
    fprintf( stderr, " GPIB options:\n" );
    fprintf( stderr, "   -a <GPIB address>\n" );
    fprintf( stderr, "   -u - wait for gpib SRQ line before sending command and reading\n" );
    fprintf( stderr, "   -c - clear device first\n" );
    fprintf( stderr, "   -et - disable EOT mode\n" );
#endif
    fprintf( stderr, " Serial options:\n" );
    fprintf( stderr, "   -d <serial port (eg COM1)>\n" );
    fprintf( stderr, "   -s <serial port setup (19200,N,8,1)>\n" );
	fprintf( stderr, "   -c - send break first\n" );
	fprintf( stderr, "   -P <delay> - wait <delay> between characters\n" );

#if ENABLE_NIDAQ
    fprintf( stderr, " NIDAQ options:\n" );
    fprintf( stderr, "   -N <NIDAQ device (eg Dev1/ai0)>\n" );
#endif

    fprintf( stderr, " IP options:\n");
    fprintf( stderr, "   -i [UDP:]<hostname>:<port> - connect to hostname and port\n");

    fprintf( stderr, " Program options:\n" );
    fprintf( stderr, "   -x <program>\n" );

    fprintf( stderr, " Time/Repeat options:\n" );
    fprintf( stderr, "   -f [+][[hh:]mm:]ss - first time (abs or offset)\n" );
    fprintf( stderr, "   -l [+][[hh:]mm:]ss - last time (abs or offset)\n" );
    fprintf( stderr, "   -p [[hh:]mm:]ss - period\n" );
    fprintf( stderr, "   -n <number of messages>\n" );

    fprintf( stderr, " Read options:\n" );
    fprintf( stderr, "   -r - read device\n" );
    fprintf( stderr, "   -q <ms> - set read timeout\n" );
    fprintf( stderr, "   -es <x> - set EOS mode to char x\n" );

    fprintf( stderr, " NMEA option:\n" );
    fprintf( stderr, "   -m enable nmea mode, send message checksum's and check sum checking\n" );
    fprintf( stderr, "   -mx enable xcat nmea mode, send message checksum's and check sum checking\n" );

    fprintf( stderr, " Output options:\n" );
    fprintf( stderr, "   -h <line header>\n" );

    fprintf( stderr, "   -F <filename> - load config from file\n" );
    fprintf( stderr, "   -t - don't print timestamp\n" );
    fprintf( stderr, "   -b - print control characters\n" );
    fprintf( stderr, "   -g - print stats when finished\n" );
    fprintf( stderr, "   -o <filename> - append to file, and stdout, accepts strftime chars (EG %%Y%%m%%d-%%H%%M%%S) \n" );
    fprintf( stderr, "   -or <filename> - append to file, and stdout, accepts strftime chars, roll log file\n" );
}

char *cmdlineargs[5];
int cmdargsn = 0;

int config( char cmdLine[CMD_PARAMS][CMD_PARAM_LEN],timeT time )
{
    char fileConfig[CMD_PARAMS][CMD_PARAM_LEN];
    int arg = 0;
    int argParam = 0;
    char * p;
    int newRecord = 0;
    char commandFileName[256];
    query *newQ;
    while ( cmdLine[arg][0] != '\0' )
    {
        p = cmdLine[arg];

        if ( * p == '-' )
        {
            switch ( p[1] )
            {
                case 'r':
                    newQ->receiveWait = 1;
                    break;
                case 'm':
                    newQ->nmea = 1;
                    if ( p[2] == 'x' )
                    {
                        newQ->nmea = 2; // xcat mode
                    }
                    break;
                case 'c':
                    newQ->sendClear = 1;
                    break;
                case 'f':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->setFirstTime(p, time);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -f\n" );
                        exit( -1 );
                    }
                    break;
                case 'l':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->setLastTime(p, time);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -l\n" );
                        exit( -1 );
                    }
                    break;
                case 'p':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->setPeriod(p);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -p\n" );
                        exit( -1 );
                    }
                    break;
#if ENABLE_GPIB
                case 'a':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ = new query();
                        q[record++] = newQ;

                        newQ->device = new gpibPort(p);

                        newRecord = 1;
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -a\n" );
                        exit( -1 );
                    }
                    break;
#endif
                case 'n':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->number = strtoul( p, NULL, 10 );
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -n\n" );
                        exit( -1 );
                    }
                    break;
                case 'q': // set the timeout
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->device->portMode(newQ->device->TIMEOUT, p);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -q\n" );
                        exit( -1 );
                    }
                    break;
                case 'P': // set character pacing
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->device->portMode(newQ->device->PACING, p);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -P\n" );
                        exit( -1 );
                    }
                    break;
                case 'd':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ = new query();
                        q[record++] = newQ;

                        newQ->device = new serialPort(p);

                        newRecord = 1;
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -d\n" );
                        exit( -1 );
                    }
                    break;
                  case 'i':
                      p = cmdLine[++arg];
                      if ( ( * p != '\0' ) && ( * p != '-' ) )
                      {
                          newQ = new query();
                          q[record++] = newQ;

                          newQ->device = new IpPort(p);

                          newRecord = 1;
                      }
                      else
                      {
                          fprintf( stderr, "No argument for -i\n" );
                          exit( -1 );
                      }
                      break;
                case 's':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ->device->portMode(newQ->device->CONFIG, p);
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -s\n" );
                        exit( -1 );
                    }
                    break;
                case 'e':
                    char eosString[10];
                    memset(eosString, 0, sizeof(eosString));

                    if ( p[2] == 's' )
                    {
                        p = cmdLine[++arg];
                        if ( ( * p != '\0' ) && ( * p != '-' ) )
                        {
                            if (p[0] == 13)
                            {
                                eosString[0] = 13;
                            }
                            else if (p[0] == 10)
                            {
                                eosString[0] = 10;
                            }
                            else
                            {
                                eosString[0] = strtoul( p, NULL, 0 );
                            }

                            newQ->device->portMode(newQ->device->EOSMODE, eosString);
                        }
                        else
                        {
                            fprintf( stderr, "No argument for -es\n" );
                            exit( -1 );
                        }
                    }
#if ENABLE_GPIB
                    else if ( p[2] == 't' )
                    {
                        newQ->device->portMode(newQ->device->EOTMODE, (char*)"1");
                    }
#endif

                    break;
                case 'u':
                    newQ->srq = 1;
                    break;
                case 'x':
                    {
                        char *command;
                        int i;

                        p = cmdLine[++arg];
                        if ( ( * p != '\0' ) && ( * p != '-' ) )
                        {
                            command = p;

                            newQ = new query();
                            q[record++] = newQ;

                            newRecord = 1;
							newQ->send = 1;
							newQ->device = new commandPort(command);
                        }
                        else
                        {
                            fprintf( stderr, "No argument for -x\n" );
                            exit( -1 );
                        }
                    }
                    break;
				case 'j':
				{
					cmdlineargs[cmdargsn] = cmdLine[++arg]; // save the argument
					//fprintf(stderr, "\ncommand line arg %d - %s\n", cmdargsn, cmdlineargs[cmdargsn]);
					cmdargsn++;
				}
				break;
#if ENABLE_NIDAQ
                case 'N':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        newQ = new query();
                        q[record++] = newQ;

                        newQ->device = new NIDAQPort(p);

                        newRecord = 1;
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -N\n" );
                        exit( -1 );
                    }
                    break;
#endif
                case 'F':
                    {
                        FILE * readFile;
                        char line[1024];

                        newRecord = 0;
                        // read config records from a file
                        p = cmdLine[++arg];
                        if ( ( * p != '\0' ) && ( * p != '-' ) )
                        {
                            strcpy(commandFileName, p);
                        }
                        else
                        {
                            fprintf( stderr, "No argument for -F\n" );
                            exit( -1 );
                        }
                        printf("Read File : %s\n", commandFileName);

                        readFile = fopen( commandFileName, "r" );
                        if ( readFile == NULL )
                        {
                            fprintf( stderr, "Could not open file : %s\n", commandFileName );
                            exit(-1);
                        }

                        while ( !feof( readFile ) )
                        {
                            int i = 0;
                            char c;
                            char *p;

                            /* read a line from the file */
                            do
                            {
                                c = fgetc( readFile );
                                if ( ( c != EOF ) && ( c != '\r' ) && ( c != '\n' ) )
                                {
                                    line[i++] = c;
                                }
                            }
                            while ( ( i < sizeof(line) ) && ( c != '\n' ) && ( c != EOF ) && (c != '\r') );
                            line[i] = '\0';

                            memset(fileConfig, 0, sizeof(fileConfig));

                            if ( ( i > 0 ) && ( line[0] != ';' ) )
                            {
                                int paramNo = 0;

                                fprintf( stderr, "Record %3d : %s : ", record, commandFileName );
                                p = getParameter(line);
                                if (p != NULL)
                                {
                                    strcpy( fileConfig[paramNo], p);
                                }
                                while ( fileConfig[paramNo][0] != '\0' )
                                {
                                    paramNo++;
                                    p = getParameter(NULL);
                                    if (p != NULL)
                                    {
                                        strcpy( fileConfig[paramNo], p);
                                    }
                                }
                                config( fileConfig, timeNow );
                            }
                        }
                    }
                    break;

                case 'h':
                    p = cmdLine[++arg];
                    if ( ( * p != '\0' ) && ( * p != '-' ) )
                    {
                        strcpy( newQ->header, p );
                    }
                    else
                    {
                        fprintf( stderr, "No argument for -h\n" );
                        exit( -1 );
                    }
                    break;
                default:
                    fprintf( stderr, "Unknown option %s\n", p );
                    break;
            }
        }
        else
        {
            if ( argParam == 0 ) /* Send message */
            {
                // copy the command line argument to the message to send

                char *m;

                m = newQ->message;

                while (*p != '\0')
                {
                    if (*p == '\\')
                    {
                        p++;
                        if (*p == 'r')
                        {
                            *m++ = '\r';
							p++;
							newQ->messageLength++;
                        }
                        else if (*p == 'n')
                        {
                            *m++ = '\n';
							p++;
							newQ->messageLength++;
                        }
                        else if (*p == 'x')
						{
							sscanf(p, "x%2hhx", m);
							// printf("scan x %s %d\n", p, *m);
							p += 2;
							m++;
							newQ->messageLength++;
						}
						else if (*p == 'j')
						{
							p++;
							char *argp;
							int argno = 0;
							char an = *p;
							if ((an >= '0') && (an <= '9'))
							{
								sscanf(p, "%d", &argno);
								//fprintf(stderr, "\ncmdlinearg number %c %d\n", *p, argno);
								p++;
							}

							argp = cmdlineargs[argno];
							//fprintf(stderr, "\nsetting cmdlineargs %s\n", argp);
							while(*argp != '\0')
							{
								*m++ = *argp++;
								newQ->messageLength++;
							}
						}
                        else
                        {
							*m++ = *p++;
							newQ->messageLength++;
                        }
                    }
                    else
                    {
                        *m++ = *p++;
						newQ->messageLength++;
                    }
                }

                newQ->send = 1;

                if (newQ->nmea)
                {
                    int sum = 0;
                    char sumString[20];
                    char *p;

                    p = strchr(newQ->message, '$');
                    if (p != NULL)
                    {
                        p++; // skip the $

                        while (p < m) // m from above
                        {
                            if (newQ->nmea < 2)
                            {
                                sum ^= *p;
                            }
                            else
                            {
                                sum += *p;
                            }
                            p++;
                        }
                        sum = sum & 0xff;

                        if (newQ->nmea < 2)
                        {
                            *m++ = '*';
                            sprintf(sumString, "%02X", sum);
                            *m++ = sumString[0];
                            *m++ = sumString[1];
                            *m++ = '\r';
                            *m++ = '\n';
                            newQ->messageLength += 5;
                        }
                        else
                        {
                            *m++ = sum;
                            *m++ = '\r';
                            *m++ = '\n';
                            newQ->messageLength += 3;
                        }
                    }
                }
                *m = '\0';
            }
			else
			{
				fprintf(stderr, "\nextra command line arg %s\n", p);
			}
			
            argParam++;
        }
        arg++;
    }

    if (newRecord != 0)
    {
	// printf("Setting send %d length %d\n", newQ->send, newQ->messageLength);
        newQ->device->print(0);
    }

    if (newRecord != 0)
    {
        newQ->print(0);
    }

    return arg;
}

void quit( void )
{
    int i;
    int needClose;
    query pr;

    // close the port

    for ( record = 0; record < records; record++ )
    {
        pr = *q[record];
        needClose = 1;

        // check if we have already done this port
        for ( i = 0; i < record; i++ )
        {
            if (strcmp(pr.device->deviceString, q[i]->device->deviceString) == 0)
            {
                needClose = 0;
            }
        }
        if (needClose != 0)
        {
            pr.device->portClose();
        }
        /* output the message stats for all records */

        if (printStat)
        {
			lprintf(1, "Record %d Sent %d Received %d Timeouts %d",
								 record,
								 pr.sent,
								 pr.received,
								 pr.timeouts);

			/* if its a nmea record, we have check stats */
			if (pr.nmea)
			{
				lprintf(1, " Checked %d Errors %d ",
									  pr.checked,
									  pr.sumErrors);
			}

			lprintf(1, "\n");
        }

    }
}

int main( int argc, char * argv[] )
{
    int arg;

    char sTime[100];

    int cont;
    int timeStamp = 1;
    char cmdLine[CMD_PARAMS][CMD_PARAM_LEN];

    char message[1024];
    char logFileNameExp[1024] = "";
    char logFileNameLast[1024] = "";
    time_t curtime;
    struct tm *loctime;
    int rollFile = -1;

    int len;
    int i;
    int j;

    timeT rxTimeoutTime;
    timeT rxFirstCharTime;
    timeT rxLastCharTime;
    timeT txTime;

    query *pr;

    int recordAction;

    int cmdLineParam = 0;

    printControlChars = 0;

    logStream = NULL;
    logFileName = NULL;

    fprintf( stderr, "Serial/GPIB/IP Query (Build "__DATE__ " "__TIME__ ")\n\n" );

#ifdef __WIN32__
    // set control C handeler

    if ( !SetConsoleCtrlHandler( ( PHANDLER_ROUTINE )CtrlHandler, TRUE ) )
    {
        printf( "\nERROR: Could not set control handler" );
    }
#endif
#if __linux__
    signal( SIGHUP, Exit );
    signal( SIGINT, Exit );
    signal( SIGQUIT, Exit );
    signal( SIGTERM, Exit );
#endif

    timeNow = getTime();
    timeToStr( timeNow, sTime );
    fprintf( stderr, "NOW : %s\n\n", sTime );

    // deal with command line arguments
    record = 0;

    memset( cmdLine, 0, sizeof( cmdLine ) );

    if ( argc <= 1 )
    {
        usage();
        exit( -1 );
    }

    arg = 1;
    while ( arg < argc )
    {
		//fprintf( stderr, "Command Line arg %d - %s\n", arg, argv[arg]);
		
        if ( ( argv[arg] [0] == '-' ) && ( argv[arg] [1] == 't' ) )
        {
            timeStamp = 0;
        }
        else if ( ( argv[arg] [0] == '-' ) && ( argv[arg] [1] == 'b' ) )
        {
            printControlChars = 1;
        }
        else if ( ( argv[arg] [0] == '-' ) && ( argv[arg] [1] == 'g' ) )
        {
            printStat = 1;
        }
        else if ( ( argv[arg] [0] == '-' ) && ( argv[arg] [1] == 'o' ) )
        {
            if (argv[arg][2] == 'r')
            {
                rollFile = 1;
            }
            // output all to a log file
            arg++;
            if ( ( arg <= argc ) && ( argv[arg] [0] != '-' ) )
            {
                logFileName = argv[arg];
            }
            else
            {
                fprintf( stderr, "No argument for -o\n" );
                exit( -1 );
            }
        }
        else if ( ( argv[arg] [0] == '-' ) && ( argv[arg] [1] == '?' ) )
        {
            usage();
            exit( -1 );
        }
        else
        {
            strcpy( cmdLine[cmdLineParam++], argv[arg] );
        }
        arg++;
    }

    if ( cmdLine[0][0] != '\0' )
    {
        fprintf( stderr, "Record %3d : (cmd line) : ", record );
        config( cmdLine, timeNow );
    }
    records = record;

    fprintf( stderr, "\n" );

    // init the ports

    int alreadyOpen;
    void *p;

    for ( record = 0; record < records; record++ )
    {
        // check if its already open
        alreadyOpen = 0;

        for ( i = 0; ((i < record) && (alreadyOpen == 0)); i++ )
        {
            if (strcmp(q[record]->device->deviceString, q[i]->device->deviceString) == 0)
            {
                p = q[record]->device->reOpen(q[i]->device->getDevice());
                alreadyOpen = 1;
            }
        }
        if (alreadyOpen == 0)
        {
            p = q[record]->device->portOpen();
        }
        if (p == NULL)
        {
            fprintf( stderr, "Could not open port for record %d\n", record );
            exit(-1);
        }
    }

    cont = 1;
    exitSig = 0;

    // while we still have active records, or an exit signal is received

    while ( ( cont == 1 ) && ( exitSig == 0 ) )
    {
        // clean the request bits

        for(record=0;record<records;record++)
        {
            q[record]->srqReq = 0;
        }

        for(record=0;record<records;record++)
        {
            if (q[record]->srq != 0) /* do we need to look at the service for this port */
            {
                if (q[record]->srqReq == 0) /* has this port already requested a service */
                {
                    q[record]->srqReq = q[record]->device->portService();

                    if (q[record]->srqReq != 0)
                    {
                        /* for all records with the same port, set there service Request bit */
                        for(j=0;j<records;j++)
                        {
                            if ((strcmp(q[j]->device->deviceString, q[record]->device->deviceString) == 0) && (q[j]->srq != 0))
                            {
                                q[j]->srqReq = 1;
                            }
                        }
                    }
                }
            }
        }

        timeNow = getTime();
        curtime = time(NULL);
        loctime = gmtime(&curtime);
        if (logFileName != NULL)
        {
            if (rollFile != 0)
            {
                strftime(logFileNameExp, sizeof(logFileNameExp), logFileName, loctime);
                if (rollFile == -1)
                {
                    rollFile = 0;
                }
            }

            // printf("Name %s Exp %s Last %s\n", logFileName, logFileNameExp, logFileNameLast);
            if (strcmp(logFileNameExp, logFileNameLast) != 0)
            {
                if (logStream != NULL)
                {
                    fclose(logStream);
                }
                logStream = fopen( logFileNameExp, "a");
                if ( logStream == NULL )
                {
                    fprintf( stderr, "Could not open file : %s\n", logFileNameExp );
                }
                strcpy(logFileNameLast, logFileNameExp);
            }

        }

        // for each record process when required
        for ( record = 0; record < records; record++ )
        {
            pr = q[record];

            recordAction = pr->thisRecord(timeNow);

            // printf("Record Action 0x%02x\n", recordAction);

            if ((recordAction & 0x02) != 0)
            {
                pr->device->portMode(pr->device->CLEAR, (char*)"1"); /* send clear */
            }

            if ((recordAction & 0x05) != 0)
            {
                /* for send and receive action, Print the time stamp */
                if ( timeStamp != 0 )
                {
                    pr->timeStamp(1, timeNow);
                }
            }

            /* send the message */
            if ( (recordAction & 0x01) != 0)
            {
                /* hack to send the time to instruments */

		if (strchr(pr->message, '%') != NULL)
		{
                    len = strftime(message, sizeof(message), pr->message, loctime);
		}
		else
		{
		    len = pr->messageLength;
		    memcpy(message, pr->message, len);
		}

                lprintf( 1, ", Tx, " );
                cleanPrint( 1, message, len);

                pr->device->portWrite(message, len);

                pr->sent++;
                txTime = getTime();

                fflush(stdout);
                if (logStream != NULL)
                {
                    fflush(logStream);
                }
            }

            /* wait for a reply if required */
            if ((recordAction & 0x04) != 0)
            {
                len = pr->device->portRead(ReadBuffer, READ_BUFFER_SIZE);

                if ( strlen( ReadBuffer ) > 0 )
                {
                    lprintf( 1, ", Rx, ");
                    cleanPrint( 1, ReadBuffer, strlen(ReadBuffer) );

                    pr->received++;

                    if ( pr->nmea )
                    {
                        i = NmeaCheck( ReadBuffer);
                        if (i >= 0)
                        {
                            pr->checked++;
                        }
                        if (i > 0)
                        {
                            lprintf( 1, " NMEA check error");
                            pr->sumErrors++;
                        }
                    }
                }
                if ( len == -1 )
                {
                    lprintf( 1, " (Timeout)" );
                    pr->timeouts++;
                }
            }

            if ((recordAction & 0x05) != 0)
            {
                lprintf( 1, "\n" );
            }

            fflush(stdout);
            if (logStream != NULL)
            {
                fflush(logStream);
            }
        } // for all records

        // check if we need to keep going
        cont = 0;
        for ( record = 0; record < records; record++ )
        {
            if (q[record]->finished(timeNow) != 0)
            {
              cont = 1;
            }
        }
    }

    if (exitSig != 0)
    {
        printf( "SIGNAL %d\n\n", exitSig );
    }

    quit();

    return 0;
}
