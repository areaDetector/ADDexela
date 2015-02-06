// DexelaDetector.h : main header file for the DexelaDetector DLL
//
/*! \file */
#pragma once
#ifndef DEX_BUILD
#ifdef _DEBUG
#pragma comment(lib,"DexelaDetector-d.lib")
#else
#pragma comment(lib,"DexelaDetector.lib")
#endif
#endif

#include <vector>
#include "DexDefs.h"
#include "DexImage.h"
#include "DexelaException.h"
#include <boost/shared_ptr.hpp>


#pragma warning(disable: 4251)

/// <summary>
/// This class is used to control any interface-type Detector and acquire images from it. 
/// It will provide all the basic functionality required for all different Dexela detectors. 
/// For interface specific functionality please see the interface specific classes (e.g. DexelaDetectorGE, DexelaDetectorCL).
/// </summary>
class  DllExport DexelaDetector 
{
	typedef void (*IMAGE_CALLBACK)(int fc, int buf, DexelaDetector* det);

private:

	void* pyData;
	char param[50];
	DexelaException* cbException;
	
	bool callbackActive;
	UINT CallbackCounterThread();
	IMAGE_CALLBACK callback;

	HANDLE* detHandle;

	void* callbackWorker;

	//struct containing relevant info for threads
	struct threadInfo
	{
		int FC;
		int bufNum;
		DexelaDetector* det;
		threadInfo(int fc, int BufNum, DexelaDetector* Det)
		{
			FC = fc;
			bufNum = BufNum;
			det = Det;
		}
	};

protected:
	boost::shared_ptr<baseDetector> base;
	boost::shared_ptr<gigEDetector> gigeDet;
	boost::shared_ptr<camLinkDetector> clDet;
public:

	DexelaDetector(DevInfo &devInfo);
	DexelaDetector(DetectorInterface transport, int unit, const char* params);
	virtual ~DexelaDetector(void);

	virtual void OpenBoard();
	void OpenBoard(int NumBufs);
	void CloseBoard();
	
	int GetBufferXdim(void);
	int GetBufferYdim(void);
	int GetNumBuffers(void);
	int GetCapturedBuffer(void);
	int GetFieldCount(void);
	void ReadBuffer(int bufNum, byte* buffer);
	void ReadBuffer(int bufNum, DexImage &img, int iZ=0);
	void WriteBuffer(int bufNum, byte* buffer);

	void SetFullWellMode(FullWellModes fwm);
	void SetExposureMode( ExposureModes mode);
	void SetExposureTime(float timems);
	void SetBinningMode(bins flag);
	void SetTestMode(BOOL SetTestOn);
	void SetTriggerSource( ExposureTriggerSource ets);
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
	void GetFirmwareBuild(int& iDayAndMonth, int& iYear, int& iTime);
	DetectorInterface GetTransportMethod();
	double GetReadOutTime();
	//float GetReadOutTime();
	bool IsCallbackActive();
	bool IsLive();

	void Snap(int buffer, int timeout);

	int ReadRegister(int address, int sensorNum=1);
	void WriteRegister(int address, int value, int sensorNum=0);
	void ClearCameraBuffer(int i);
	void ClearBuffers();
	void LoadSensorConfigFile(char* filename);
	void SoftReset(void);
	
	void GoLiveSeq(int start, int stop,int numBuf);
	void GoLiveSeq();
	void GoUnLive();

	void SoftwareTrigger();

	void EnablePulseGenerator(float frequency);
	void EnablePulseGenerator();
	void DisablePulseGenerator();
	void ToggleGenerator(BOOL onOff);

	void WaitImage(int timeout);

	void SetCallback(IMAGE_CALLBACK func);
	void StopCallback();

	void CheckForCallbackError();
	void CheckForLiveError();

	void SetPreProgrammedExposureTimes(int numExposures, float* exposuretimes_ms);

	void SetROICoordinates(unsigned short usStartColumn, unsigned short usStartRow, unsigned short usROIWidth, unsigned short usROIHeight);
	void GetROICoordinates(unsigned short& usStartColumn,unsigned short& usStartRow,unsigned short& usROIWidth,unsigned short& usROIHeight);
	void EnableROIMode(bool bEnableROI);
	bool GetROIState();
	unsigned short GetSensorHeight(unsigned short uiSensorID=1);
	unsigned short GetSensorWidth(unsigned short uiSensorID=1);
	///////// unsigned short GetFrameCount()			/////////!!! TO BE REVIEWED if required and whether reading the register does not disturb frame transfer
	bool IsFrameCntWithinImage();
	void EnableFrameCntWithinImage(unsigned short usEnable);
	void SetSlowed(bool flag);

	void SetReadoutMode(ReadoutModes mode);
	ReadoutModes GetReadoutMode();
	int QueryReadoutMode(ReadoutModes mode);
	int QueryExposureMode( ExposureModes mode);
	int QueryTriggerSource( ExposureTriggerSource ets);
	int QueryFullWellMode(FullWellModes fwm);
	int QueryBinningMode(bins flag);

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
typedef void (*IMAGE_CALLBACK)(int fc, int buf, DexelaDetector* det);

