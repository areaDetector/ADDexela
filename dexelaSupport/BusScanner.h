// ******************************************************
//
// Copyright (c) 2020, Varex Imaging Corp., All rights reserved
//
// ******************************************************
//
// This class is used to scan the different interfaces and give information
// about devices found.
//
// ******************************************************

/// \file

#pragma once

#include "DexDefs.h"
#include "DexelaDetector.h"

#include <memory>

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

	friend class base_BusScan;
	friend class BusScan;

private:
	std::shared_ptr<baseBusScanner> baseScanner;
};
