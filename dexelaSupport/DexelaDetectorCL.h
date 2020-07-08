// ******************************************************
//
// Copyright (c) 2015, PerkinElmer Inc., All rights reserved
// 
// ******************************************************
//
// This class is used to control CameraLink Type Detectors. It will give access to functions 
// that are not available to other interface - type detectors.
//
// ******************************************************

#pragma once
#include "dexeladetector.h"


/// <summary>
/// This class is used to control CameraLink Type Detectors. It will give access to functions that are not available to other interface-type detectors.
/// \n<b>Note:</b> For all standard detector function calls please see the DexelaDetector class (these functions are also available to DexelaDetectorCL objects)
/// </summary>
class DllExport DexelaDetectorCL :
	public DexelaDetector
{
public:
	DexelaDetectorCL(DetectorInterface transport, int unit, const char* params);
	DexelaDetectorCL(DevInfo &devInfo);
	virtual ~DexelaDetectorCL(void);
	void PowerCLInterface(bool flag);

	void OpenBoard();
	void OpenBoard(int NumBufs);
};
