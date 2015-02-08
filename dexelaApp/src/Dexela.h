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
#define DEX_InitializeString                 "DEX_INITIALIZE"
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
#define DEX_UsePixelCorrectionString         "DEX_USE_PIXEL_CORRECTION"
#define DEX_PixelCorrectionAvailableString   "DEX_PIXEL_CORRECTION_AVAILABLE"
#define DEX_PixelCorrectionFileString        "DEX_PIXEL_CORRECTION_FILE"
#define DEX_LoadPixelCorrectionFileString    "DEX_LOAD_PIXEL_CORRECTION_FILE"
#define DEX_GainString                       "DEX_GAIN"
#define DEX_DwellTimeString                  "DEX_DWELL_TIME"
#define DEX_NumFrameBuffersString            "DEX_NUM_FRAME_BUFFERS"
#define DEX_TriggerString                    "DEX_TRIGGER"
#define DEX_SyncTimeString                   "DEX_SYNC_TIME"
#define DEX_FrameBufferIndexString           "DEX_FRAME_BUFFER_INDEX"
#define DEX_ImageNumberString                "DEX_IMAGE_NUMBER"
#define DEX_SkipFramesString                 "DEX_SKIP_FRAMES"
#define DEX_NumFramesToSkipString            "DEX_NUM_FRAMES_TO_SKIP"


class Dexela;

/** Driver for the Perkin Elmer Dexela CMOS flat panel detectors */

class Dexela : public ADDriver
{
public:
  Dexela(const char *portName, int detIndex, int numFrameBuffers, 
              int maxBuffers, size_t maxMemory,
              int priority, int stackSize);

  /* These are the methods that we override from ADDriver */
  virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
  virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
  void report(FILE *fp, int details);

  // These should really be private, but they are called from C so must be public
  void acquireStopTask(void);
  void newFrameCallback(int frameCounter, int bufferNumber);

  ~Dexela();

protected:
  int DEX_SerialNumber;
  #define DEX_FIRST_PARAM DEX_SerialNumber
  int DEX_Initialize;
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
  int DEX_UsePixelCorrection;
  int DEX_PixelCorrectionAvailable;
  int DEX_PixelCorrectionFile;
  int DEX_LoadPixelCorrectionFile;
  int DEX_Gain;
  int DEX_DwellTime;
  int DEX_NumFrameBuffers;
  int DEX_Trigger;
  int DEX_SyncTime;
  int DEX_FrameBufferIndex;
  int DEX_ImageNumber;
  int DEX_SkipFrames;
  int DEX_NumFramesToSkip;
  #define DEX_LAST_PARAM DEX_NumFramesToSkip


private:
  DexelaDetector *pDetector_;
  DevInfo        devInfo_;
  BusScanner     *pBusScanner_;
  epicsEventId  acquireStopEvent_;
  epicsUInt16   *pAcqBuffer_;
  epicsUInt16   *pOffsetBuffer_;
  epicsUInt16   *pGainBuffer_;
  epicsUInt16   *pBadPixelMap_;
  int           *pPixelCorrectionList_;
  unsigned int  uiNumFrameBuffers_;
  unsigned int  uiNumBuffersInUse_;
  int           iAcqMode_;
  double        dAcqTimeReq_;
  double        dAcqTimeAct_;
  int           iTrigModeReq_;
  int           iTrigModeAct_;

  void setBinning(void);
  void reportSensors(FILE *fp, int details);

  void acquireStart(void);
  void acquireStop(void);
  void acquireOffsetImage(void);
  void acquireGainImage(void);

  asynStatus loadGainFile(void);
  asynStatus saveGainFile(void);
  asynStatus loadPixelCorrectionFile();

  asynStatus setTriggerMode(void);
  asynStatus setExposureTime(void);

};


#define NUM_DEXELA_PARAMS ((int)(&DEX_LAST_PARAM - &DEX_FIRST_PARAM + 1))

#endif
