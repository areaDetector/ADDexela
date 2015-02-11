/* Dexela.h
 *
 * This is a driver for the Perkin Elmer Dexela detectors
 *
 *
 * Author: Mark Rivers
 *
 * Created:  02/07/2015
 *
 * Current author: Mark Rivers
 *
 */

#ifndef Dexela_H
#define Dexela_H

#include "ADDriver.h"
#include "DexelaDetector.h"

#define DEX_SerialNumberString               "DEX_SERIAL_NUMBER"
#define DEX_BinningModeString                "DEX_BINNING_MODE"
#define DEX_FullWellModeString               "DEX_FULL_WELL_MODE"
#define DEX_CorrectionsDirectoryString       "DEX_CORRECTIONS_DIRECTORY"
#define DEX_AcquireOffsetString              "DEX_ACQUIRE_OFFSET"
#define DEX_NumOffsetFramesString            "DEX_NUM_OFFSET_FRAMES"
#define DEX_CurrentOffsetFrameString         "DEX_CURRENT_OFFSET_FRAME"
#define DEX_UseOffsetString                  "DEX_USE_OFFSET"
#define DEX_OffsetAvailableString            "DEX_OFFSET_AVAILABLE"
#define DEX_AcquireGainString                "DEX_ACQUIRE_GAIN"
#define DEX_NumGainFramesString              "DEX_NUM_GAIN_FRAMES"
#define DEX_CurrentGainFrameString           "DEX_CURRENT_GAIN_FRAME"
#define DEX_UseGainString                    "DEX_USE_GAIN"
#define DEX_GainAvailableString              "DEX_GAIN_AVAILABLE"
#define DEX_GainFileString                   "DEX_GAIN_FILE"
#define DEX_LoadGainFileString               "DEX_LOAD_GAIN_FILE"
#define DEX_SaveGainFileString               "DEX_SAVE_GAIN_FILE"
#define DEX_UseDefectMapString               "DEX_USE_DEFECT_MAP"
#define DEX_DefectMapAvailableString         "DEX_DEFECT_MAP_AVAILABLE"
#define DEX_DefectMapFileString              "DEX_DEFECT_MAP_FILE"
#define DEX_LoadDefectMapFileString          "DEX_LOAD_DEFECT_MAP_FILE"
#define DEX_SoftwareTriggerString            "DEX_SOFTWARE_TRIGGER"


/** Driver for the Perkin Elmer Dexela CMOS flat panel detectors */

class Dexela : public ADDriver
{
public:
  Dexela(const char *portName, int detIndex, 
         int maxBuffers, size_t maxMemory,
         int priority, int stackSize);

  /* These are the methods that we override from ADDriver */
  virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
  virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
  virtual asynStatus readEnum(asynUser *pasynUser, char *strings[], int values[], int severities[], 
                              size_t nElements, size_t *nIn);
  void report(FILE *fp, int details);

  // These should really be private, but they are called from C so must be public
  void acquireStopTask(void);
  void newFrameCallback(int frameCounter, int bufferNumber);

  ~Dexela();

protected:
  int DEX_SerialNumber;
  #define DEX_FIRST_PARAM DEX_SerialNumber
  int DEX_BinningMode;
  int DEX_FullWellMode;
  int DEX_CorrectionsDirectory;
  int DEX_AcquireOffset;
  int DEX_NumOffsetFrames;
  int DEX_CurrentOffsetFrame;
  int DEX_UseOffset;
  int DEX_OffsetAvailable;
  int DEX_AcquireGain;
  int DEX_NumGainFrames;
  int DEX_CurrentGainFrame;
  int DEX_UseGain;
  int DEX_GainAvailable;
  int DEX_GainFile;
  int DEX_LoadGainFile;
  int DEX_SaveGainFile;
  int DEX_UseDefectMap;
  int DEX_DefectMapAvailable;
  int DEX_DefectMapFile;
  int DEX_LoadDefectMapFile;
  int DEX_SoftwareTrigger;
  #define DEX_LAST_PARAM DEX_SoftwareTrigger


private:
  DexelaDetector *pDetector_;
  DevInfo        devInfo_;
  BusScanner     *pBusScanner_;
  DexImage       offsetImage_;
  DexImage       gainImage_;
  DexImage       defectMapImage_;
  int            sensorX_;
  int            sensorY_;
  char           modelName_[80];
  int            serialNumber_;

  void reportSensors(FILE *fp, int details);
  void reportError(const char *functionName, DexelaException &e);
  void acquireStart(void);
  void acquireStop(void);
  void acquireOffsetImage(void);
  void acquireGainImage(void);
  asynStatus loadGainFile(void);
  asynStatus saveGainFile(void);
  asynStatus loadDefectMapFile();
};

#define NUM_DEXELA_PARAMS ((int)(&DEX_LAST_PARAM - &DEX_FIRST_PARAM + 1))

#endif

