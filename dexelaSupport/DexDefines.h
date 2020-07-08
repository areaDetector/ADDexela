// ******************************************************
//
// Copyright (c) 2015, PerkinElmer Inc., All rights reserved
// 
// ******************************************************
//
// Contains defines and macros used throughout the API
//
// ******************************************************

#pragma once

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned char byte;

#if _WIN32 || _WIN64
#if _WIN64
#define _X64
#else
#define _X86
#endif
#endif

#define TransMsgSize 1024

//define import for applications using the natveapi library.
#ifdef DEX_BUILD
#define DllExport __declspec( dllexport )
#else
#define DllExport __declspec( dllimport )
#endif


#ifdef DEX_BUILD_C
#define DllExportC __declspec( dllexport )
#else
#define DllExportC __declspec( dllimport )
#endif
////////////DETECTOR CONSTANTS ////////////////////////////

/// <summary>
/// Maximum value for any pixel in detector output (14 bit)
/// </summary>
#define MAX_PIXEL_VAL 16383
/// <summary>
/// Minimum allowable pixel value
/// </summary>
#define MIN_PIXEL_VAL  0
/// <summary>
/// The minimum time increment in ms for expose and read mode
/// </summary>
#define minTimeIncrement 0.01F
/// <summary>
/// The minimum time increment in ns for line delay mode
/// </summary>
#define minTimeIncrement2  195.2F
/// <summary>
/// Variable to hold a time in ms used for sleeping threads in streaming mode.
/// </summary>
#define ExposureSleepTimems 10
/// <summary>
/// The resolution of the exposure time settings 1 for ms 100 for 0.01 ms
/// </summary>
#define TimingResolution 100

#define RETURN_CHAR_LENGTH_CONST 50



/// <summary>
/// The offset in pixels in x when reaing dark pixel data
/// </summary>d
#define DarkPixelXOffset 2
/// <summary>
/// The offset in pixels in y when reading dark pixel data
/// </summary>
#define DarkPixelYOffset 4


//////////REGISTER CONSTANTS/////////////////////////
/// <summary>
/// Register address for the FPGA version number of the detector
/// </summary>
#define AddrFPGANumber 126
/// <summary>
/// Register address for the serial number of the detector
/// </summary>
#define AddrSerialNumber 125
/// <summary>
/// Register address for the model number of the detector
/// </summary>
#define AddrModelNumber 124
/// <summary>
/// Register address for the gap time used in Frame Rate mode
/// </summary>
#define AddrGapTime 18
/// <summary>
/// Register address of number of frames register for use with sequence modes
/// </summary>
#define AddrNumberOfFrames 17
/// <summary>
/// Register address of firmware verion information
/// </summary>
#define AddrFirmwareVersion 127
/// <summary>
/// Register address of the Trigger Source bits
/// </summary>
#define AddrTriggerSource 0
///// <summary>
///// Register address of low bytes of exposure time information
///// </summary>
#define AddrExposureTimeLow  11
///// <summary>
///// Register address of high bytes of exposure time information
///// </summary>
#define AddrExposureTimeHigh  12
/// <summary>
/// Register address of exposure time information in low res system
/// </summary>
#define AddrExposureTime 12
/// <summary>
/// Register address of exposure time information in low res system for Line_Delay mode
/// </summary>
#define AddrExposureTime2 13
/// <summary>
/// Register address of low bytes of exposure time information for line delay mode
/// </summary>
#define AddrExposureTime2Low 13
/// <summary>
/// Register address of high bytes of exposure time information for line delay mode
/// </summary>
#define AddrExposureTime2High 14
/// <summary>
/// Address of horizontal binning register
/// </summary>
#define AddrHorizontalBinReg 10
/// <summary>
/// Address of vertical binning register
/// </summary>
#define AddrVerticalBinReg 9
/// <summary>
/// Address of control register
/// </summary>
#define AddrControlReg 0
/// <summary>
/// Register address of low bytes of pre-programmed exposure time 1 information
/// </summary>
#define AddrPPExposreTime1Low 27
/// <summary>
/// Register address of high bytes of pre-programmed exposure time 1 information
/// </summary>
#define AddrPPExposreTime1High 28
/// <summary>
/// Register address of low bytes of pre-programmed exposure time 2 information
/// </summary>
#define AddrPPExposreTime2Low 29
/// <summary>
/// Register address of high bytes of pre-programmed exposure time 2 information
/// </summary>
#define AddrPPExposreTime2High 30
/// <summary>
/// Register address of low bytes of pre-programmed exposure time 3 information
/// </summary>
#define AddrPPExposreTime3Low 31
/// <summary>
/// Register address of high bytes of pre-programmed exposure time 3 information
/// </summary>
#define AddrPPExposreTime3High 32
/// <summary>
/// Register address of low bytes of pre-programmed exposure time 4 information
/// </summary>
#define AddrPPExposreTime4Low 33
/// <summary>
/// Register address of high bytes of pre-programmed exposure time 4 information
/// </summary>
#define AddrPPExposreTime4High 34
/// <summary>
/// Address of serial number register 1
/// </summary>
#define SerialNumberReg1  0
/// <summary>
/// Address of serial number register 2
/// </summary>
#define SerialNumberReg2 0
/// <summary>
/// Address of serial number register 3
/// </summary>
#define SerialNumberReg3 0
/// <summary>
/// Address of temperature register
/// </summary>
#define TemperatureReg 0
/// <summary>
/// Address of Well register
/// </summary>
#define AddrWellReg 3
/// <summary>
/// Address of High Fullwell register
/// </summary>
#define AddrWellHigh 4
/// <summary>
/// Address of Low Fullwell register
/// </summary>
#define AddrWellLow 65531
/// <summary>
/// Address of sensor bin register
/// </summary>
#define AddrSensorBinReg 3
/// <summary>
/// Address of sensor bin register 2
/// </summary>
#define AddrSensorBinReg2 5
/// <summary>
/// Address of Numer of lines per sensor register
/// </summary>
#define AddrNumLines  7
/// <summary>
/// Address of Number of pixels (per line) per sensor register
/// </summary>
#define AddrNumPixels  8
/// <summary>
/// Constant for clearing binning command
/// </summary>
#define SensorBinClear 65087
/// <summary>
/// Constant for digital binning bit
/// </summary>
#define DigitalBinBit 65533
/// <summary>
/// Constant for 1x1 binning command
/// </summary>
#define Sensor1x1 0 //000 65087
/// <summary>
/// Constant for 1x2 binning command
/// </summary>
#define Sensor1x2 0 //000 65087
/// <summary>
/// Constant for 1x4 binning command
/// </summary>
#define Sensor1x4 64 //reset 65087 or 64
/// <summary>
/// Constant for 2x1 binning command
/// </summary>
#define Sensor2x1 128 //128
/// <summary>
/// Constant for 2x2 binning command
/// </summary>
#define Sensor2x2 128 //128
/// <summary>
/// Constant for 2x4 binning command
/// </summary>
#define Sensor2x4 192 //192
/// <summary>
/// Constant for 4x1 binning command
/// </summary>
#define Sensor4x1 256 //256
/// <summary>
/// Constant for 4x2 binning command
/// </summary>
#define Sensor4x2 256 //256
/// <summary>
/// Constant for 4x4 binning command
/// </summary>
#define Sensor4x4 320 //320
/// <summary>
/// Constant for binning commit command
/// </summary>
#define BinCommit 514
/// <summary>
/// Readout time for the sensor, dynamically adapted to the read out mode (binning, ROI). Dependent on the implementation the value has to be muliplied by the factor ReadoutTimeFactor
/// </summary>
#define AddrReadOutTime				410
/// <summary>
/// ReadoutTimeFactor for Dexela 1313
/// </summary>
#define ReadoutTimeFactor1313		2
/// <summary>
/// Register address for the ROI OFFSET / first column:
/// </summary>
#define AddrROIStartColumn			404
/// <summary>
/// Register address for the width of the ROI stripe
/// </summary>
#define AddrROIwidth				405
/// <summary>
/// Register address for the ROI OFFSET / first row:
/// </summary>
#define AddrROIStartRow				402
/// <summary>
/// Register address for the height of the ROI stripe
/// </summary>
#define AddrROIheight				403
/// <summary>
/// Register address for 16bit framecounter
/// </summary>
#define AddrFrameCounter			63								//0x3F
/// <summary>
/// Register address for frame packing mode: number of images per block
/// </summary>
#define AddrFramePackingMode_ImageCountPerBlock		64			//0x40
/// <summary>
/// Register address for frame packing mode: number of images per block
/// </summary>
#define AddrFramePackingMode_BlockHeightInRows		65			//0x41
/// <summary>
/// Register address storing day and month of the current firmware build
/// </summary>
#define AddrBuildDayAndMonth			38
/// <summary>
/// Register address storing the year of the current firmware built
/// </summary>
#define AddrBuildYear					39
/// <summary>
/// Register address storing the time of the current firmware built
/// </summary>
#define AddrBuildTime					40
/// <summary>
/// Register address storing the first 16bit of detector read-out time
/// will be retrieved in ticks (1 tick = ReadOutTimeBase1313 ns)
/// </summary>
#define AddrReadOutTimeLow				55
/// <summary>
/// Register address storing the second 16bit of detector read-out time
/// will be retrieved in ticks (1 tick = ReadOutTimeBase1313 ns)
/// </summary>
#define AddrReadOutTimeHigh				56
/// <summary>
/// Address of control register 1
/// </summary>
#define AddrControlReg1					1
/// <summary>
/// Address of features register 0
/// </summary>
#define AddrFeaturesReg0				36
/// <summary>
/// Address of features register 1
/// </summary>
#define AddrFeaturesReg1				37
/// <summary>
/// Address of features register 2
/// </summary>
#define AddrFeaturesReg2				130

#define SUPPORTS_x12_BINNING_FLAG					0x1
#define SUPPORTS_x14_BINNING_FLAG					0x2
#define SUPPORTS_x21_BINNING_FLAG					0x4
#define SUPPORTS_x22_BINNING_FLAG					0x8
#define SUPPORTS_x24_BINNING_FLAG					0x10
#define SUPPORTS_x41_BINNING_FLAG					0x20
#define SUPPORTS_x42_BINNING_FLAG					0x40
#define SUPPORTS_x44_BINNING_FLAG					0x80
#define SUPPORTS_iX22_BINNING_FLAG					0x100
#define SUPPORTS_FPGA_CROSSTALK_CORRECITON_FLAG		0x200
#define SUPPORTS_FPGA_LINEARIZATION_FLAG			0x400
#define SUPPORTS_TEST_LED_PULSE_MODE_FLAG			0x800
#define SUPPORTS_SEQUENCE_MODE_FLAG					0x1000
#define SUPPORTS_PREPROGRAMMED_EXP_MODE_FLAG		0x2000
#define SUPPORTS_FRAMERATE_MODE_FLAG				0x4000
#define SUPPORTS_IDLE_MODE_FLAG						0x8000
#define SUPPORTS_OUTPUT_PORT_EXCHANGE_FLAG			0x1
#define SUPPORTS_DOSE_SENSING_DURATION_FLAG			0x2
#define SUPPORTS_DOSE_SENSING_EDGE_FLAG				0x4
#define SUPPORTS_QUERYABLE_READOUT_FLAG				0x8
#define SUPPORTS_ROI_MODE_FLAG						0x10
#define SUPPORTS_RESOULTION_MODE_FLAG				0x20
#define SUPPORTS_ONBOARD_DARK_CORRECTION_FLAG		0x40
#define SUPPORTS_ONBOARD_FLOOD_CORRECTION_FLAG		0x80
#define SUPPORTS_ONBOARD_DEFECT_CORRECTION_FLAG		0x100
#define SUPPORTS_ONBOARD_DATASORTING_FLAG			0x200
#define SUPPORTS_x11_BINNING_FLAG					0x400
#define SUPPORTS_EXPOSE_AND_READ_FLAG				0x800
#define SUPPORTS_CONTINUOUS_READOUT_FLAG			0x1000
#define SUPPORTS_EXT_EDGE_TRIG_FLAG					0x2000
#define SUPPORTS_EXT_DURATION_TRIG_FLAG				0x4000
#define SUPPORTS_SW_TRIG_FLAG						0x8000
#define SUPPORTS_LOW_FW_MODE_FLAG					0x1
#define SUPPORTS_HIGH_FW_MODE_FLAG					0x2
#define SUPPORTS_DIRECT_TEMP_FLAG					0x4
#define SUPPORTS_INDIRECT_TEMP_FLAG					0x8

//this will be used to determine if any of the new flags are present
#define EXTRA_FLAGS									0xFE00 
/// < summary>
/// Address of temperature readout control
/// </summary>
#define AddrTempCntl					116
/// < summary>
/// Address of temperature sensor 1
/// </summary>
#define AddrTempRead1					112
/// < summary>
/// Address of temperature readout control
/// </summary>
#define AddrTempCntl					116
/// < summary>
/// Address of temperature sensor 1
/// </summary>
#define AddrTempRead1					112

#define AVGERAGED_FLAG					1
#define CLEAR_AVERAGED_FLAG				0xFFFE
#define FIXED_FLAG						2
#define CLEAR_FIXED_FLAG				0xFFFD
#define LINEARIZED_FLAG					4
#define CLEAR_LINEARIZED_FLAG			0xFFFB
#define SORTED_FLAG						8
#define CLEAR_SORTED_FLAG				0xFFF7
#define OPERATION_KNOWN_FLAG			0x8000
#define CLEAR_OPERATION_KNOWN_FLAG		0x7FFF
#define NOOP_FLAG						0x0
#define XIS_OFFSET_CORRECTED_FLAG			1
#define CLEAR_XIS_OFFSET_CORRECTED_FLAG		0xFFFE
#define XIS_GAIN_CORRECTED_FLAG				2
#define CLEAR_XIS_GAIN_CORRECTED_FLAG		0xFFFD
#define XIS_DEFECT_CORRECTED_FLAG			4
#define CLEAR_XIS_DEFECT_CORRECTED_FLAG		0xFFFB
#define XIS_MULTIGAIN_CORRECTED				8		/*this is not currently used appart from in XIS*/
#define CLEAR_XIS_MULTIGAIN_CORRECTED_FLAG	0xFFF7
#define DEX_OFFSET_CORRECTED_FLAG			16		/*Dexela versions of the corrections*/
#define CLEAR_DEX_OFFSET_CORRECTED_FLAG		0xFFEF
#define DEX_GAIN_CORRECTED_FLAG				32		
#define CLEAR_DEX_GAIN_CORRECTED_FLAG		0xFFDF
#define DEX_DEFECT_CORRECTED_FLAG			64		
#define CLEAR_DEX_DEFECT_CORRECTED_FLAG		0xFFBF
#define DEX_EXTRA_PARAMS_FLAG				0x4000	/*this flag will indicate the presence of new parameters (e.g. model, binning, operations) in the HIS header*/
#define CLEAR_DEX_EXTRA_PARAMS_FLAG			0xBFFF
#define CORRECTION_KNOWN_FLAG			0x8000
#define CLEAR_CORRECTION_KNOWN_FLAG		0x7FFF
#define UNCORRECTED_FLAG				0x0

#define LINEARIZAION_ENABLE_FLAG		0x4
#define LINEARIZATION_DISABLE_FLAG		0xFFFB
#define XTALK_CORRECTION_ENABLE_FLAG	0x4000
#define XTALK_CORRECTION_DISABLE_FLAG	0xBFFF

#define	TIFFTAG_DEX_CORRECTION_FLAGS	34595	/* New tiff-tag for storing correction flags parameter */
#define	TIFFTAG_DEX_OPERATION_FLAGS		34596	/* New tiff-tag for storing operation flags parameter */
#define	TIFFTAG_DEX_IMAGE_TYPE			34597	/* New tiff-tag for storing image-type parameter */
#define	    DEX_DATA_IMAGE			0	/* regular data image */
#define	    DEX_OFFSET_IMAGE		1	/* offset data image */
#define	    DEX_GAIN_IMAGE			2	/* gain data image */
#define     DEX_DEFECT_MAP			3   /* defect map image */
#define	    DEX_UNKONWN_TYPE_IMAGE	0xFF	/* type of image is unknown */
#define	TIFFTAG_DEX_MODEL_NUM			34598	/* New tiff-tag for storing image-type parameter */
#define	TIFFTAG_DEX_BIN_FMT				34599
#define TIFFTAG_ROI_START_COL			34600
#define TIFFTAG_ROI_START_ROW			34601
#define TIFFTAG_DEFECT_FLAGS			34602


#define MAX_REG_ADDR					999
#define MAX_REG_VALUE					0xFFFF

#ifdef __cplusplus

class BaseImage;
class baseDetector;
class gigEDetector;
class camLinkDetector;
class PleoraLib;
class xclib;
class baseBusScanner;

#endif