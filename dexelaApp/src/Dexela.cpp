/* Dexela.cpp
 *
 * This is a driver for the Dexela Dexela detectors
 *
 *
 * Author: Mark Rivers
 *
 * Created:  02/07/2015
 *
 * Current author: Mark Rivers
 *
 */

#include <sys/stat.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <epicsMutex.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include <epicsMutex.h>
#include <cantProceed.h>
#include <iocsh.h>

#include "ADDriver.h"

#include "BusScanner.h"
#include "DexelaDetector.h"

#include <epicsExport.h>

#include "Dexela.h"

static const char *driverName = "Dexela";

typedef enum
{
  DEX_ACQUIRE_ACQUISITION,
  DEX_ACQUIRE_OFFSET,
  DEX_ACQUIRE_GAIN
} PEAcquisitionMode_t;

// We add an additional mode to ADImageMode = PEImageAverage

typedef enum
{
  PEImageSingle     = ADImageSingle,
  PEImageMultiple   = ADImageMultiple,
  PEImageContinuous = ADImageContinuous,
  PEImageAverage
} PEImageMode_t;

typedef enum
{
  DEX_INTERNAL_TRIGGER,
  DEX_EXTERNAL_TRIGGER,
  DEX_FREE_RUNNING,
  DEX_SOFT_TRIGGER
} PETimingMode_t;

#define TIME0        0
#define TIME0_STR    "66.5ms"
#define TIME1        1
#define TIME1_STR    "79.9ms"
#define TIME2        2
#define TIME2_STR    "99.8ms"
#define TIME3        3
#define TIME3_STR    "133.2ms"
#define TIME4        4
#define TIME4_STR    "199.9ms"
#define TIME5        5
#define TIME5_STR    "400.0ms"
#define TIME6        6
#define TIME6_STR    "999.8ms"
#define TIME7        7
#define TIME7_STR    "1999.8ms"


#define GAIN0        0
#define GAIN0_STR    "0.25pF"
#define GAIN1        1
#define GAIN1_STR    "0.5pF"
#define GAIN2        2
#define GAIN2_STR    "1pF"
#define GAIN3        3
#define GAIN3_STR    "2pF"
#define GAIN4        4
#define GAIN4_STR    "4pF"
#define GAIN5        5
#define GAIN5_STR    "8pF"

typedef enum
{
  DEX_STATUS_OK,
  DEX_STATUS_INITIALIZING,
  DEX_STATUS_RUNNING_OFFSET,
  DEX_STATUS_RUNNING_GAIN,
  DEX_STATUS_ERROR
} PEStatus_t;

typedef enum
{
  NOT_AVAILABLE,
  AVAILABLE
} Avalability_t;

typedef enum
{
  NO,
  YES
} YesNo_t;


// I don't yet know how to pass a pointer to "this" to their frame callback function.
// For now we set this to a static variable, which limits one to a single instance of this class per process
static Dexela *pGlobalDexela;

// Forward function definitions
static void acquireStopTaskC(void *drvPvt);
static void exitCallbackC(void *drvPvt);

//_____________________________________________________________________________________________

/** Configuration command for Dexel driver; creates a new Dexela object.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] detIndex The detector index in system (0=first detector, etc.)
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */
extern "C" int DexelaConfig(const char *portName, int detIndex, int numFrameBuffers, 
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new Dexela(portName, detIndex, numFrameBuffers, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}

//_____________________________________________________________________________________________

/** callback function that is called by Dexela SDK for each frame */
static void newFrameCallback(int frameCounter, int bufferNumber, DexelaDetector *pDet)
{
  pGlobalDexela->newFrameCallback(frameCounter, bufferNumber);
}


//_____________________________________________________________________________________________
/** Constructor for Dexela driver; most parameters are simply passed to ADDriver::ADDriver.
  * After calling the base class constructor this method creates a thread to collect the detector data, 
  * and sets reasonable default values the parameters defined in this class, asynNDArrayDriver, and ADDriver.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] detIndex The detector index in system (0=first detector, etc.)
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is 
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */

Dexela::Dexela(const char *portName,  int detIndex, int numFrameBuffers, 
                         int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : ADDriver(portName, 1, (int)NUM_DEXELA_PARAMS, maxBuffers, maxMemory, asynEnumMask, 0, ASYN_CANBLOCK, 1, priority, stackSize)
{
  int status = asynSuccess;
  static const char *functionName = "Dexela";
  
  int numDevices;
  int sensorX, sensorY;
  char modelName[80];
  
  pGlobalDexela = this;
  
  pAcqBuffer_           = NULL;
  pOffsetBuffer_        = NULL;
  pGainBuffer_          = NULL;
  pBadPixelMap_         = NULL;
  pPixelCorrectionList_ = NULL;
  
  /* Add parameters for this driver */
  createParam(DEX_SerialNumberString,                asynParamInt32,   &DEX_SerialNumber);
  createParam(DEX_InitializeString,                  asynParamInt32,   &DEX_Initialize);
  createParam(DEX_AcquireOffsetString,               asynParamInt32,   &DEX_AcquireOffset);
  createParam(DEX_NumOffsetFramesString,             asynParamInt32,   &DEX_NumOffsetFrames);
  createParam(DEX_CurrentOffsetFrameString,          asynParamInt32,   &DEX_CurrentOffsetFrame);
  createParam(DEX_UseOffsetString,                   asynParamInt32,   &DEX_UseOffset);
  createParam(DEX_OffsetAvailableString,             asynParamInt32,   &DEX_OffsetAvailable);
  createParam(DEX_AcquireGainString,                 asynParamInt32,   &DEX_AcquireGain);
  createParam(DEX_NumGainFramesString,               asynParamInt32,   &DEX_NumGainFrames);
  createParam(DEX_CurrentGainFrameString,            asynParamInt32,   &DEX_CurrentGainFrame);
  createParam(DEX_UseGainString,                     asynParamInt32,   &DEX_UseGain);
  createParam(DEX_GainAvailableString,               asynParamInt32,   &DEX_GainAvailable);
  createParam(DEX_GainFileString,                    asynParamOctet,   &DEX_GainFile);
  createParam(DEX_LoadGainFileString,                asynParamInt32,   &DEX_LoadGainFile);
  createParam(DEX_SaveGainFileString,                asynParamInt32,   &DEX_SaveGainFile);
  createParam(DEX_UsePixelCorrectionString,          asynParamInt32,   &DEX_UsePixelCorrection);
  createParam(DEX_PixelCorrectionAvailableString,    asynParamInt32,   &DEX_PixelCorrectionAvailable);
  createParam(DEX_PixelCorrectionFileString,         asynParamOctet,   &DEX_PixelCorrectionFile);
  createParam(DEX_LoadPixelCorrectionFileString,     asynParamInt32,   &DEX_LoadPixelCorrectionFile);
  createParam(DEX_GainString,                        asynParamInt32,   &DEX_Gain);
  createParam(DEX_DwellTimeString,                   asynParamInt32,   &DEX_DwellTime);
  createParam(DEX_NumFrameBuffersString,             asynParamInt32,   &DEX_NumFrameBuffers);
  createParam(DEX_TriggerString,                     asynParamInt32,   &DEX_Trigger);
  createParam(DEX_SyncTimeString,                    asynParamInt32,   &DEX_SyncTime);
  createParam(DEX_CorrectionsDirectoryString,        asynParamOctet,   &DEX_CorrectionsDirectory);
  createParam(DEX_FrameBufferIndexString,            asynParamInt32,   &DEX_FrameBufferIndex);
  createParam(DEX_ImageNumberString,                 asynParamInt32,   &DEX_ImageNumber);
  createParam(DEX_SkipFramesString,                  asynParamInt32,   &DEX_SkipFrames);
  createParam(DEX_NumFramesToSkipString,             asynParamInt32,   &DEX_NumFramesToSkip);

  /* Set some default values for parameters */
  setIntegerParam(NDArraySize, 0);
  setIntegerParam(NDDataType, NDUInt16);
  setIntegerParam(DEX_DwellTime, 0);
  setIntegerParam(DEX_SyncTime, 100);
  setIntegerParam(DEX_Initialize, 0);
  setIntegerParam(DEX_AcquireOffset, 0);
  setIntegerParam(DEX_OffsetAvailable, NOT_AVAILABLE);
  setIntegerParam(DEX_AcquireGain, 0);
  setIntegerParam(DEX_GainAvailable, NOT_AVAILABLE);
  setIntegerParam(DEX_PixelCorrectionAvailable, NOT_AVAILABLE);
  setStringParam (DEX_CorrectionsDirectory, "");
  setStringParam (DEX_GainFile, "");
  setStringParam (DEX_PixelCorrectionFile, "");
  setIntegerParam(DEX_FrameBufferIndex, 0);
  setIntegerParam(DEX_ImageNumber, 0);

  try {
    pBusScanner_ = new BusScanner();
    numDevices = pBusScanner_->EnumerateDevices();
    if (numDevices < 0) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: no Dexela devices found\n",
        driverName, functionName);
      return;
    }

    if (detIndex > numDevices-1) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: detector index %s not available, only %d devices found\n",
        driverName, functionName, detIndex, numDevices);
      return;
    }
    
    devInfo_ = pBusScanner_->GetDevice(detIndex);
    pDetector_ = new DexelaDetector(devInfo_);
    sensorX = pDetector_->GetSensorWidth();
    setIntegerParam(ADMaxSizeX, sensorX);
    sensorY = pDetector_->GetSensorHeight();
    setIntegerParam(ADMaxSizeY, sensorY);
    setStringParam(ADManufacturer, "Perkin Elmer");
    sprintf(modelName, "Dexela %d", pDetector_->GetModelNumber());
    setStringParam(ADModel, modelName);
    setIntegerParam(DEX_SerialNumber, pDetector_->GetSerialNumber());

    // Connect to board
    pDetector_->OpenBoard(numFrameBuffers);

    // Set callback
    pDetector_->SetCallback(::newFrameCallback);

    // get int times for selected binning mode
  Acquisition_GetIntTimes(hAcqDesc_, m_pTimingsListBinning, &timings))!=HIS_ALL_OK)

  //  set detector timing mode
  Acquisition_SetCameraMode(hAcqDesc_, 0))!=HIS_ALL_OK)

  // Get the hardware header information
  Acquisition_GetHwHeaderInfoEx(hAcqDesc_, &cHwHeaderInfo_, 
                                                  &cHwHeaderInfoEx_)) != HIS_ALL_OK)

  } catch (DexelaException &e) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: %s\n",
      driverName, functionName, e.what());
    return;
  }
 
  // Update readback values
  setIntegerParam(ADMaxSizeX, uiColumns_);
  setIntegerParam(ADMaxSizeY, uiRows_);
  setIntegerParam(ADSizeX, uiColumns_);
  setIntegerParam(ADSizeY, uiRows_);
  setIntegerParam(NDArraySizeX, uiColumns_);
  setIntegerParam(NDArraySizeY, uiRows_);
  setIntegerParam(DEX_SystemID, dwSystemID_);
  setIntegerParam(DEX_Gain, iGain);
  setIntegerParam(DEX_DwellTime, iTimeIndex);
  setIntegerParam(DEX_NumFrameBuffers, uiNumFrameBuffers_);
  setIntegerParam(ADStatus, ADStatusIdle);
 
   acquireStopEvent_ = epicsEventCreate(epicsEventEmpty);
  /* Create the thread that updates the images */
  status = (epicsThreadCreate("acquireStopTaskC",
                              epicsThreadPriorityMedium,
                              epicsThreadGetStackSize(epicsThreadStackMedium),
                              (EPICSTHREADFUNC)acquireStopTaskC,
                              this) == NULL);
  
  // Set exit handler to clean up
  epicsAtExit(exitCallbackC, this);

}

//_____________________________________________________________________________________________

/** Callback function that is called by EPICS when the IOC exits */
static void exitCallbackC(void *pPvt)
{
  Dexela *pDexela = (Dexela*) pPvt;
  delete(pDexela);
}

//_____________________________________________________________________________________________
/** Destructor for Dexela driver; most parameters are simply passed to ADDriver::ADDriver.
 * Frees all resources and calls Acquisition_Close() */

Dexela::~Dexela()
{
  Acquisition_Close (hAcqDesc_);

  if (pAcqBuffer_ != NULL)
    free(pAcqBuffer_);

  if (pOffsetBuffer_ != NULL)
    free(pOffsetBuffer_);

  if (pGainBuffer_ != NULL)
    free(pGainBuffer_);

  if (pBadPixelMap_ != NULL)
    free(pBadPixelMap_);

  if (pPixelCorrectionList_ != NULL)
    free(pPixelCorrectionList_);
}


//_____________________________________________________________________________________________

/** Configures the detector binning according to the ADBinX and ADBinY parameters */
void Dexela::setBinning(void)
{
  unsigned int uiPEResult;
  int binX, binY, sizeX, sizeY;
  int status;
  bool validBinning = true;
  WORD PEBinning;
  int averageBinning = 256;
  int accumulateBinning = 512;
  static const char *functionName = "setBinning";

  status = getIntegerParam(ADBinX, &binX);
  status |= getIntegerParam(ADBinY, &binY);
  // If either binning is not defined return because it has not yet
  // been initialized, but we will be called again when it is.
  if (status) return;
  getIntegerParam(ADMaxSizeX, &sizeX);
  getIntegerParam(ADMaxSizeY, &sizeY);

  if ((cHwHeaderInfo_.dwHeaderID >= 14) && 
      (cHwHeaderInfoEx_.wCameratype >= 2)) {
    if      ((binX == 1) && (binY == 1)) PEBinning = 1;
    else if ((binX == 2) && (binY == 2)) PEBinning = 2;
    else if ((binX == 4) && (binY == 4)) PEBinning = 3;
    else if ((binX == 1) && (binY == 2)) PEBinning = 4;
    else if ((binX == 1) && (binY == 4)) PEBinning = 5;
    else validBinning = false;
    // Hardcode average style binning for now
    PEBinning = PEBinning + averageBinning;
  } else {
    if      ((binX == 1) && (binY == 1)) PEBinning = 0;
    else if ((binX == 2) && (binY == 2)) PEBinning = 1;
    else validBinning = false;
  }
  
  if (validBinning) {
    uiPEResult = Acquisition_SetCameraBinningMode(hAcqDesc_, PEBinning);
    if (uiPEResult != HIS_ALL_OK) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s:%s:  Error: %d  Acquisition_SetCameraBinningMode failed!\n", 
        driverName, functionName, uiPEResult);
      return;
    }
    uiColumns_ = sizeX/binX;
    uiRows_    = sizeY/binY;
    asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
      "%s:%s:  set binning %d x %d, PEBinning=%d\n", 
      driverName, functionName, binX, binY, PEBinning);
  } else {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s:  invalid binning %d x %d\n", 
      driverName, functionName, binX, binY);
  }
}

//_____________________________________________________________________________________________

/** Report status of the driver.
  * Prints details about the detector in us if details>0.
  * Prints information about all local and network detectors if details>1.
  * It then calls the ADDriver::report() method.
  * \param[in] fp File pointed passed by caller where the output is written to.
  * \param[in] details Controls the level of detail in the report. */
void Dexela::report(FILE *fp, int details)
{
  unsigned int uiPEResult;
  static const char *functionName = "report";

  // Get the hardware header information
  if ((uiPEResult = Acquisition_GetHwHeaderInfoEx(hAcqDesc_, &cHwHeaderInfo_, 
                                                  &cHwHeaderInfoEx_)) != HIS_ALL_OK)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error: %d GetHwHeaderInfoEx failed!\n", 
      driverName, functionName, uiPEResult);
    return;
  }
  fprintf(fp, "Dexela %s\n", this->portName);
  if (details > 0) {
    int nx, ny, dataType;
    getIntegerParam(ADSizeX, &nx);
    getIntegerParam(ADSizeY, &ny);
    getIntegerParam(NDDataType, &dataType);
    fprintf(fp, "  Number of rows:    %d\n", nx);
    fprintf(fp, "  Number of columns: %d\n", ny);
    fprintf(fp, "  Data type:         %d\n", dataType);
    fprintf(fp, "  Channel type:      %d\n", uiChannelType_);
    fprintf(fp, "  System ID:         %d\n", dwSystemID_);
    fprintf(fp, "  Channel number:    %d\n", iChannelNum_);
    fprintf(fp, "  Frames allocated:  %d\n", uiNumFrameBuffers_);
    fprintf(fp, "  Current rows:      %d\n", uiRows_);
    fprintf(fp, "  Current columns:   %d\n", uiColumns_);
    fprintf(fp, "  # device frames:   %d\n", uiDevFrames_);
    fprintf(fp, "  Device data type:  %d\n", uiDataType_);
    fprintf(fp, "  Device sort flags: %d\n", uiSortFlags_);
    fprintf(fp, "  IRQ enabled:       %d\n", bEnableIRQ_);
    fprintf(fp, "  Acquisition type:  %d\n", dwAcqType_);
    fprintf(fp, "  SystemID:          %d\n", dwSystemID_);
    fprintf(fp, "  Sync mode:         %d\n", dwSyncMode_);
    if (cHwHeaderInfo_.dwHeaderID >= 14) {
      // This detector supports CHwHeaderInfoEx
      fprintf(fp, "  CHwHeaderInfoEx:\n");
      fprintf(fp, "    HeaderID:     %d\n", cHwHeaderInfoEx_.wHeaderID);
      fprintf(fp, "    PROMID:       %d\n", cHwHeaderInfoEx_.wPROMID);
      fprintf(fp, "    ResolutionX:  %d\n", cHwHeaderInfoEx_.wResolutionX);
      fprintf(fp, "    ResolutionY:  %d\n", cHwHeaderInfoEx_.wResolutionY);
      fprintf(fp, "    NrRows:       %d\n", cHwHeaderInfoEx_.wNrRows);
      fprintf(fp, "    NrColumns:    %d\n", cHwHeaderInfoEx_.wNrColumns);
      fprintf(fp, "    ZoomULRow:    %d\n", cHwHeaderInfoEx_.wZoomULRow);
      fprintf(fp, "    ZoomULColumn: %d\n", cHwHeaderInfoEx_.wZoomULColumn);
      fprintf(fp, "    ZoomBRColumn: %d\n", cHwHeaderInfoEx_.wZoomBRColumn);
      fprintf(fp, "    FrmNrRows:    %d\n", cHwHeaderInfoEx_.wFrmNrRows);
      fprintf(fp, "    FrmRowType:   %d\n", cHwHeaderInfoEx_.wFrmRowType);
      fprintf(fp, "    RowTime:      %d\n", cHwHeaderInfoEx_.wRowTime);
      fprintf(fp, "    Clock:        %d\n", cHwHeaderInfoEx_.wClock);
      fprintf(fp, "    DataSorting:  %d\n", cHwHeaderInfoEx_.wDataSorting);
      fprintf(fp, "    Timing:       %d\n", cHwHeaderInfoEx_.wTiming);
      fprintf(fp, "    Gain:         %d\n", cHwHeaderInfoEx_.wGain);
      fprintf(fp, "    LeakRows:     %d\n", cHwHeaderInfoEx_.wLeakRows);
      fprintf(fp, "    Access:       %d\n", cHwHeaderInfoEx_.wAccess);
      fprintf(fp, "    Bias:         %d\n", cHwHeaderInfoEx_.wBias);
      fprintf(fp, "    UgComp:       %d\n", cHwHeaderInfoEx_.wUgComp);
      fprintf(fp, "    Cameratype:   %d\n", cHwHeaderInfoEx_.wCameratype);
      fprintf(fp, "    FrameCnt:     %d\n", cHwHeaderInfoEx_.wFrameCnt);
      fprintf(fp, "    BinningMode:  %d\n", cHwHeaderInfoEx_.wBinningMode);
      fprintf(fp, "    RealInttime_milliSec: %d\n", cHwHeaderInfoEx_.wRealInttime_milliSec);
      fprintf(fp, "    RealInttime_microSec: %d\n", cHwHeaderInfoEx_.wRealInttime_microSec);
      fprintf(fp, "    Status:       %d\n", cHwHeaderInfoEx_.wStatus);
    } else {
      fprintf(fp, "  CHwHeaderInfo:\n");
      fprintf(fp, "    PROMID:       %d\n", cHwHeaderInfo_.dwPROMID);
      fprintf(fp, "    HeaderID:     %d\n", cHwHeaderInfo_.dwHeaderID);
      fprintf(fp, "    AddRow:       %d\n", cHwHeaderInfo_.bAddRow);
      fprintf(fp, "    PwrSave:      %d\n", cHwHeaderInfo_.bPwrSave);
      fprintf(fp, "    NrRows:       %d\n", cHwHeaderInfo_.dwNrRows);
      fprintf(fp, "    NrColumns:    %d\n", cHwHeaderInfo_.dwNrColumns);
      fprintf(fp, "    ZoomULRow:    %d\n", cHwHeaderInfo_.dwZoomULRow);
      fprintf(fp, "    ZoomULColumn: %d\n", cHwHeaderInfo_.dwZoomULColumn);
      fprintf(fp, "    ZoomBRRow:    %d\n", cHwHeaderInfo_.dwZoomBRRow);
      fprintf(fp, "    ZoomBRColumn: %d\n", cHwHeaderInfo_.dwZoomBRColumn);
      fprintf(fp, "    FrmNrRows:    %d\n", cHwHeaderInfo_.dwFrmNrRows);
      fprintf(fp, "    FrmRowType:   %d\n", cHwHeaderInfo_.dwFrmRowType);
      fprintf(fp, "    FrmFillRowIntervalls: %d\n", cHwHeaderInfo_.dwFrmFillRowIntervalls);
      fprintf(fp, "    NrOfFillingRows:     %d\n", cHwHeaderInfo_.dwNrOfFillingRows);
      fprintf(fp, "    DataType:     %d\n", cHwHeaderInfo_.dwDataType);
      fprintf(fp, "    DataSorting:  %d\n", cHwHeaderInfo_.dwDataSorting);
      fprintf(fp, "    Timing:       %d\n", cHwHeaderInfo_.dwTiming);
      fprintf(fp, "    Gain:         %d\n", cHwHeaderInfo_.dwGain);
      fprintf(fp, "    Offset:       %d\n", cHwHeaderInfo_.dwOffset);
      fprintf(fp, "    SyncMode:     %d\n", cHwHeaderInfo_.bSyncMode);
      fprintf(fp, "    Bias:         %d\n", cHwHeaderInfo_.dwBias);
      fprintf(fp, "    LeakRows:     %d\n", cHwHeaderInfo_.dwLeakRows);
    }
  }
  if (details > 1) reportSensors(fp, details);
  
  /* Invoke the base class method */
  ADDriver::report(fp, details);
}

//_____________________________________________________________________________________________
/** Report information about all local and network Dexela detectors */
void Dexela::reportSensors(FILE *fp, int details)
{
  ACQDESCPOS Pos = 0;
  unsigned int uiPEResult;
  unsigned int uiChannelType, uiRows, uiColumns, uiDataType, uiSortFlags;
  int iChannelNum, iFrames;
  long numGbIFSensors;
  int i;
  GBIF_DEVICE_PARAM *pGbIFDeviceParam;
  BOOL bEnableIRQ;
  DWORD dwAcqType, dwSystemID, dwSyncMode, dwHwAccess;
  HACQDESC  hAcqDesc;
  
  fprintf(fp, "Total sensors in system: %d\n", uiNumSensors_);
  // Iterate through all this sensors and display sensor data
  do
  {
    if ((uiPEResult = Acquisition_GetNextSensor(&Pos, &hAcqDesc)) != HIS_ALL_OK)
      fprintf(fp, "  Error: %d  GetNextSensor failed!\n", uiPEResult);

    fprintf(fp, "  Sensor %d\n", Pos);

    // Ask for communication device type and its number
    if ((uiPEResult = Acquisition_GetCommChannel(hAcqDesc, &uiChannelType, &iChannelNum)) != HIS_ALL_OK)
      fprintf(fp, "    Error: %d  GetCommChannel failed!\n", uiPEResult);

    // Ask for data organization
    if ((uiPEResult=Acquisition_GetConfiguration(hAcqDesc, (unsigned int *) &iFrames, &uiRows, &uiColumns, &uiDataType,
        &uiSortFlags, &bEnableIRQ, &dwAcqType, &dwSystemID, &dwSyncMode, &dwHwAccess)) != HIS_ALL_OK)
      fprintf(fp, "    Error: %d GetConfiguration failed!\n", uiPEResult);

    fprintf(fp, "    Channel type:     %d\n", uiChannelType);
    fprintf(fp, "    Channel number:   %d\n", iChannelNum);
    fprintf(fp, "    Rows:             %d\n", uiRows);
    fprintf(fp, "    Columns:          %d\n", uiColumns);
    fprintf(fp, "    Frames:           %d\n", iFrames);
    fprintf(fp, "    Data type:        %d\n", uiDataType);
    fprintf(fp, "    Sort flags:       %d\n", uiSortFlags);
    fprintf(fp, "    Enable IRQ:       %d\n", bEnableIRQ);
    fprintf(fp, "    Acquisition type: %d\n", dwAcqType);
    fprintf(fp, "    System ID:        %d\n", dwSystemID);
    fprintf(fp, "    Sync mode:        %d\n", dwSyncMode);
    fprintf(fp, "    Hardware access:  %d\n", dwHwAccess);

  } while (Pos!=0);
  
  if ((uiPEResult = Acquisition_GbIF_GetDeviceCnt(&numGbIFSensors)) != HIS_ALL_OK)
    fprintf(fp, "  Error: %d  GbIF_GetDeviceCnt failed!\n", uiPEResult);
  if (numGbIFSensors <= 0) return;
  
  pGbIFDeviceParam = (GBIF_DEVICE_PARAM *)calloc(numGbIFSensors, sizeof(GBIF_DEVICE_PARAM));
  if ((uiPEResult = Acquisition_GbIF_GetDeviceList(pGbIFDeviceParam, numGbIFSensors)) != HIS_ALL_OK)
    fprintf(fp, "  Error: %d  GbIF_GetDeviceList failed!\n", uiPEResult);
  
  fprintf(fp, "Total GbIF sensors in system: %d\n", numGbIFSensors);
  for (i=0; i<numGbIFSensors; i++) {
    fprintf(fp, "  GbIF Sensor %d\n", i+1);
    fprintf(fp, "    MAC address:     %s\n", pGbIFDeviceParam[i].ucMacAddress);
    fprintf(fp, "    IP address:      %s\n", pGbIFDeviceParam[i].ucIP);
    fprintf(fp, "    Subnet mask:     %s\n", pGbIFDeviceParam[i].ucSubnetMask);
    fprintf(fp, "    Gateway:         %s\n", pGbIFDeviceParam[i].ucGateway);
    fprintf(fp, "    Adapter IP:      %s\n", pGbIFDeviceParam[i].ucAdapterIP);
    fprintf(fp, "    Adapter mask:    %s\n", pGbIFDeviceParam[i].ucAdapterMask);
    fprintf(fp, "    Boot options:    %d\n", pGbIFDeviceParam[i].dwIPCurrentBootOptions);
    fprintf(fp, "    Manufacturer:    %s\n", pGbIFDeviceParam[i].cManufacturerName);
    fprintf(fp, "    Model:           %s\n", pGbIFDeviceParam[i].cModelName);
    fprintf(fp, "    Firmware:        %s\n", pGbIFDeviceParam[i].cGBIFFirmwareVersion);
    fprintf(fp, "    Device name:     %s\n", pGbIFDeviceParam[i].cDeviceName);  
  }  
}

//_____________________________________________________________________________________________
/** callback function that is called by XISL every frame at end of data transfer */
void Dexela::newFrameCallback(int frameCounter, int bufferNumber)
{
  unsigned int  uiStatus;
  DWORD         ActAcqFrame;
  DWORD         ActBuffFrame;
  unsigned int  currBuff;
  epicsUInt16   *pInput;
  NDArrayInfo   arrayInfo;
  int           arrayCounter;
  int           imageCounter;
  int           numImages;
  int           imageMode;
  int           offsetCounter;
  int           gainCounter;
  int           useOffset;
  int           useGain;
  int           usePixelCorrection;
  size_t        dims[2];
  int           acquiring;
  DWORD         HISError;
  DWORD         FGError;
  NDArray       *pImage;
  NDDataType_t  dataType;
  epicsTimeStamp currentTime;
  static const char *functionName = "endFrameCallback";
    
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: entry ...\n",
    driverName, functionName);

  lock();
  uiStatus =  Acquisition_GetAcqData(hAcqDesc, (void **) &pDexela);
  getIntegerParam(DEX_UseOffset, &useOffset);
  getIntegerParam(DEX_UseGain, &useGain);
  getIntegerParam(DEX_UsePixelCorrection, &usePixelCorrection);

  switch (iAcqMode_) {
    case DEX_ACQUIRE_OFFSET:
      getIntegerParam(DEX_CurrentOffsetFrame, &offsetCounter);
      offsetCounter++;
      setIntegerParam(DEX_CurrentOffsetFrame, offsetCounter);
      pInput = pOffsetBuffer_;
      dataType = NDUInt16;
      break;

    case DEX_ACQUIRE_GAIN:
      getIntegerParam(DEX_CurrentGainFrame, &gainCounter);
      gainCounter++;
      setIntegerParam(DEX_CurrentGainFrame, gainCounter);
      pInput = (epicsUInt16 *)pGainBuffer_;
      dataType = NDInt32;
      break;

    case DEX_ACQUIRE_ACQUISITION:
      /** Find offset into secondary frame buffer */
      uiStatus =  Acquisition_GetActFrame(hAcqDesc, &ActAcqFrame, &ActBuffFrame);
      if (uiStatus != 0) {
        Acquisition_GetErrorCode(hAcqDesc,  &HISError,  &FGError);
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
          "%s:%s: Error: %d Acquisition_GetActFrame failed, HIS Error: %d, Frame Grabber Error: %d\n", 
          driverName, functionName, uiStatus, HISError, FGError);
      }
      getIntegerParam(ADImageMode, &imageMode);
      getIntegerParam(ADNumImages, &numImages);
      getIntegerParam(ADNumImagesCounter, &imageCounter);
      imageCounter++;
      setIntegerParam(ADNumImagesCounter, imageCounter);
      asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
        "%s:%s: ActAcqFrame = %d, ActBuffFrame = %d\n", 
        driverName, functionName, ActAcqFrame, ActBuffFrame);
      setIntegerParam(DEX_ImageNumber, ActBuffFrame);
      setIntegerParam(DEX_FrameBufferIndex, ActAcqFrame);
      asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
        "%s:%s: uiNumBuffersInUse_ = %d\n", 
        driverName, functionName, uiNumBuffersInUse_);
      if (imageMode == PEImageAverage) {
        // We don't want to do anything if we are in average mode except if
        // we have been called from the endAcqCallback, in which case ADAcquire will be 0
        getIntegerParam(ADAcquire, &acquiring);
        if (acquiring) goto done;
        // We force it to use the first buffer, even though Acquisition_GetActFrame 
        // says ActBuffFrame is something else
        ActBuffFrame = 1;
      }
      currBuff = (ActBuffFrame - 1) % uiNumBuffersInUse_;
      pInput = &pAcqBuffer_[uiColumns_ * uiRows_ * currBuff];
      dataType = NDUInt16;
      asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
        "%s:%s: currBuff = %d, pInput = %p\n", 
        driverName, functionName, currBuff, pInput);

      /** Correct for detector offset and gain as necessary */
      if ((useOffset) && (pOffsetBuffer_ != NULL))
      {
        if ((useGain) && (pGainBuffer_ != NULL))
          uiStatus = Acquisition_DoOffsetGainCorrection(pInput, pInput, pOffsetBuffer_, pGainBuffer_, uiRows_ * uiColumns_);
        else
          uiStatus = Acquisition_DoOffsetCorrection(pInput, pInput, pOffsetBuffer_, uiRows_ * uiColumns_);
      }

      /** Correct for dead pixels as necessary */
      if ((usePixelCorrection) && (pPixelCorrectionList_ != NULL))
        uiStatus = Acquisition_DoPixelCorrection (pInput, pPixelCorrectionList_);

      if ((imageMode == PEImageMultiple) && (imageCounter >= numImages))
        acquireStop();
      break;
  }

  /* Update the image */
  /* We save the most recent image buffer so it can be used in the read() function.
   * Now release it before getting a new version. */
  if (this->pArrays[0])
      this->pArrays[0]->release();
  /* Allocate the array */
  dims[0] = uiColumns_;
  dims[1] = uiRows_;
  this->pArrays[0] = pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
  if (this->pArrays[0] == NULL) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: error allocating buffer\n",
      driverName, functionName);
    unlock();
    return;
  }
  pImage = this->pArrays[0];
  pImage->getInfo(&arrayInfo);
  // Copy the data from the input to the output
  memcpy(pImage->pData, pInput, arrayInfo.totalBytes);

  setIntegerParam(NDArraySize,  (int)arrayInfo.totalBytes);
  setIntegerParam(NDArraySizeX, (int)pImage->dims[0].size);
  setIntegerParam(NDArraySizeY, (int)pImage->dims[1].size);


  /* Put the frame number and time stamp into the buffer */
  getIntegerParam(NDArrayCounter, &arrayCounter);
  arrayCounter++;
  setIntegerParam(NDArrayCounter, arrayCounter);
  pImage->uniqueId = arrayCounter;
  epicsTimeGetCurrent(&currentTime);
  pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
  updateTimeStamp(&pImage->epicsTS);

  /* Get any attributes that have been defined for this driver */
  getAttributes(pImage->pAttributeList);

  /* Call the NDArray callback */
  /* Must release the lock here, or we can get into a deadlock, because we can
   * block on the plugin lock, and the plugin can be calling us */
  unlock();
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: calling imageData callback\n", 
    driverName, functionName);
  doCallbacksGenericPointer(pImage, NDArrayData, 0);
  lock();

  done:
  // Do callbacks on parameters
  callParamCallbacks();

  unlock();
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: exit\n",
    driverName, functionName);
}

//_____________________________________________________________________________________________

/** callback function that is called by XISL at end of acquisition */
static void CALLBACK endAcqCallbackC(HACQDESC hAcqDesc)
{
  Dexela   *pDexela;
  unsigned int  uiStatus;
  DWORD         HISError;
  DWORD         FGError;
  static const char *functionName = "endAcqCallbackC";

#ifdef __X64
  uiStatus =  Acquisition_GetAcqData(hAcqDesc, (void **) &pDexela);
#else
  uiStatus =  Acquisition_GetAcqData(hAcqDesc, (DWORD *) &pDexela);
#endif
  if (uiStatus != 0) {
    printf("%s:%s: Error: %d Acquisition_GetAcqData failed\n", 
      driverName, functionName, uiStatus);
    Acquisition_GetErrorCode(hAcqDesc, &HISError, &FGError);
    printf ("%s:%s: HIS Error: %d, Frame Grabber Error: %d\n", 
      driverName, functionName, HISError, FGError);
    return;
  }
  pDexela->endAcqCallback(hAcqDesc);
}

//_____________________________________________________________________________________________

/** callback function that is called by XISL at end of acquisition */
void Dexela::endAcqCallback(HACQDESC hAcqDesc)
{
  int imageMode;
  static const char *functionName = "endAcqCallback";

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: entry...\n",
    driverName, functionName);

  lock();
  switch(iAcqMode_) {
    case DEX_ACQUIRE_OFFSET: 
    setIntegerParam(DEX_AcquireOffset, 0);
      /* raise a flag to the user if offset data is available */
      if (pOffsetBuffer_ != NULL) {
        setIntegerParam(DEX_OffsetAvailable, AVAILABLE);
      }
      break;

  case DEX_ACQUIRE_GAIN: 
    setIntegerParam(DEX_AcquireGain, 0);
    /* raise a flag to the user if gain data is available */
    if (pGainBuffer_ != NULL) {
      setIntegerParam(DEX_GainAvailable, AVAILABLE);
    }
    setShutter(ADShutterClosed);
    break;
    
  case DEX_ACQUIRE_ACQUISITION:
    setIntegerParam(ADStatus, ADStatusIdle);
    setIntegerParam(ADAcquire, 0);
    getIntegerParam(ADImageMode, &imageMode);
    if (imageMode == PEImageAverage) {
      endFrameCallback(hAcqDesc);
    }
    setShutter(ADShutterClosed);
    break;
  
  }
  
  callParamCallbacks();
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: exit\n",
    driverName, functionName);
  unlock();
}

//_____________________________________________________________________________________________

/** Called when asyn clients call pasynInt32->write().
  * This function performs actions for some parameters, including ADAcquire, ADBinX, etc.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus Dexela::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  int adstatus;
  int status = asynSuccess;
  unsigned int uiPEResult;
  int retstat;
  static const char *functionName = "writeInt32";

  /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
   * status at the end, but that's OK */
  status = setIntegerParam(function, value);

  getIntegerParam(ADStatus, &adstatus);

  /* For a real detector this is where the parameter is sent to the hardware */
  if (function == ADAcquire) {

    // Start acquisition
    if (value && (adstatus == ADStatusIdle) )
    {
      acquireStart();
    }

    // Stop acquisition
    if (!value && (adstatus != ADStatusIdle))
    {
      acquireStop();
    }
  }
  else if ((function == ADBinX) ||
           (function == ADBinY)) {
    if ( adstatus == ADStatusIdle ) {
      setBinning();
    }    
  }
  else if (function == DEX_Initialize) {
    if ( adstatus == ADStatusIdle ) {
      initializeDetector();
    }
  }
  else if (function == DEX_AcquireOffset) {
    if ( adstatus == ADStatusIdle ) {
      acquireOffsetImage();
    }
  }
  else if (function ==  DEX_AcquireGain) {
    if ( adstatus == ADStatusIdle ) {
      acquireGainImage();
    }
  }
  else if (function == DEX_Trigger) {
    if ((uiPEResult = Acquisition_SetFrameSync(hAcqDesc_))!=HIS_ALL_OK)
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s:%s: Error: %d  Acquisition_SetFrameSync failed!\n", 
        driverName, functionName, uiPEResult);
  }

  else if (function == ADTriggerMode) {
    getIntegerParam(ADStatus, &adstatus);
    retstat |= getIntegerParam(ADTriggerMode, &iTrigModeReq_);
    asynPrint(pasynUser, ASYN_TRACE_FLOW,
      "%s:%s: Setting Requested Trigger Mode: %d\n", 
      driverName, functionName, iTrigModeReq_);
    retstat |= setIntegerParam(ADTriggerMode, iTrigModeAct_);
    //if not running go ahead and set the trigger mode
    if ( adstatus == ADStatusIdle ) {
      setTriggerMode();
    }
  }
  else if (function == DEX_LoadGainFile) {
    loadGainFile();
  }
  else if (function == DEX_SaveGainFile) {
    saveGainFile();
  }
  else if (function == DEX_LoadPixelCorrectionFile) {
    loadPixelCorrectionFile();
  }


  else {
    /* If this parameter belongs to a base class call its method */
    if (function < DEX_FIRST_PARAM) {
      status = ADDriver::writeInt32(pasynUser, value);
    }
  }

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status)
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
      "%s:%s: error, status=%d function=%d, value=%d\n",
      driverName, functionName, status, function, value);
  else
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
      "%s:%s: function=%d, value=%d\n",
      driverName, functionName, function, value);

  return (asynStatus) status;
}


//_____________________________________________________________________________________________

/** Called when asyn clients call pasynFloat64->write().
  * This function performs actions for some parameters.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks.
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus Dexela::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  int retstat;
  int adstatus;
  static const char *functionName = "writeFloat64";

  /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
   * status at the end, but that's OK */
  status = setDoubleParam(function, value);

  /* Changing any of the following parameters requires recomputing the base image */
  if (function == ADAcquireTime) {
    getIntegerParam(ADStatus, &adstatus);

    retstat |= getDoubleParam(ADAcquireTime, &dAcqTimeReq_);
    asynPrint(pasynUser, ASYN_TRACE_FLOW,
      "%s:%s: Setting Requested Acquisition Time: %f\n", 
      driverName, functionName, dAcqTimeReq_);
    retstat |= setDoubleParam(ADAcquireTime, dAcqTimeAct_);
    // if the detector is idle then go ahead and set the value
    if ( adstatus == ADStatusIdle ) {
      setExposureTime();
    }

  }
  else if (function == ADGain) {
  }
  else {
    /* If this parameter belongs to a base class call its method */
    if (function < DEX_FIRST_PARAM) {
      status = ADDriver::writeFloat64(pasynUser, value);
    }
  }

  /* Do callbacks so higher layers see any changes */
  callParamCallbacks();

  if (status)
    asynPrint(pasynUser, ASYN_TRACE_ERROR,
        "%s:%s: error, status=%d function=%d, value=%f\n",
        driverName, functionName, status, function, value);
  else
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
        "%s:%s: function=%d, value=%f\n",
        driverName, functionName, function, value);
  return status;
}

//_____________________________________________________________________________________________

/** Starts acquisition */
void Dexela::acquireStart(void)
{
  int     iMode;
  int     iFrames;
  int     status = asynSuccess;
  unsigned int uiPEResult;
  DWORD   HISError;
  DWORD   FGError;
  int     skipFrames;
  int     numFramesToSkip = 0;
  static const char *functionName = "acquireStart";

  //get some information
  getIntegerParam(ADImageMode, &iMode);
  getIntegerParam(DEX_SkipFrames, &skipFrames);

  if (skipFrames !=0) {
    status |= getIntegerParam(DEX_NumFramesToSkip, &numFramesToSkip);
    asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
      "%s:%s: Skipping %d frames\n", 
      driverName, functionName, numFramesToSkip );
    }

  switch(iMode)
  {
    case PEImageSingle:  
      iFrames = 1; 
      break;

    case PEImageMultiple:
    case PEImageContinuous:
      iFrames = uiNumFrameBuffers_;
      break;
    case PEImageAverage:
      status |= getIntegerParam(ADNumImages, &iFrames);
      break;

  }

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Frame mode: %d, Frames: %d, Rows: %d, Columns: %d\n", 
    driverName, functionName, iMode, iFrames, uiRows_, uiColumns_);

  iAcqMode_ = DEX_ACQUIRE_ACQUISITION;
  uiNumBuffersInUse_ = iFrames;

  if ((uiPEResult=Acquisition_DefineDestBuffers(hAcqDesc_, pAcqBuffer_,  iFrames, uiRows_, uiColumns_)) != HIS_ALL_OK)
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error : %d  Acquisition_DefineDestBuffers failed!\n", 
      driverName, functionName, uiPEResult);

  setIntegerParam(ADNumImagesCounter, 0);
  setIntegerParam(ADStatus, ADStatusAcquire);

  Acquisition_ResetFrameCnt(hAcqDesc_);
  Acquisition_SetReady(hAcqDesc_, 1);
  setShutter(ADShutterOpen);
  switch (iMode) {
    case PEImageSingle:
      uiPEResult = Acquisition_Acquire_Image(hAcqDesc_, iFrames, numFramesToSkip,
                                             HIS_SEQ_ONE_BUFFER, NULL, NULL, NULL);
      break;
    case PEImageMultiple:
    case PEImageContinuous:
      uiPEResult = Acquisition_Acquire_Image(hAcqDesc_, iFrames, numFramesToSkip,
                                             HIS_SEQ_CONTINUOUS, NULL, NULL, NULL);
      break;
    case PEImageAverage:
      uiPEResult = Acquisition_Acquire_Image(hAcqDesc_, iFrames, numFramesToSkip,
                                             HIS_SEQ_AVERAGE, NULL, NULL, NULL);
      break;
  }

  if (uiPEResult != HIS_ALL_OK) {
    Acquisition_GetErrorCode(hAcqDesc_, &HISError, &FGError);
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error: %d Acquisition_Acquire_Image failed, HIS Error: %d, Frame Grabber Error: %d\n", 
      driverName, functionName, uiPEResult, HISError, FGError);
  }
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Acquisition started...\n",
    driverName, functionName);

}


//_____________________________________________________________________________________________
/** Stops acquisition
  * acquireStop cannot directly call Acquisition_Abort because acquireStop is called from the
  * endFrameCallback function which is running in an XISL thread.  When it calls Acquisition_Abort
  * that appears to result in a deadlock, presumably because there is a second XISL thread that
  * does the abort, and it blocks waiting for the first XISL thread (deadlock). So instead
  * we run a thread whose only job is to call Acquisition_Abort on receipt of an EPICS event. */

void Dexela::acquireStop(void)
{
  static const char *functionName = "acquireStop";

  epicsEventSignal(acquireStopEvent_);
}

static void acquireStopTaskC(void *drvPvt)
{
    Dexela *pPvt = (Dexela *)drvPvt;

    pPvt->acquireStopTask();
}

void Dexela::acquireStopTask(void)
{
  while (1) {
    epicsEventWait(acquireStopEvent_);
    Acquisition_Abort(hAcqDesc_);
    setShutter(ADShutterClosed);
  }
}
    

//_____________________________________________________________________________________________

/** Acquires an offset image */
void Dexela::acquireOffsetImage (void)
{
  int iFrames;
  int status = asynSuccess;
  unsigned int uiPEResult;
  const char   *functionName = "acquireOffsetImage";

  getIntegerParam(DEX_NumOffsetFrames, &iFrames);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Frames: %d, Rows: %d, Columns: %d\n",
    driverName, functionName, iFrames, uiRows_, uiColumns_);

  if (pOffsetBuffer_ != NULL)
    free (pOffsetBuffer_);
  pOffsetBuffer_ = (epicsUInt16 *) malloc(sizeof(epicsUInt16) * uiRows_ * uiColumns_);
  if (pOffsetBuffer_ == NULL)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error:  Memory allocation failed for offset buffer!\n",
      driverName, functionName);
    return;
  }

  memset (pOffsetBuffer_, 0, sizeof(epicsUInt16) * uiRows_ * uiColumns_);

  iAcqMode_ = DEX_ACQUIRE_OFFSET;
  setIntegerParam(DEX_CurrentOffsetFrame, 0);
  setIntegerParam(DEX_OffsetAvailable, NOT_AVAILABLE);
  
  // Make sure the shutter is closed
  setShutter(ADShutterClosed);

  if ((uiPEResult = Acquisition_Acquire_OffsetImage(hAcqDesc_, pOffsetBuffer_, 
                                                    uiRows_, uiColumns_, iFrames)) != HIS_ALL_OK)
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error: %d Acquisition_Acquire_OffsetImage failed!\n", 
      driverName, functionName, uiPEResult);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Offset acquisition started...\n",
    driverName, functionName);

}

//_____________________________________________________________________________________________

/** Acquires a gain image */
void Dexela::acquireGainImage(void)
{
  int iFrames;
  int status = asynSuccess;
  unsigned int uiPEResult;
  const char  *functionName = "acquireGainImage";

  getIntegerParam(DEX_NumGainFrames, &iFrames);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Frames: %d, Rows: %d, Columns: %d\n",
    driverName, functionName, iFrames, uiRows_, uiColumns_);

  if (pGainBuffer_ != NULL)
  free (pGainBuffer_);
  pGainBuffer_ = (DWORD *) malloc(uiRows_ * uiColumns_ * sizeof(DWORD));
  if (pGainBuffer_ == NULL)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error:  Memory allocation failed for gain buffer!\n",
      driverName, functionName);
    return;
  }

  iAcqMode_ = DEX_ACQUIRE_GAIN;
  setIntegerParam(DEX_CurrentGainFrame, 0);
  setIntegerParam(DEX_GainAvailable, NOT_AVAILABLE);
  setShutter(ADShutterOpen);

  if ((uiPEResult = Acquisition_Acquire_GainImage(hAcqDesc_, pOffsetBuffer_, pGainBuffer_, 
                                                  uiRows_, uiColumns_, iFrames)) != HIS_ALL_OK)
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error: %d Acquisition_Acquire_GainImage failed!\n", 
      driverName, functionName, uiPEResult);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Gain acquisition started...\n",
    driverName, functionName);

}

//_____________________________________________________________________________________________

/** Saves a gain file */
asynStatus Dexela::saveGainFile(void)
{
  int iSizeX;
  int iSizeY;
  int iByteDepth;
  int status = asynSuccess;
  char gainPath[256];
  char gainFile[256];
  FILE  *pOutputFile;
  static const char *functionName = "saveGainFile";

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, Saving correction files...\n",
    driverName, functionName);

  status |= getStringParam(DEX_CorrectionsDirectory, sizeof(gainPath), gainPath);
  status |= getStringParam(DEX_GainFile, sizeof(gainFile), gainFile);
  strcat(gainPath, gainFile);
  status |= getIntegerParam(NDArraySizeX, &iSizeX);
  status |= getIntegerParam(NDArraySizeY, &iSizeY);

  if (pGainBuffer_ == NULL) return asynError;

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, saving gain file: %s\n",
    driverName, functionName, gainPath);

  pOutputFile = fopen (gainPath, "wb");

  if (pOutputFile == NULL) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Error opening gain file %s\n", 
      driverName, functionName, gainFile);
    return asynError;
  }
  iByteDepth = sizeof (DWORD);

  fwrite ((void *) &iSizeX, sizeof (int), 1, pOutputFile);
  fwrite ((void *) &iSizeY, sizeof (int), 1, pOutputFile);
  fwrite ((void *) &iByteDepth, sizeof (int), 1, pOutputFile);
  if (ferror (pOutputFile)) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to write file header for file %s\n", 
      driverName, functionName, gainFile);
    fclose (pOutputFile);
    return asynError;
  }

  fwrite (pGainBuffer_, iByteDepth, iSizeX*iSizeY, pOutputFile);
  if (ferror (pOutputFile)) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to write data for file %s\n", 
      driverName, functionName, gainFile);
    fclose (pOutputFile);
    return asynError;
  }

  fclose (pOutputFile);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, Gain file %s saved.\n",
    driverName, functionName, gainFile);
  return asynSuccess;
}


//_____________________________________________________________________________________________

/** Loads a gain file */
asynStatus Dexela::loadGainFile (void)
{
  int status = asynSuccess;
  char gainPath[256];
  char gainFile[256];
  int iSizeX, iSizeY, iByteDepth;
  FILE  *pInputFile;
  struct stat stat_buffer;
  static const char *functionName = "loadGainFile";

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Loading gain file...\n",
    driverName, functionName);

  status |= getStringParam(DEX_CorrectionsDirectory, sizeof(gainPath), gainPath);
  status |= getStringParam(DEX_CorrectionsDirectory, sizeof(gainFile), gainFile);
  strcat(gainPath, gainFile);

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, Gain correction file name: %s\n",
    driverName, functionName, gainPath);
  if ((stat (gainPath, &stat_buffer) != 0)|| (stat_buffer.st_mode & S_IFREG) == 0) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to find gain correction file %s\n", 
      driverName, functionName, gainPath);
    return asynError;
  }
  if (pGainBuffer_ != NULL)
    free (pGainBuffer_);

  pInputFile = fopen (gainPath, "rb");
  if (pInputFile == NULL) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to open gain correction file %s\n", 
      driverName, functionName, gainPath);
    return asynError;
  }
  fread (&iSizeX, sizeof (int), 1, pInputFile);
  fread (&iSizeY, sizeof (int), 1, pInputFile);
  fread (&iByteDepth, sizeof (int), 1, pInputFile);
  if (ferror (pInputFile)) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to read file header for gain correction file %s\n", 
      driverName, functionName, gainPath);
    return asynError;
  }
  pGainBuffer_ = (DWORD *) malloc (iSizeX * iSizeY * iByteDepth);
  fread (pGainBuffer_, iByteDepth, iSizeX * iSizeY, pInputFile);
  if (ferror (pInputFile)) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to read data for gain correction file %s\n", 
      driverName, functionName, gainPath);
    return asynError;
  }

  fclose (pInputFile);

  status |= setIntegerParam(DEX_GainAvailable, AVAILABLE);
  callParamCallbacks();

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, Gain file %s loaded\n",
    driverName, functionName, gainPath);
  return asynSuccess;

}
//_____________________________________________________________________________________________

/** Loads a pixel correction file */
asynStatus Dexela::loadPixelCorrectionFile()
{
  FILE                *pInputFile;
  WinHeaderType       file_header;
  WinImageHeaderType  image_header;
  int                 iBufferSize;
  int                 iCorrectionMapSize;
  unsigned int        uiStatus;
  char                pixelCorrectionFile[256];
  char                pixelCorrectionPath[256];
  struct              stat stat_buffer;
  static const char   *functionName = "readPixelCorrectionFile";
  
  setIntegerParam(DEX_PixelCorrectionAvailable, NOT_AVAILABLE);

  getStringParam(DEX_PixelCorrectionFile, sizeof(pixelCorrectionFile), pixelCorrectionFile);
  getStringParam(DEX_CorrectionsDirectory, sizeof(pixelCorrectionPath), pixelCorrectionPath);
  strcat(pixelCorrectionPath, pixelCorrectionFile);
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s:, Pixel correction file name: %s\n",
    driverName, functionName, pixelCorrectionPath);
  if ((stat (pixelCorrectionPath, &stat_buffer) != 0) || (stat_buffer.st_mode & S_IFREG) == 0) {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: error opening pixel correction file %s\n",
      driverName, functionName, pixelCorrectionPath);
    return asynError;
  }
  
  pInputFile = fopen(pixelCorrectionPath, "r");

  if (pInputFile == NULL)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
    "%s:%s: Failed to open file %s\n", 
    driverName, functionName, pixelCorrectionPath);
    return asynError;
  }

  //read file header
  fread((void *) &file_header, 68, 1, pInputFile);
  if (ferror(pInputFile))
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to read file header from file %s\n", 
      driverName, functionName, pixelCorrectionPath);
    return asynError;
  }


  //read image header
  fread((void *) &image_header, 32, 1, pInputFile);
  if (ferror (pInputFile))
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to read image header from file %s\n", 
      driverName, functionName, pixelCorrectionPath);
    return asynError;
  }

  //read bad pixel map
  if (pBadPixelMap_ != NULL)
    free (pBadPixelMap_);
  iBufferSize = file_header.ULY * file_header.BRX * sizeof (epicsUInt16);
  pBadPixelMap_ = (epicsUInt16 *) malloc (iBufferSize);
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: buffer size: %d, pBadPixelMap: %d\n", 
    driverName, functionName, iBufferSize, pBadPixelMap_);
  if (pBadPixelMap_ == NULL)
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to allocate bad pixel map buffer\n", 
      driverName, functionName);
    return asynError;
  }
  fread ((void *) pBadPixelMap_, iBufferSize, 1, pInputFile);
  if (ferror (pInputFile))
  {
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s:%s: Failed to read bad pixel map from file %s\n", 
      driverName, functionName, pixelCorrectionPath);
    free (pBadPixelMap_);
    pBadPixelMap_ = NULL;
    return asynError;
  }

  fclose (pInputFile);

  int counter = 0;
  for (int loop=0;loop<file_header.ULY * file_header.BRX;loop++)
  {
    if (pBadPixelMap_[loop] == 65535)
      counter++;
  }
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: Bad pixel map read in: %d bad pixels found\n", 
    driverName, functionName, counter);

  //first call with correction list = NULL returns size of buffer to allocate
  //second time gets the correction list
  uiStatus = Acquisition_CreatePixelMap (pBadPixelMap_, file_header.ULY, file_header.BRX, NULL, &iCorrectionMapSize);
  pPixelCorrectionList_ = (int *) malloc (iCorrectionMapSize);
  uiStatus = Acquisition_CreatePixelMap (pBadPixelMap_, file_header.ULY, file_header.BRX, pPixelCorrectionList_, &iCorrectionMapSize);

  free (pBadPixelMap_);
  pBadPixelMap_ = NULL;

  setIntegerParam(DEX_PixelCorrectionAvailable, AVAILABLE);
  return asynSuccess;
}

//-------------------------------------------------------------
/** Sets the trigger mode */
asynStatus Dexela::setTriggerMode() {
  int error;
  int mode;

  mode = iTrigModeReq_;

  switch (mode) {
   case DEX_FREE_RUNNING: {
      error = Acquisition_SetFrameSyncMode(hAcqDesc_, HIS_SYNCMODE_FREE_RUNNING);
      break;
   }
   case DEX_EXTERNAL_TRIGGER: {
      error = Acquisition_SetFrameSyncMode(hAcqDesc_, HIS_SYNCMODE_EXTERNAL_TRIGGER);
      break;
   }
   case DEX_INTERNAL_TRIGGER: {
      error = Acquisition_SetFrameSyncMode(hAcqDesc_, HIS_SYNCMODE_INTERNAL_TIMER);
      break;
   }
   case DEX_SOFT_TRIGGER: {
      error = Acquisition_SetFrameSyncMode(hAcqDesc_, HIS_SYNCMODE_SOFT_TRIGGER);
      break;
   }
  }
  iTrigModeAct_ = mode;
  setIntegerParam(ADTriggerMode, iTrigModeAct_);
  callParamCallbacks();

  return asynSuccess;

}

//-------------------------------------------------------------
/** Sets the exposure time */
asynStatus Dexela::setExposureTime() {
  DWORD dwDwellTime;
  int status = asynSuccess;
  int error;
  const char *functionName = "setExposureTime";

  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, 
    "%s:%s: Setting AcquireTime %f\n",
     driverName, functionName, dAcqTimeReq_);
  dwDwellTime = (DWORD) (dAcqTimeReq_ * 1000000);
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: internal timer requested: %d\n", 
    driverName, functionName, dwDwellTime);
  error = Acquisition_SetTimerSync(hAcqDesc_, &dwDwellTime);
  dAcqTimeAct_ = dwDwellTime/1000000.;
  asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
    "%s:%s: internal timer: %f\n", 
    driverName, functionName, dAcqTimeAct_);
  status |= setDoubleParam(ADAcquireTime, dAcqTimeAct_);
  callParamCallbacks();

  return asynSuccess;
}

/* Code for iocsh registration */

/* DexelaConfig */
static const iocshArg DexelaConfigArg0 = {"Port name", iocshArgString};
static const iocshArg DexelaConfigArg1 = {"detIndex", iocshArgInt};
static const iocshArg DexelaConfigArg2 = {"numFrameBuffers", iocshArgInt};
static const iocshArg DexelaConfigArg3 = {"maxBuffers", iocshArgInt};
static const iocshArg DexelaConfigArg4 = {"maxMemory", iocshArgInt};
static const iocshArg DexelaConfigArg5 = {"priority", iocshArgInt};
static const iocshArg DexelaConfigArg6 = {"stackSize", iocshArgInt};
static const iocshArg * const DexelaConfigArgs[] =  {&DexelaConfigArg0,
                                                          &DexelaConfigArg1,
                                                          &DexelaConfigArg2,
                                                          &DexelaConfigArg3,
                                                          &DexelaConfigArg4,
                                                          &DexelaConfigArg5,
                                                          &DexelaConfigArg6};
static const iocshFuncDef configDexela = {"DexelaConfig", 7, DexelaConfigArgs};
static void configDexelaCallFunc(const iocshArgBuf *args)
{
  DexelaConfig(args[0].sval, args[1].ival, args[2].ival, args[3].ival, 
                    args[4].ival, args[5].ival, args[6].ival);
}


static void DexelaRegister(void)
{
  iocshRegister(&configDexela, configDexelaCallFunc);
}

extern "C" {
epicsExportRegistrar(DexelaRegister);
}
