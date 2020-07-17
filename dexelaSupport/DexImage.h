// ******************************************************
//
// Copyright (c) 2020, Varex Imaging Corp., All rights reserved
//
// ******************************************************
//
// This class is used to perform all image related functionality.
//
// ******************************************************

/// \file

#pragma once

#include "DexDefs.h"
#include "DexImage.h"
#include <memory>
#include <vector>

#pragma warning(disable : 4251)

/// <summary>
/// This class is used to store and handle the images acquired from a detector.
/// </summary>
class DllExport DexImage
{
private:
	std::shared_ptr<BaseImage> baseIm;

	friend class Dex_ImageTest;
	friend class Base_ImageTest;

public:
	DexImage(void);
	DexImage(const char *filename);
	DexImage(const DexImage &input);
	void operator=(const DexImage &input);
	~DexImage(void);

	void ReadImage(const char *filename);
	void WriteImage(const char *filename);
	void WriteImage(const char *filename, int iZ);
	void Build(int iWidth, int iHeight, int iDepth, pType iPxType);
	void Build(int model, bins binFmt, int iDepth);
	void Build(int model, bins binFmt, int iDepth, pType iPxType,
						 int sensorVersion, bool isSliceInterlaced);
	void Build(int model, bins binFmt, int iDepth, pType iPxType,
						 int sensorVersion, bool isSliceInterlaced, bool roiEnabled,
						 int roiStartColumn, int roiStartRow, int roiWidth, int roiHeight);

	void *GetDataPointerToPlane(int iZ = 0);
	int GetImageXdim();
	int GetImageYdim();
	int GetImageDepth();
	pType GetImagePixelType();
	float PlaneAvg(int iZ = 0);
	void FixFlood();
	void FindMedianofPlanes();
	void FindAverageofPlanes();
	void LinearizeData();
	void SubtractDark();
	void FloodCorrection();
	void DefectCorrection(int DefectFlags = 31);
	void SubImageDefectCorrection(int startCol, int startRow, int width,
																int height, int CorrectionsFlag = 31);
	void FullCorrection(int DefectFlags = 31);
	void UnscrambleImage();
	void AddImage();

	void SetDefectFlags(int flags);
	int GetDefectFlags();

	void GetROI(int &startColumn, int &startRow, int &width, int &height);
	void GetCapturedROI(int &startColumn, int &startRow, int &width, int &height);
	bool IsROI();
	bool IsCropped();

	bool IsSliceInterlaced();

	void GetSubImage(int startCol, int startRow, int width, int height);

	void LoadDarkImage(const DexImage &dark);
	void LoadDarkImage(const char *filename);
	void LoadFloodImage(const DexImage &flood);
	void LoadFloodImage(const char *filename);
	void LoadDefectMap(const DexImage &defect);
	void LoadDefectMap(const char *filename);

	DexImage GetDarkImage();
	DexImage GetFloodImage();
	DexImage GetDefectMap();
	DexImage GetImagePlane(int iZ);

	DexImageTypes GetImageType();
	void SetImageType(DexImageTypes type);

	void SetDarkOffset(int offset);
	int GetDarkOffset();

	void SetLinearizationStarts(std::vector<unsigned int> linearizationStarts);
	void SetLinearizationStarts(const unsigned int *linearizationStarts,
															std::size_t size);
	std::vector<unsigned int> GetLinearizationStarts();
	int GetLinearizationStarts(unsigned int *linearizationStarts,
														 std::size_t size);

	void SetImageParameters(bins binningMode, int modelNumber,
													bool isSliceInterlaced = true, int sensorVersion = 1,
													bool roiEnabled = false, int roiStartColumn = 0,
													int roiStartRow = 0, int roiWidth = 0,
													int roiHeight = 0);

	int GetImageModel();
	int GetSensorVersion();
	bins GetImageBinning();

	bool IsEmpty();

	bool IsDefectCorrected();
	bool IsDarkCorrected();
	bool IsFloodCorrected();
	bool IsAveraged();
	bool IsFixed();
	bool IsLinearized();
	bool IsSorted();

	void SetFloodCorrectedFlag(bool onOff);
	void SetDarkCorrectedFlag(bool onOff);
	void SetDefectCorrectedFlag(bool onOff);
	void SetAveragedFlag(bool onOff);
	void SetFixedFlag(bool onOff);
	void SetLinearizedFlag(bool onOff);
	void SetSortedFlag(bool onOff);
};
