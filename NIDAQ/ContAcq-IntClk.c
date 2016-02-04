/*********************************************************************
*
* ANSI C Example program:
*    ContAcq-IntClk.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to acquire a continuous amount of
*    data using the DAQ device's internal clock.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage range.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the rate of the acquisition. Also set the Samples per
*       Channel control. This will determine how many samples are
*       read at a time. This also determines how many points are
*       plotted on the graph each time.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Set the rate for the sample clock. Additionally, define the
*       sample mode to be continuous.
*    4. Call the Start function to start the acquistion.
*    5. Read the data in the EveryNCallback function until the stop
*       button is pressed or an error occurs.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>
#include <windows.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

#define nContSamples 1

SYSTEMTIME stTimeNow;

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"",1.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,nContSamples));

	//DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(taskHandle,DAQmx_Val_Acquired_Into_Buffer,nContSamples,0,EveryNCallback,NULL));
	//DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Press Enter to interrupt\n");

	static int  totalRead=0;
	int32       read=0;
	float64     data[nContSamples];
	int32		i;
	
	while (!kbhit())
	{
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,nContSamples,10.0,DAQmx_Val_GroupByScanNumber,data,nContSamples,&read,NULL));
		GetLocalTime(& stTimeNow);
		if( read>0 ) 
		{
			totalRead+=read;
			// printf("Acquired %d samples. Total %d\n",read,totalRead);
			
			printf( "%04d-%02d-%02d %02d:%02d:%02d.%03d",
					stTimeNow.wYear, stTimeNow.wMonth, stTimeNow.wDay,
					stTimeNow.wHour, stTimeNow.wMinute,
					stTimeNow.wSecond, stTimeNow.wMilliseconds );		
					
			for(i=0;i<read;i++)
			{
				printf("%d , %lf\n", i, data[i]);
			}
			fflush(stdout);
		}
	}
	
	
	getchar();

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program\n");
	// getchar();
	return 0;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32       error=0;
	char        errBuff[2048]={'\0'};
	static int  totalRead=0;
	int32       read=0;
	float64     data[nContSamples];
	int32		i;

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	GetLocalTime(& stTimeNow);
	DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,nContSamples,10.0,DAQmx_Val_GroupByScanNumber,data,nContSamples,&read,NULL));
	if( read>0 ) {
		totalRead+=read;
		// printf("Acquired %d samples. Total %d\n",read,totalRead);
		
		printf( "%04d-%02d-%02d %02d:%02d:%02d.%03d",
				stTimeNow.wYear, stTimeNow.wMonth, stTimeNow.wDay,
				stTimeNow.wHour, stTimeNow.wMinute,
				stTimeNow.wSecond, stTimeNow.wMilliseconds );		
				
		for(i=0;i<read;i++)
		{
			printf("%d , %lf\n", i, data[i]);
		}
		fflush(stdout);
	}

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
