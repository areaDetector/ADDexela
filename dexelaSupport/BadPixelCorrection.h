// BadPixelCorrection.h : main header file for the BadPixelCorrection DLL
//

/// \file

#ifndef _BADPIXELCORRECTION_H
#define _BADPIXELCORRECTION_H

//#pragma once

typedef struct _GeometryCorrectionParams
{
public:
	int iRefX;
	int iRefY;
} GeometryCorrectionParams;

// structure to specify the ROI information for defect correction
typedef struct _DexDefectCorrectionParams
{
public:
	// specify two cases using a boolean flag
	// case1 (true): input and output are already sub-images (of ROI dimensions)
	// case2: (false): all images are full dimensions, defect correction is done
	// inside the ROI while maintaining the original data outside the ROI
	bool bInputOutputAreSubimages;

	// Specify the start and end points of region of interest (ROI). If all ROI
	// coordinate are <= 0, the full correction will be used
	int iROI_StartX, iROI_StartY, iROI_EndX, iROI_EndY;
} DexDefectCorrectionParams;

// the following enum values can be combined (OR'ed) to switch ON/OF certain
// corrections for the Dexela defection correction
typedef enum
{
	/// <summary>
	/// Switch ON the bad pixels correction
	/// </summary>
	DexOptFLG_BadPixel = 1,
	/// <summary>
	/// Switch ON the cluster defect correction
	/// </summary>
	DexOptFLG_Cluster = 2,
	/// <summary>
	/// Switch ON the fast cluster defect correction
	/// </summary>
	DexOptFLG_FastCluster = 4,
	/// <summary>
	/// Switch ON the gain adjustment for columns, normally used for read-out columns
	/// </summary>
	DexOptFLG_GainAdjustment = 8,
	/// <summary>
	/// Switch ON the convolution based correction
	/// </summary>
	DexOptFLG_ConvoCor = 16,
	/// <summary>
	/// Switch ON the faster convolution based correction
	/// </summary>
	DexOptFLG_FastConvoCor = 32,
	/// <summary>
	/// Switch ON the extra operation on gain-adjusted pixel
	/// </summary>
	DexOptFLG_GainAdjustmentExtraProcess = 1024,
} enumDexDefCorOptimizeFlag;

extern "C" {
// legacy functions
int _declspec(dllexport)
		BadPixelCorrectionFloat(float *inBuf, float *outBuf, unsigned short *MapBuf,
														int imageWidth, int imageHeight);
int _declspec(dllexport)
		BadPixelCorrectionUShort(unsigned short *inBuf, unsigned short *outBuf,
														 unsigned short *MapBuf, int imageWidth,
														 int imageHeight);

int _declspec(dllexport)
		GeometryCorrectionFloat(float *inBuf, float *outBuf, int imageWidth,
														int imageHeight, GeometryCorrectionParams Params);
int _declspec(dllexport)
		GeometryCorrectionUShort(unsigned short *inBuf, unsigned short *outBuf,
														 int imageWidth, int imageHeight,
														 GeometryCorrectionParams Params);
}

extern "C++" {
// new functions

/// <summary>
/// defect correction for Dexela detectors
/// </summary>
/// Input data
/// Ourput data
/// Defect map data of usigned short only
/// image width
/// image height
/// pixel type: 2 = unsigned_short and 4 = float, 6 = unsigned-int-32
/// iOptimizeFlag: A bit-wise setting of different correction:
///                bit 0: for label 1 correction,
//                 bit 1: for label 11 correction,
//                 bit 2: for label 11 optimized correction,
//                 bit 3: for label 12 correction,
//                 bit 4: for swithing ON/OFF convolution based correction,
//                 bit 5: for turning ON/OFF the speed optimization for the
//                 convolution-based correction. bit 10: for fine-tuning already
//                 corrected label 12
///                Default value is 31
///				   Note that the values in the enumDexDefCorOptimizeFlag can be combined (OR'ed) to get the desired corrections
int _declspec(dllexport)
		DexDefectCorrection(void *inBuf, void *outBuf, unsigned short *MapBuf,
												int imageWidth, int imageHeight, int iPixelType,
												int iOptimizeFlag = 31);

/// <summary>
/// defect correction for Dexela detectors applied on region of interest. The correction will be applied in two cases:
/// case1: inBuf and outBuf are sub-images and the defect map is full image
/// case2: all images have full dimension, the defect correction is applied in the ROI and the original data outside the ROI is maintained
/// </summary>
/// Input data (sub-image)
/// Ourput data (sub-image)
/// Defect map data of usigned short only (original full image)
/// Original full image width
/// Original full image height
/// pixel type: 2 = unsigned_short and 4 = float, 6 = unsigned-int-32
/// IOptimizeFlag: Note that the values in the enumDexDefCorOptimizeFlag can be combined (OR'ed) to get the desired corrections
/// a structure where the ROI coordinates are specified and also a flag to switch between case1 and case2
int _declspec(dllexport)
		DexDefectCorrection(void *inBuf, void *outBuf, unsigned short *MapBuf,
												int imageWidth, int imageHeight, int iPixelType,
												int iOptimizeFlag,
												const DexDefectCorrectionParams &param);

/// <summary>
/// Geomtric correction for detectors that consist of more than one sensors
/// </summary>
// Input data
// Ourput data
// image width
// image height
// pixel type: 2 = unsigned_short and 4 = float, 6 = unsigned-int-32
// Dexela detector model number
int _declspec(dllexport)
		DexGeometryCorrection(void *inBuf, void *outBuf, int imageWidth,
													int imageHeight, int iPixelType,
													int iDetectorModelNumber);

/// <summary>
/// Geomtric correction for detectors that consist of more than one sensors
/// The correction will be applied in two cases:
/// case1: inBuf and outBuf are sub-images and the defect map is full image
/// case2: all images have full dimension, the defect correction is applied in the ROI and the original data outside the ROI is maintained
/// </summary>
// Input data
// Ourput data
// width of the full image (detector's width)
// height of the full image (detector's height)
// pixel type: 2 = unsigned_short and 4 = float, 6 = unsigned-int-32
// Dexela detector model number
int _declspec(dllexport)
		DexGeometryCorrection(void *inBuf, void *outBuf, int iFullImageWidth,
													int iFullImageHeight, int iPixelType,
													int iDetectorModelNumber,
													const DexDefectCorrectionParams &param);
}

#endif
