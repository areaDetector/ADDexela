// ImageCallbackEx.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DexelaDetector.h"
#include "BusScanner.h"
#include "DexImage.h"
#include "DexelaException.h"
#include <conio.h>
#include <iostream>

using namespace std;

void AcquireCallbackSequence(DexelaDetector &detector);
void myCallback(int fc, int buf, DexelaDetector* det);

int _tmain(int argc, _TCHAR* argv[])
{
	int nRetCode = 0;
	try
	{
		printf("Scanning to see how many devices are present...\n");
		BusScanner scanner;
		int count = scanner.EnumerateDevices();
		printf("\nFound %d devices\n",count);
		
		for(int i = 0; i<count; i++)
		{
			DevInfo info = scanner.GetDevice(i);

			DexelaDetector det = DexelaDetector(info);

			printf("\nAcquiring images from detector with serial number: %d\n",info.serialNum);
			AcquireCallbackSequence(det);
		}
		nRetCode = 0;
	}
	catch(DexelaException ex)
	{		
		printf("\n\nException Occurred!\nDescription: %s\nFunction: %s\n",ex.what(),ex.GetFunctionName());
		printf("\nTransportMessage: %s\n",ex.GetTransportMessage());
		nRetCode = -1;
	}
	return nRetCode;
}

void AcquireCallbackSequence(DexelaDetector &detector)
{
	ExposureModes expMode = Expose_and_read;
	FullWellModes wellMode = High;
	float exposureTime = 50.0f;
	ExposureTriggerSource trigger = Internal_Software;;
	int imCnt = 0; //image count
	bins binFmt = x11;

	//connect to detector
	printf("Connecting to detector...\n");
	detector.OpenBoard();
	detector.SetFullWellMode(wellMode);
	
	//initialize detector with desired settings
	printf("Initializing detector settings...\n");
	detector.SetExposureTime(exposureTime);
	detector.SetBinningMode(binFmt);
	detector.SetExposureMode(expMode);
	detector.SetTriggerSource(trigger);
	detector.EnablePulseGenerator();

	detector.SetCallback(myCallback);
	
	//put detector into live mode
	detector.GoLiveSeq();
	bool isLive;
		
	//wait for input from user to trigger image sequence
	printf("\nPress any key to begin acquisition!\n");
	while(!_kbhit());
	_getch();
	detector.ToggleGenerator(true);
		
	//wait until user wants to quit the loop!
	while(!_kbhit())
	{
		//check for any possible exceptions in the background callback thread
		detector.CheckForCallbackError();
		detector.CheckForLiveError();

		Sleep(50);	
	}

	detector.ToggleGenerator(false);
	detector.DisablePulseGenerator();
	if(detector.IsLive())
		detector.GoUnLive();

	detector.StopCallback();
		
	//exit live mode
	detector.CloseBoard();		
}

void myCallback(int fc, int buf, DexelaDetector* det)
{
	printf("Image: %d grabbed! Image is in buffer: %d\n",fc,buf);
}