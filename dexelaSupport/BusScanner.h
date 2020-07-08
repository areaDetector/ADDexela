// ******************************************************
//
// Copyright (c) 2015, PerkinElmer Inc., All rights reserved
// 
// ******************************************************
//
// This class is used to scan the different interfaces and give information about devices found.
//
// ******************************************************

#pragma once

#ifndef DEX_BUILD
#ifdef _DEBUG
#pragma comment(lib,"BusScanner-d.lib")
#else
#pragma comment(lib,"BusScanner.lib")
#endif
#endif

/*! \file */
#include "DexDefs.h"
#include "DexelaDetector.h"
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace std;


/// <summary>
/// This class is used to scan the different interfaces and give information about devices found.
/// </summary>
class DllExport BusScanner
{
public:
	BusScanner(void);
	~BusScanner(void);

	int EnumerateDevices();
	int EnumerateGEDevices();
	int EnumerateCLDevices();

	DevInfo GetDevice(int index);
	DevInfo GetDeviceGE(int index);
	DevInfo GetDeviceCL(int index);

	friend class ScanMockSetter;

#ifndef MOCK_TEST
private:
#endif
	boost::shared_ptr<baseBusScanner> baseScanner;

};
