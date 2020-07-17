// ******************************************************
//
// Copyright (c) 2020, Varex Imaging Corp., All rights reserved
//
// ******************************************************
//
// This class is used to control any interface-type Detector and acquire images
// from it. It will provide all the basic functionality required for all
// different Dexela detectors.
//
// ******************************************************

/// \file

#pragma once

#include "DexDefs.h"
#include "DexImage.h"
#include "DexelaException.h"

#include <memory>
#include <string>
#include <vector>

#pragma warning(disable : 4251)

/// <summary>
/// This class is used to control any interface-type Detector and acquire images from it.
/// It will provide all the basic functionality required for all different Dexela detectors.
/// For interface specific functionality please see the interface specific classes (e.g. DexelaDetectorGE, DexelaDetectorCL).
/// </summary>
class DllExport DexelaDetector
{
	typedef void (*IMAGE_CALLBACK)(int fc, int buf, DexelaDetector *det);

private:
	void *_cbData;
	void *pyData;
	char param[50];
	DexelaException *cbException;

	bool callbackActive;
	UINT CallbackCounterThread();
	IMAGE_CALLBACK callback;

	HANDLE *detHandle;

	void *callbackWorker;

	// struct containing relevant info for threads
	struct threadInfo
	{
		int FC;
		int bufNum;
		DexelaDetector *det;
		threadInfo(int fc, int BufNum, DexelaDetector *Det)
		{
			FC = fc;
			bufNum = BufNum;
			det = Det;
		}
	};

protected:
	std::shared_ptr<baseDetector> base;
	std::shared_ptr<gigEDetector> gigeDet;
	std::shared_ptr<camLinkDetector> clDet;

public:
	DexelaDetector(DevInfo &devInfo);
	DexelaDetector(DetectorInterface transport, int unit, const char *params);
	virtual ~DexelaDetector(void);

	virtual void OpenBoard();
	void OpenBoard(int NumBufs);
	void CloseBoard();

	int GetBufferXdim(void);
	int GetBufferYdim(void);
	int GetNumBuffers(void);
	int GetCapturedBuffer(void);
	int GetFieldCount(void);
	void ReadBuffer(int bufNum, byte *buffer);
	void ReadBuffer(int bufNum, DexImage &img, int iZ = 0);
	void WriteBuffer(int bufNum, byte *buffer, int size);

	void SetFullWellMode(FullWellModes fwm);
	void SetExposureMode(ExposureModes mode);
	void SetExposureTime(float timems);
	void SetBinningMode(bins flag);
	void SetTestMode(BOOL SetTestOn);
	void SetTriggerSource(ExposureTriggerSource ets);
	void SetNumOfExposures(int num);
	int GetNumOfExposures();
	void SetGapTime(float timems);
	float GetGapTime();
	bool IsConnected();

	ExposureModes GetExposureMode();
	float GetExposureTime();
	DetStatus GetDetectorStatus();
	ExposureTriggerSource GetTriggerSource();
	BOOL GetTestMode();
	FullWellModes GetFullWellMode();
	bins GetBinningMode();
	int GetSerialNumber();
	int GetModelNumber();
	int GetFirmwareVersion();
	void GetFirmwareBuild(int &iDayAndMonth, int &iYear, int &iTime);
	DetectorInterface GetTransportMethod();
	int GetSensorVersion();
	float GetReadOutTime();
	float GetMinimumExposureTime();
	bool IsCallbackActive();
	bool IsLive();

	void Snap(int buffer, int timeout);

	int ReadRegister(int address, int sensorNum = 1);
	void WriteRegister(int address, int value, int sensorNum = 0);
	void ClearCameraBuffer(int i);
	void ClearBuffers();
	void LoadSensorConfigFile(char *filename);
	void SoftReset(void);
	void PowerSwitch(bool flag);

	void GoLiveSeq(int start, int stop, int numBuf);
	void GoLiveSeq();
	void GoUnLive();

	void SoftwareTrigger();

	void EnablePulseGenerator(float frequency);
	void EnablePulseGenerator();
	void DisablePulseGenerator();
	void ToggleGenerator(BOOL onOff);

	void WaitImage(int timeout);

	void SetCallback(IMAGE_CALLBACK func);
	void SetCallbackData(void *cbData);
	void *GetCallbackData();
	void StopCallback();
	void CheckForCallbackError();
	void CheckForLiveError();
	unsigned short GetSensorHeight(unsigned short uiSensorID = 1);
	unsigned short GetSensorWidth(unsigned short uiSensorID = 1);
	void SetSlowed(bool flag);

	void SetReadoutMode(ReadoutModes mode);
	ReadoutModes GetReadoutMode();
	int QueryReadoutMode(ReadoutModes mode);
	int QueryExposureMode(ExposureModes mode);
	int QueryTriggerSource(ExposureTriggerSource ets);
	int QueryFullWellMode(FullWellModes fwm);
	int QueryBinningMode(bins flag);
	int QueryTempReporting();
	float GetDetectorTemp(int SensorNum = 0);

	void SetPreProgrammedExposureTimes(std::vector<float> exposureTimes);
	void SetPreProgrammedExposureTimes(const float *exposureTimes, std::size_t size);
	std::vector<float> GetPreProgrammedExposureTimes();
	int GetPreProgrammedExposureTimes(float *exposureTimes, std::size_t size);

	int QueryOnBoardLinearization();
	void ToggleOnBoardLinearization(bool onOff);
	bool GetOnBoardLinearizationState();
	int QueryOnBoardXTalkCorrection();
	void ToggleOnBoardXTalkCorrection(bool onOff);
	bool GetOnBoardXTalkCorrectionState();
	int QueryOnBoardUnscrambling();
	std::string SendCommand(std::string cmd);
	void SendCommand(char *cmd, char *returnStr);
	void SendCommand(const char *const command, const std::size_t commandSize,
									 char *const returnStr);

	void SetSliceInterlacing(bool isInterlaced);
	bool GetSliceInterlacing();
	int QueryDisableSliceInterlacing();

	void SetLegacyTiming(bool isEnabled);
	bool IsLegacyTimingEnabled();
	int QueryLegacyTiming();

	void SetPixelFrameCounter(bool isEnabled);
	bool IsPixelFrameCounterEnabled();
	int QueryPixelFrameCounter();

	void GetDefaultImageSize(int &width, int &height);
	void GetMaximumROISize(int &width, int &height);

	void SetROIArea(int width, int height, int startRow, int startColumn);
	void SetROIArea(int width, int height, int startRow);
	void SetROIArea(int width, int height);
	void GetROIArea(int &width, int &height, int &startRow,
									int &startColumn) const;

	void SetROIEnabled(bool isEnabled);
	bool IsROIEnabled() const;

	void SetROIMarginEnabled(bool isEnabled);
	bool IsROIMarginEnabled() const;

	int QueryROI() const;

	void UploadFirmware(const char *filename, int board, bool verbose = false);

	friend class baseBusScanner;
	friend class MockSetter;
	friend class DexelaDetectorPy;
	friend class Dex_CL;
};

/// <summary>
/// Image callback function signature. This is the signature that any user-passed callback funcitons must adhere to.
/// </summary>
/// <param name = "fc"> The field count associated with the image that just arrived </param>
/// <param name = "buf"> The buffer number where the image was written to (this can be used by DexelaDetector::ReadBuffer() to read-out the image data). </param>
/// <param name = "det"> The DexelaDetector object that sent the image. </param>
typedef void (*IMAGE_CALLBACK)(int fc, int buf, DexelaDetector *det);
