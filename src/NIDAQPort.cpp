/*
 * NIDAQ Port interface
 *
 * Peter Jansen
 * UTAS 2010
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __WIN32__
#include <windows.h>
#define socklen_t int
#endif

#ifdef __linux__
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <NIDAQmx.h>

#include "port.hpp"
#include "common.h"
#include "timeStamp.h"

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

TaskHandle  taskHandle = 0;
int32       error = 0;
char        errBuff[2048] = {'\0'};
int 		dataAv = 0;
int 		timeout = 1000;

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	dataAv = 1;
	
	return 0;
}

NIDAQPort::NIDAQPort(char *newDev) : port(newDev)
{
    type = NIDAQ;
	timeout = 1000; // 1 second
	
	// printf("NIDAQ port constructor\n");
	
	/*********************************************/
	// DAQmx Create a task
	/*********************************************/
	DAQmxCreateTask("", &taskHandle);

	if( DAQmxFailed(error) )
	{
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
		if( taskHandle!=0 ) 
		{
			/*********************************************/
			// DAQmx Stop Code
			/*********************************************/
			DAQmxStopTask(taskHandle);
			DAQmxClearTask(taskHandle);
		}
		if( DAQmxFailed(error) )
			printf("DAQmx Error: %s\n", errBuff);
	}
}

void *NIDAQPort::portOpen(void)
{
	// printf("NIDAQ port portOpen %s\n", deviceString);
	
	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/

	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle, deviceString, "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));
	
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle, "", 1000.0/timeout, DAQmx_Val_Rising,DAQmx_Val_ContSamps, 1));
	
	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle, DAQmx_Val_Acquired_Into_Buffer, 1, 0, EveryNCallback, NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	Dev = &taskHandle;
	
    return Dev;
	
Error:
	if( DAQmxFailed(error) ) 
	{
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return NULL;	
}

void *NIDAQPort::reOpen(void *newDev)
{
    Dev = newDev;

    return Dev;
}

int NIDAQPort::portClose(void)
{
	// printf("NIDAQ port portClose\n");
	
	if( taskHandle!=0 ) 
	{
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
    return 0;
}

int NIDAQPort::portRead(char *buffer, int len)
{
	int ret = 0;
	int32 read = 0;
	float64 data[1];
		
	ret = DAQmxReadAnalogF64(taskHandle, 1, 0.0, DAQmx_Val_GroupByScanNumber, data, 1, &read, NULL);
	
	sprintf(buffer, "%lf", data[0]);
	
	// printf("NIDAQPort::portRead Read Returned %d\n", ret);
	
    return ret;
}

int NIDAQPort::portWrite(char *buffer, int len)
{
}

int NIDAQPort::dataAvaliable(void)
{
	if (dataAv > 0)
	{
		dataAv = 0;
		return 	1;
	}
	
	return 0;
}

int NIDAQPort::portService(void)
{
    return 0;
}

int NIDAQPort::portMode(setType setting, char *set)
{
    switch (setting)
    {
        case TIMEOUT:
            timeout = strtoul(set, NULL, 10);
			// printf("Timeout set %d\n", timeout);
            break;
    }

    return 0;
	return 0;
}

void NIDAQPort::print(int to)
{
	lprintf(to, "NIDAQ %s", deviceString);
}

