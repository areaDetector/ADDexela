// ******************************************************
//
// Copyright (c) 2020, Varex Imaging Corp., All rights reserved
//
// ******************************************************
//
// This class is used to control GigE Type Detectors. It will give access
// to functions that are not available to other interface-type detectors.
//
// ******************************************************

/// \file

#pragma once
#include "DexelaDetector.h"

/// <summary>
/// This class is used to control GigE Type Detectors. It will give access to functions that are not available to other interface-type detectors.
/// \n<b>Note:</b> For all standard detector function calls please see the DexelaDetector class (these functions are also available to DexelaDetectorGE objects)
/// </summary>
class DllExport DexelaDetectorGE : public DexelaDetector
{
public:
	DexelaDetectorGE(DevInfo &devInfo);
	DexelaDetectorGE(DetectorInterface transport, int unit, const char *params);
	virtual ~DexelaDetectorGE(void);

	void SetPersistentIPAddress(int firstByte, int secondByte, int thirdByte,
															int fourthByte);
	void OpenBoard();
	void OpenBoard(int NumBufs);
};
