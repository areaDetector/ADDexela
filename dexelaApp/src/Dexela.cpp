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
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsExit.h>
#include <epicsString.h>
#include <epicsStdio.h>
#include <iocsh.h>

#include "ADDriver.h"

using namespace std;

#include "BusScanner.h"
#include "DexelaDetector.h"

#include <epicsExport.h>

#include "Dexela.h"


static const char *driverName = "Dexela";

typedef struct {
  int value;
  const char* string;
} enumStruct_t;

#define MAX_BINNING 10
static enumStruct_t binEnums[MAX_BINNING]= {
  {x11,  "Unbinned"},
  {x12,  "1H x 2V"},
  {x14,  "1H x 4V"},
  {x21,  "2H x 1V"},
  {x22,  "2H x 2V"},
  {x24,  "2H x 4V"},
  {x41,  "4H x 1V"},
  {x42,  "4H x 2V"},
  {x44,  "4H x 4V"},
  {ix22, "2H x 2V digital"}
};

typedef enum {
  DEXInternalFreeRun,
  DEXInternalFixedRate,
  DEXInternalSoftware,
  DEXExternalEdgeSingle,
  DEXExternalEdgeMulti,
  DEXExternalBulb
} DEXTriggerMode_t;

#define MAX_TRIGGERS 6
static enumStruct_t triggerEnums[MAX_TRIGGERS]= {
  {DEXInternalFreeRun,    "Int. Free Run"},
  {DEXInternalFixedRate,  "Int. Fixed Rate"},
  {DEXInternalSoftware,   "Int. Software"},
  {DEXExternalEdgeSingle, "Ext. Edge Single"},
  {DEXExternalEdgeMulti,  "Ext. Edge Multi"},
  {DEXExternalBulb,       "Ext. Bulb"}
};

#define MAX_FULL_WELL 2
static enumStruct_t fullWellEnums[MAX_FULL_WELL]= {
  {Low,  "Low noise"},
  {High, "High range"}
};

// Forward function definitions
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
extern "C" int DexelaConfig(const char *portName, int detIndex,
                                 int maxBuffers, size_t maxMemory, int priority, int stackSize)
{
    new Dexela(portName, detIndex, maxBuffers, maxMemory, priority, stackSize);
    return(asynSuccess);
}

//_____________________________________________________________________________________________

// Callback function that is called by Dexela SDK for each frame
static void newFrameCallback(int frameCounter, int bufferNumber, DexelaDetector *pDet)
{
  Dexela *pDexela = (Dexela *)pDet->GetCallbackData();
  pDexela->newFrameCallback(frameCounter, bufferNumber);
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

Dexela::Dexela(const char *portName,  int detIndex, 
                         int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : ADDriver(portName, 1, (int)NUM_DEXELA_PARAMS, maxBuffers, maxMemory, 
               asynEnumMask, asynEnumMask, ASYN_CANBLOCK, 1, priority, stackSize)
{
  int status = asynSuccess;
  static const char *functionName = "Dexela";
  
  int numDevices;
  
  /* Add parameters for this driver */
  createParam(DEX_SerialNumberString,                asynParamInt32,   &DEX_SerialNumber);
  createParam(DEX_BinningModeString,                 asynParamInt32,   &DEX_BinningMode);
  createParam(DEX_FullWellModeString,                asynParamInt32,   &DEX_FullWellMode);
  createParam(DEX_AcquireOffsetString,               asynParamInt32,   &DEX_AcquireOffset);
  createParam(DEX_NumOffsetFramesString,             asynParamInt32,   &DEX_NumOffsetFrames);
  createParam(DEX_CurrentOffsetFrameString,          asynParamInt32,   &DEX_CurrentOffsetFrame);
  createParam(DEX_UseOffsetString,                   asynParamInt32,   &DEX_UseOffset);
  createParam(DEX_OffsetAvailableString,             asynParamInt32,   &DEX_OffsetAvailable);
  createParam(DEX_OffsetFileString,                  asynParamOctet,   &DEX_OffsetFile);
  createParam(DEX_LoadOffsetFileString,              asynParamInt32,   &DEX_LoadOffsetFile);
  createParam(DEX_SaveOffsetFileString,              asynParamInt32,   &DEX_SaveOffsetFile);
  createParam(DEX_OffsetConstantString,              asynParamInt32,   &DEX_OffsetConstant);
  createParam(DEX_AcquireGainString,                 asynParamInt32,   &DEX_AcquireGain);
  createParam(DEX_NumGainFramesString,               asynParamInt32,   &DEX_NumGainFrames);
  createParam(DEX_CurrentGainFrameString,            asynParamInt32,   &DEX_CurrentGainFrame);
  createParam(DEX_UseGainString,                     asynParamInt32,   &DEX_UseGain);
  createParam(DEX_GainAvailableString,               asynParamInt32,   &DEX_GainAvailable);
  createParam(DEX_GainFileString,                    asynParamOctet,   &DEX_GainFile);
  createParam(DEX_LoadGainFileString,                asynParamInt32,   &DEX_LoadGainFile);
  createParam(DEX_SaveGainFileString,                asynParamInt32,   &DEX_SaveGainFile);
  createParam(DEX_UseDefectMapString,                asynParamInt32,   &DEX_UseDefectMap);
  createParam(DEX_DefectMapAvailableString,          asynParamInt32,   &DEX_DefectMapAvailable);
  createParam(DEX_DefectMapFileString,               asynParamOctet,   &DEX_DefectMapFile);
  createParam(DEX_LoadDefectMapFileString,           asynParamInt32,   &DEX_LoadDefectMapFile);
  createParam(DEX_SoftwareTriggerString,             asynParamInt32,   &DEX_SoftwareTrigger);
  createParam(DEX_CorrectionsDirectoryString,        asynParamOctet,   &DEX_CorrectionsDirectory);

  /* Set some default values for parameters */
  setIntegerParam(NDArraySize, 0);
  setIntegerParam(NDDataType, NDUInt16);
  setIntegerParam(DEX_AcquireOffset, 0);
  setIntegerParam(DEX_OffsetAvailable, 0);
  setIntegerParam(DEX_AcquireGain, 0);
  setIntegerParam(DEX_GainAvailable, 0);
  setIntegerParam(DEX_DefectMapAvailable, 0);
  setStringParam (DEX_CorrectionsDirectory, "");
  setStringParam (DEX_GainFile, "");
  setStringParam (DEX_DefectMapFile, "");

  try {
    pBusScanner_ = new BusScanner();
    numDevices = pBusScanner_->EnumerateDevices();
    if (numDevices <= 0) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: no Dexela devices found\n",
        driverName, functionName);
      return;
    }

    if (detIndex > numDevices-1) {
      asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
        "%s::%s Error: detector index %d not available, only %d devices found\n",
        driverName, functionName, detIndex, numDevices);
      return;
    }
    
    devInfo_ = pBusScanner_->GetDevice(detIndex);
    pDetector_ = new DexelaDetector(devInfo_);
    // Connect to board
    pDetector_->OpenBoard();
    sensorX_      = pDetector_->GetSensorWidth();
    sensorY_      = pDetector_->GetSensorHeight();
    modelNumber_  = pDetector_->GetModelNumber();
    binningMode_  = pDetector_->GetBinningMode();
    serialNumber_ = pDetector_->GetSerialNumber();
    numBuffers_   = pDetector_->GetNumBuffers();
    setIntegerParam(ADMaxSizeX, sensorX_);
    setIntegerParam(ADMaxSizeY, sensorY_);
    setStringParam(ADManufacturer, "Perkin Elmer");
    sprintf(modelName_, "Dexela %d", modelNumber_);
    setStringParam(ADModel, modelName_);
    setIntegerParam(DEX_SerialNumber, serialNumber_);
    snapBuffer_ = 0;

    // Set callback
    pDetector_->SetCallback(::newFrameCallback);
    pDetector_->SetCallbackData(this);

    // Enable pulse generator
    pDetector_->EnablePulseGenerator();

    // Turn off pulses
    pDetector_->ToggleGenerator(false);

  } catch (DexelaException &e) {
    reportError(functionName, e);
    return;
  }
 
  // Set exit handler to clean up
  epicsAtExit(exitCallbackC, this);
}

void Dexela::reportError(const char *functionName, DexelaException &e)
{
    asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
      "%s::%s exception description=%s\n, function=%s, transport message=%s\n",
      driverName, functionName, e.what(), e.GetFunctionName(), e.GetTransportMessage());
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
  static const char *functionName = "~Dexela";
  
  asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
     "%s::%s calling DexelaDetector::CloseBoard()\n",driverName, functionName);
  pDetector_->CloseBoard();
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
  int dataType;
  static const char *functionName = "report";

  try {
    fprintf(fp, "Dexela, port=%s, model=%s, serial number=%d\n", 
      this->portName, modelName_, serialNumber_);
    if (details > 0) {
      getIntegerParam(NDDataType, &dataType);
      fprintf(fp, "  Number of rows:    %d\n", sensorX_);
      fprintf(fp, "  Number of columns: %d\n", sensorY_);
      fprintf(fp, "  Data type:         %d\n", dataType);
      fprintf(fp, "  Frames allocated:  %d\n", pDetector_->GetNumBuffers());
    }
    if (details > 1) reportSensors(fp, details);

    /* Invoke the base class method */
    ADDriver::report(fp, details);
  } catch (DexelaException &e) {
    reportError(functionName, e);
 }
}

//_____________________________________________________________________________________________
/** Report information about all local and network Dexela detectors */
void Dexela::reportSensors(FILE *fp, int details)
{
  int numDevices;
  DevInfo devInfo;
  static const char *functionName = "reportSensors";
  int i;
  
  try {
    numDevices = pBusScanner_->EnumerateDevices();
    fprintf(fp, "Total sensors in system: %d\n", numDevices);
    // Iterate through all devices and display information
    for (i=0; i<numDevices; i++) {
      fprintf(fp, "Device: %d\n", i);
      devInfo = pBusScanner_->GetDevice(i);
      fprintf(fp, "   Model number: %d\n", devInfo.model);
      fprintf(fp, "  Serial number: %d\n", devInfo.serialNum);
      fprintf(fp, "      Interface: %s\n", devInfo.iface ? "GigE" : "CameraLink");
    }
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
}

//_____________________________________________________________________________________________
// Callback function that is called by from ::newFrameCallback for each frame
void Dexela::newFrameCallback(int frameCounter, int bufferNumber)
{
  void          *pData;
  NDArrayInfo   arrayInfo;
  int           arrayCounter;
  int           imageCounter;
  int           arrayCallbacks;
  int           numImages;
  int           imageMode;
  int           numOffsetFrames;
  int           offsetCounter;
  int           offsetAvailable;
  int           useOffset;
  int           numGainFrames;
  int           gainCounter;
  int           gainAvailable;
  int           useGain;
  int           useDefectMap;
  int           frameType;
  int           acquiring;
  int           darkOffset;
  size_t        dims[2];
  NDArray       *pImage;
  NDDataType_t  dataType = NDUInt16;
  epicsTimeStamp currentTime;
  DexImage      dataImage;
  static const char *functionName = "newFrameCallback";
    
  asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
    "%s:%s: entry ...\n",
    driverName, functionName);

  lock();
  
  try {
    getIntegerParam(ADFrameType,         &frameType);
    getIntegerParam(DEX_OffsetAvailable, &offsetAvailable);
    getIntegerParam(DEX_UseOffset,       &useOffset);
    getIntegerParam(DEX_GainAvailable,   &gainAvailable);
    getIntegerParam(DEX_UseGain,         &useGain);
    getIntegerParam(DEX_UseDefectMap,    &useDefectMap);
    getIntegerParam(ADAcquire,           &acquiring);
    // At high rates we can be called for a few extra frames after acquisition is done
    if (!acquiring) goto done;

    switch (frameType) {
      case ADFrameBackground:
        getIntegerParam(DEX_NumOffsetFrames,    &numOffsetFrames);
        getIntegerParam(DEX_CurrentOffsetFrame, &offsetCounter);

        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
          "%s::%s calling DexelaDetector::ReadBuffer(%d, %p, %d)\n",
          driverName, functionName, bufferNumber, offsetImage_, offsetCounter);
        pDetector_->ReadBuffer(bufferNumber, offsetImage_, offsetCounter);

        pData = offsetImage_.GetDataPointerToPlane(offsetCounter);
        offsetCounter++;
        setIntegerParam(DEX_CurrentOffsetFrame, offsetCounter);
        // If this is the last offset image then compute the median image and raise a flag to the 
        // user that offset data is available
        if (offsetCounter == numOffsetFrames) {
          offsetImage_.FindMedianofPlanes();
          offsetImage_.UnscrambleImage();
          offsetImage_.SetImageType(Offset);
          setIntegerParam(DEX_OffsetAvailable, 1);
          pData = offsetImage_.GetDataPointerToPlane();
          setIntegerParam(DEX_AcquireOffset, 0);
          setIntegerParam(ADAcquire, 0);
          acquireStop();
        }
        break;

      case ADFrameFlatField:
        getIntegerParam(DEX_NumGainFrames,    &numGainFrames);
        getIntegerParam(DEX_CurrentGainFrame, &gainCounter);

        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
          "%s::%s calling DexelaDetector::ReadBuffer(%d, %p, %d)\n",
          driverName, functionName, bufferNumber, gainImage_, gainCounter);
        pDetector_->ReadBuffer(bufferNumber, gainImage_, gainCounter);

        pData = gainImage_.GetDataPointerToPlane(gainCounter);
        gainCounter++;
        setIntegerParam(DEX_CurrentGainFrame, gainCounter);
        // If this is the last offset image then compute the flood image and raise a flag to the 
        // user that offset data is available
        if (gainCounter >= numGainFrames) {
          gainImage_.FindMedianofPlanes();
          gainImage_.UnscrambleImage();
          gainImage_.FixFlood();
          gainImage_.SetImageType(Gain);
          setIntegerParam(DEX_GainAvailable, 1);
          dataType = NDFloat32;
          pData = gainImage_.GetDataPointerToPlane();
          setIntegerParam(DEX_AcquireGain, 0);
          setIntegerParam(ADAcquire, 0);
          acquireStop();
        }
        break;

      case ADFrameNormal:
        getIntegerParam(ADImageMode, &imageMode);
        getIntegerParam(ADNumImages, &numImages);
        getIntegerParam(ADNumImagesCounter, &imageCounter);
        // In SDK prior to 1.0.0.5 the following line was needed or performance suffered
        // because it needed to read values from detector registers
//      dataImage.SetImageParameters(binningMode_, modelNumber_);

        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
          "%s::%s calling DexelaDetector::ReadBuffer(%d, %p)\n",
          driverName, functionName, bufferNumber, dataImage);
        pDetector_->ReadBuffer(bufferNumber, dataImage);

        if ((imageMode == ADImageSingle) ||
            ((imageMode == ADImageMultiple) && 
             (imageCounter >= numImages-1))) {
          acquireStop();
          setIntegerParam(ADAcquire, 0);
        }
        imageCounter++;
        setIntegerParam(ADNumImagesCounter, imageCounter);
        getIntegerParam(NDArrayCounter, &arrayCounter);
        arrayCounter++;
        setIntegerParam(NDArrayCounter, arrayCounter);
        dataImage.UnscrambleImage();
        dataImage.SetImageType(Data);

        /** Correct for detector offset and gain as necessary */
        if (offsetAvailable && useOffset && !offsetImage_.IsEmpty()) {
          getIntegerParam(DEX_OffsetConstant, &darkOffset);
          dataImage.SetDarkOffset(darkOffset);
          dataImage.LoadDarkImage(offsetImage_);
          if (gainAvailable && useGain && !gainImage_.IsEmpty()) {
            dataImage.LoadFloodImage(gainImage_);
            dataImage.FloodCorrection();
          } else {
            dataImage.SubtractDark();
          }
        }

        /** Correct for dead pixels as necessary */
        pData = dataImage.GetDataPointerToPlane();
        break;
    }

    getIntegerParam(NDArrayCallbacks, &arrayCallbacks);
    if (arrayCallbacks) {
      /* Update the image */
      /* We save the most recent image buffer so it can be used in the read() function.
       * Now release it before getting a new version. */
      if (this->pArrays[0])
          this->pArrays[0]->release();

      /* Allocate the array */
      dims[0] = pDetector_->GetBufferXdim();
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s called DexelaDetector::GetBufferYdim() returned dims[0]=%d\n",
        driverName, functionName, dims[0]);

      dims[1] = pDetector_->GetBufferYdim();
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s called DexelaDetector::GetBufferYdim() returned dims[1]=%d\n",
        driverName, functionName, dims[0]);

      this->pArrays[0] = pNDArrayPool->alloc(2, dims, dataType, 0, NULL);
      if (this->pArrays[0] == NULL) {
        asynPrint(pasynUserSelf, ASYN_TRACE_ERROR,
          "%s:%s: error allocating buffer\n",
          driverName, functionName);
        goto done;
      }
      pImage = this->pArrays[0];
      pImage->getInfo(&arrayInfo);
      // Copy the data from the input to the output
      memcpy(pImage->pData, pData, arrayInfo.totalBytes);

      setIntegerParam(NDArraySize,  (int)arrayInfo.totalBytes);
      setIntegerParam(NDArraySizeX, (int)pImage->dims[0].size);
      setIntegerParam(NDArraySizeY, (int)pImage->dims[1].size);

      /* Put the frame number and time stamp into the buffer */
      pImage->uniqueId = frameCounter;
      epicsTimeGetCurrent(&currentTime);
      pImage->timeStamp = currentTime.secPastEpoch + currentTime.nsec / 1.e9;
      updateTimeStamp(&pImage->epicsTS);

      /* Get any attributes that have been defined for this driver */
      getAttributes(pImage->pAttributeList);

      /* Call the NDArray callback */
      asynPrint(pasynUserSelf, ASYN_TRACE_FLOW,
        "%s:%s: calling imageData callback\n", 
        driverName, functionName);
      doCallbacksGenericPointer(pImage, NDArrayData, 0);
    }

    done:

    // Do callbacks on parameters
    callParamCallbacks();

  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  unlock();
  asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
    "%s:%s: exit\n",
    driverName, functionName);
}


//_____________________________________________________________________________________________
/** Called when asyn clients call pasynInt32->write().
  * This function performs actions for some parameters, including ADAcquire, DEX_AcquireOffset, etc.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus Dexela::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  int acquiring;
  int status = asynSuccess;
  static const char *functionName = "writeInt32";

  getIntegerParam(ADAcquire, &acquiring);

  try {
    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setIntegerParam(function, value);

    if (function == ADAcquire) {
      // Start acquisition
      if (value && !acquiring) {
        acquireStart();
      }
      // Stop acquisition
      if (!value && acquiring) {
        acquireStop();
      }
    }
    else if (function == DEX_AcquireOffset) {
      if (!acquiring) {
        acquireOffsetImage();
      }
    }
    else if (function ==  DEX_AcquireGain) {
      if (!acquiring) {
        acquireGainImage();
      }
    }
    else if (function == DEX_BinningMode) {
      binningMode_ = (bins)value;
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s calling DexelaDetector::SetBinningMode(%d)\n",
        driverName, functionName, binningMode_);
      pDetector_->SetBinningMode(binningMode_);
    }
    else if (function == DEX_FullWellMode) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s calling DexelaDetector::SetFullWellMode(%d)\n",
        driverName, functionName, (FullWellModes)value);
      pDetector_->SetFullWellMode((FullWellModes)value);
    }
    else if (function == DEX_SoftwareTrigger) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s calling DexelaDetector::SoftwareTrigger()\n",
        driverName, functionName);
      pDetector_->SoftwareTrigger();
    }
    else if (function == DEX_LoadOffsetFile) {
      loadOffsetFile();
    }
    else if (function == DEX_SaveOffsetFile) {
      saveOffsetFile();
    }
    else if (function == DEX_LoadGainFile) {
      loadGainFile();
    }
    else if (function == DEX_SaveGainFile) {
      saveGainFile();
    }
    else if (function == DEX_LoadDefectMapFile) {
      loadDefectMapFile();
    }

    else {
      /* If this parameter belongs to a base class call its method */
      if (function < DEX_FIRST_PARAM) {
        status = ADDriver::writeInt32(pasynUser, value);
      }
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }

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
  static const char *functionName = "writeFloat64";

  try {
    /* Set the parameter and readback in the parameter library.  This may be overwritten but that's OK */
    status = setDoubleParam(function, value);

    if (function == ADAcquireTime) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
        "%s::%s calling DexelaDetector::SetExposureTime(%f)\n",
        driverName, functionName, value*1000.);
      pDetector_->SetExposureTime((float)(value * 1000.));
    }
    else {
      /* If this parameter belongs to a base class call its method */
      if (function < DEX_FIRST_PARAM) {
        status = ADDriver::writeFloat64(pasynUser, value);
      }
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks();
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }

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
/** Called when asyn clients call pasynEnum->read().
  * Sets the enum values and strings for DEX_BinningMode, DEX_FullWellMode, and ADTriggerMode
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] strings Array of string pointers. 
  * \param[in] values Array of values 
  * \param[in] severities Array of severities 
  * \param[in] nElements Size of value array 
  * \param[out] nIn Number of elements actually returned */
asynStatus Dexela::readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[], 
                            size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  int i, j=0;
  int exists;

  if (function == DEX_BinningMode) {
    for (i=0; ((i<MAX_BINNING) && (i<(int)nElements)); i++) {
      exists = pDetector_->QueryBinningMode((bins)binEnums[i].value);
      if (exists == 1) {
        if (strings[j]) free(strings[j]);
        strings[j] = epicsStrDup(binEnums[i].string);
        values[j] = binEnums[i].value;
        severities[j] = 0;
        j++;
      }
    }
  }
  else if (function == DEX_FullWellMode) {
    for (i=0; ((i<MAX_FULL_WELL) && (i<(int)nElements)); i++) {
      exists = pDetector_->QueryFullWellMode((FullWellModes)fullWellEnums[i].value);
      if (exists == 1) {
        if (strings[j]) free(strings[j]);
        strings[j] = epicsStrDup(fullWellEnums[i].string);
        values[j] = fullWellEnums[i].value;
        severities[j] = 0;
        j++;
      }
    }
  }
  else if (function == ADTriggerMode) {
    for (i=0; ((i<MAX_TRIGGERS) && (i<(int)nElements)); i++) {
      // We assume the 3 detector trigger modes all are supported
      if (strings[j]) free(strings[j]);
      strings[j] = epicsStrDup(triggerEnums[i].string);
      values[j] = triggerEnums[i].value;
      severities[j] = 0;
      j++;
    }
  }
  else {
    *nIn = 0;
    return asynError;
  }
  *nIn = j;
  return asynSuccess;   
}


//_____________________________________________________________________________________________

/** Starts acquisition */
void Dexela::acquireStart(void)
{
  int imageMode;
  int numImages;
  int imagesPerTrigger;
  int triggerMode;
  double acquireTime;
  double acquirePeriod;
  double gapTime;
  ExposureTriggerSource triggerSource;
  ExposureModes exposureMode;
  double pulseFrequency;
  int status = asynSuccess;
  static const char *triggerSourceStrings[] = {"Ext_neg_edge_trig",
                                               "Internal_Software",
                                               "Ext_Duration_Trig"};
  static const char *exposureModeStrings[]  = {"Expose_and_read",
                                               "Sequence_Exposure",
                                               "Frame_Rate_exposure",
                                               "Preprogrammed_exposure"};
  static const char *functionName = "acquireStart";

  // Do callbacks so Acquire_RBV goes to 1 so user sees acquisition has started
  callParamCallbacks();
  
  try {
    getIntegerParam(ADImageMode,     &imageMode);
    getIntegerParam(ADNumImages,     &numImages);
    getIntegerParam(ADTriggerMode,   &triggerMode);
    getDoubleParam (ADAcquireTime,   &acquireTime);
    getDoubleParam (ADAcquirePeriod, &acquirePeriod);

    setIntegerParam(ADFrameType, ADFrameNormal);
    setIntegerParam(ADNumImagesCounter, 0);
    setIntegerParam(ADStatus, ADStatusAcquire);

    // Set the defaults which may be overridden below
    triggerSource = Internal_Software;
    exposureMode = Sequence_Exposure;
    pulseFrequency = -1;
    imagesPerTrigger = 1;

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
       "%s::%s calling DexelaDetector::ToggleGenerator(false)\n",
       driverName, functionName);
    pDetector_->ToggleGenerator(false);
    
    switch (triggerMode) {

      case DEXInternalFreeRun:
        pulseFrequency = 0;
        break;
      
      case DEXInternalFixedRate:
        pulseFrequency = 1./acquirePeriod;
        break;
        
      case DEXInternalSoftware:
        break;
        
      case DEXExternalEdgeSingle:
        triggerSource = Ext_neg_edge_trig;
        break;
        
      case DEXExternalEdgeMulti:
        triggerSource = Ext_neg_edge_trig;
        imagesPerTrigger = numImages;
        gapTime = (acquirePeriod - acquireTime)*1000.;

        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
           "%s::%s calling DexelaDetector::SetGapTime(%f)\n",
           driverName, functionName, gapTime);
        pDetector_->SetGapTime((float)gapTime);

        exposureMode = Frame_Rate_exposure;
        break;
        
      case DEXExternalBulb:
        triggerSource = Ext_Duration_Trig;
        break;
        
    }
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::SetTriggerSource(%s)\n",
      driverName, functionName, triggerSourceStrings[triggerSource]);
    pDetector_->SetTriggerSource(triggerSource);

    if (imageMode == ADImageSingle) exposureMode = Expose_and_read;
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
       "%s::%s calling DexelaDetector::SetExposureMode(%s)\n",
       driverName, functionName, exposureModeStrings[exposureMode]);
    pDetector_->SetExposureMode(exposureMode);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
       "%s::%s calling DexelaDetector::SetNumOfExposures(%d)\n",
       driverName, functionName, imagesPerTrigger);
    pDetector_->SetNumOfExposures(imagesPerTrigger);

    if (pulseFrequency < 0) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
         "%s::%s calling DexelaDetector::DisablePulseGenerator()\n",
         driverName, functionName);
      pDetector_->DisablePulseGenerator();
    } else if (pulseFrequency == 0) {    
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
         "%s::%s calling DexelaDetector::EnablePulseGenerator()\n",
         driverName, functionName);
      pDetector_->EnablePulseGenerator();
    } else {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
         "%s::%s calling DexelaDetector::EnablePulseGenerator(%f)\n",
         driverName, functionName, pulseFrequency);
      pDetector_->EnablePulseGenerator((float)pulseFrequency);
    }

    setShutter(ADShutterOpen);

    /** Acquire Modes
      * Single: use Snap(), internal software trigger (disable Pulse Gen)
      * Multiple: use Live View, internal software trigger (disable Pulse Gen)
      * Continuous: use Live View, ExpTime PV for exposure, Pulse Gen for image rate */
    switch (imageMode) {

      case ADImageSingle:
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
           "%s::%s calling DexelaDetector::Snap(%d, %d)\n",
           driverName, functionName, snapBuffer_, (int)((acquireTime + 1)*1000.));
        // Must release the lock because Snap is a blocking function call, and the newFrameCallback
        // needs to take the lock
        unlock();
        pDetector_->Snap(snapBuffer_, (int)((acquireTime + 1)*1000.));
        lock();
        snapBuffer_++;
        if (snapBuffer_ >= numBuffers_) snapBuffer_ = 0;
        break;

      case ADImageMultiple:
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
           "%s::%s calling DexelaDetector::GoLiveSeq(%d, %d, %d)\n",
           driverName, functionName, 0, numBuffers_, numImages);
        pDetector_->GoLiveSeq(0, numBuffers_-1, numImages);
        break;

      case ADImageContinuous:
        asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
           "%s::%s calling DexelaDetector::GoLiveSeq(%d, %d, %d)\n",
           driverName, functionName, 0, numBuffers_, 0);
        pDetector_->GoLiveSeq(0, numBuffers_-1, 0);
        break;
    }
    if (pulseFrequency >= 0) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
         "%s::%s calling DexelaDetector::ToggleGenerator(true)\n",
         driverName, functionName);
      pDetector_->ToggleGenerator(true);
    }

  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
}

//_____________________________________________________________________________________________

/** Stop acquisition */
void Dexela::acquireStop(void)
{
  static const char *functionName = "acquireStop";
  
  try {
    setShutter(ADShutterClosed);
    setIntegerParam(ADStatus, ADStatusIdle);
    if (pDetector_->IsLive()) {
      asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
         "%s::%s calling DexelaDetector::GoUnLive()\n",
         driverName, functionName);
      pDetector_->GoUnLive();
    }
    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
       "%s::%s calling DexelaDetector::ToggleGenerator(false)\n",
       driverName, functionName);
    pDetector_->ToggleGenerator(false);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
}

//_____________________________________________________________________________________________

/** Acquires an offset image */
void Dexela::acquireOffsetImage(void)
{
  int numFrames;
  static const char *functionName = "acquireOffsetImage";

  try {
    getIntegerParam(DEX_NumOffsetFrames, &numFrames);

    setIntegerParam(ADFrameType, ADFrameBackground);
    setIntegerParam(DEX_CurrentOffsetFrame, 0);
    setIntegerParam(DEX_OffsetAvailable, 0);
    setIntegerParam(ADAcquire, 1);
    
    offsetImage_ = DexImage();

    // Make sure the shutter is closed
    setShutter(ADShutterClosed);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::SetTriggerSource(Internal_Software)\n",
      driverName, functionName);
    pDetector_->SetTriggerSource(Internal_Software);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::SetExposureMode(Sequence_Exposure)\n",
      driverName, functionName);
    pDetector_->SetExposureMode(Sequence_Exposure);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::ToggleGenerator(true)\n",
      driverName, functionName);
    pDetector_->ToggleGenerator(true);      

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::GoLiveSeq()\n",
      driverName, functionName);
    pDetector_->GoLiveSeq();

  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
}

//_____________________________________________________________________________________________

/** Acquires a gain image */
void Dexela::acquireGainImage(void)
{
  int numFrames;
  static const char *functionName = "acquireOffsetImage";

  try {
    getIntegerParam(DEX_NumGainFrames, &numFrames);

    setIntegerParam(ADFrameType, ADFrameFlatField);
    setIntegerParam(DEX_CurrentGainFrame, 0);
    setIntegerParam(DEX_GainAvailable, 0);
    setIntegerParam(ADAcquire, 1);
    gainImage_ = DexImage();

    // Make sure the shutter is open
    setShutter(ADShutterOpen);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::SetTriggerSource(Internal_Software)\n",
      driverName, functionName);
    pDetector_->SetTriggerSource(Internal_Software);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::SetExposureMode(Sequence_Exposure)\n",
      driverName, functionName);
    pDetector_->SetExposureMode(Sequence_Exposure);

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::ToggleGenerator(true)\n",
      driverName, functionName);
    pDetector_->ToggleGenerator(true);      

    asynPrint(pasynUserSelf, ASYN_TRACEIO_DRIVER,
      "%s::%s calling DexelaDetector::GoLiveSeq()\n",
      driverName, functionName);
    pDetector_->GoLiveSeq();

  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
}

//_____________________________________________________________________________________________

/** Saves an offset file */
asynStatus Dexela::saveOffsetFile(void)
{
  char filePath[256];
  char fileName[256];
  static const char *functionName = "saveOffsetFile";

  try {
    getStringParam(DEX_CorrectionsDirectory, sizeof(filePath), filePath);
    getStringParam(DEX_OffsetFile, sizeof(fileName), fileName);
    strcat(filePath, fileName);

    if (offsetImage_.IsEmpty()) return asynError;
    offsetImage_.WriteImage(filePath);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  return asynSuccess;
}


//_____________________________________________________________________________________________

/** Loads an offset file */
asynStatus Dexela::loadOffsetFile (void)
{
  char filePath[256];
  char fileName[256];
  static const char *functionName = "loadOffsetFile";

  try {
    getStringParam(DEX_CorrectionsDirectory, sizeof(filePath), filePath);
    getStringParam(DEX_OffsetFile, sizeof(fileName), fileName);
    strcat(filePath, fileName);

    offsetImage_.ReadImage(filePath);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  return asynSuccess;
}
//_____________________________________________________________________________________________

/** Saves a gain file */
asynStatus Dexela::saveGainFile(void)
{
  char filePath[256];
  char fileName[256];
  static const char *functionName = "saveGainFile";

  try {
    getStringParam(DEX_CorrectionsDirectory, sizeof(filePath), filePath);
    getStringParam(DEX_GainFile, sizeof(fileName), fileName);
    strcat(filePath, fileName);

    if (gainImage_.IsEmpty()) return asynError;
    gainImage_.WriteImage(filePath);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  return asynSuccess;
}


//_____________________________________________________________________________________________

/** Loads a gain file */
asynStatus Dexela::loadGainFile (void)
{
  char filePath[256];
  char fileName[256];
  static const char *functionName = "loadGainFile";

  try {
    getStringParam(DEX_CorrectionsDirectory, sizeof(filePath), filePath);
    getStringParam(DEX_GainFile, sizeof(fileName), fileName);
    strcat(filePath, fileName);

    gainImage_.ReadImage(filePath);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  return asynSuccess;
}
//_____________________________________________________________________________________________

/** Loads a defect file */
asynStatus Dexela::loadDefectMapFile()
{
  char filePath[256];
  char fileName[256];
  static const char *functionName = "loadDefectFile";

  try {
    getStringParam(DEX_CorrectionsDirectory, sizeof(filePath), filePath);
    getStringParam(DEX_DefectMapFile, sizeof(fileName), fileName);
    strcat(filePath, fileName);

    defectMapImage_.ReadImage(filePath);
  } catch (DexelaException &e) {
    reportError(functionName, e);
  }
  return asynSuccess;
}


/* Code for iocsh registration */

/* DexelaConfig */
static const iocshArg DexelaConfigArg0 = {"Port name",  iocshArgString};
static const iocshArg DexelaConfigArg1 = {"detIndex",   iocshArgInt};
static const iocshArg DexelaConfigArg2 = {"maxBuffers", iocshArgInt};
static const iocshArg DexelaConfigArg3 = {"maxMemory",  iocshArgInt};
static const iocshArg DexelaConfigArg4 = {"priority",   iocshArgInt};
static const iocshArg DexelaConfigArg5 = {"stackSize",  iocshArgInt};
static const iocshArg * const DexelaConfigArgs[] =  {&DexelaConfigArg0,
                                                     &DexelaConfigArg1,
                                                     &DexelaConfigArg2,
                                                     &DexelaConfigArg3,
                                                     &DexelaConfigArg4,
                                                     &DexelaConfigArg5};
static const iocshFuncDef configDexela = {"DexelaConfig", 6, DexelaConfigArgs};
static void configDexelaCallFunc(const iocshArgBuf *args)
{
  DexelaConfig(args[0].sval, args[1].ival, args[2].ival,
               args[3].ival, args[4].ival, args[5].ival);
}

static void DexelaRegister(void)
{
  iocshRegister(&configDexela, configDexelaCallFunc);
}

extern "C" {
epicsExportRegistrar(DexelaRegister);
}
