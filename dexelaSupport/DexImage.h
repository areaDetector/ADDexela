// ******************************************************
//
// Copyright (c) 2015, PerkinElmer Inc., All rights reserved
// 
// ******************************************************
//
// This class is used to perform all image related functionality. 
//
// ******************************************************

#pragma once

#ifndef DEX_BUILD
#ifdef _DEBUG
#pragma comment(lib,"DexImage-d.lib")
#else
#pragma comment(lib,"DexImage.lib")
#endif
#endif
#include "DexDefs.h"
#include "DexImage.h"
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace std;

#pragma warning(disable: 4251)

/// <summary>
/// This class is used to store and handle the images acquired from a detector.
/// </summary>
class  DllExport DexImage
{

#ifdef MOCK_TEST
	public:
#else
	private:
#endif
		boost::shared_ptr<BaseImage> baseIm;

public:
		DexImage(void);
		DexImage(const char* filename);
		DexImage(const DexImage &input);
		void operator =(const DexImage &input);
		~DexImage(void);

		void ReadImage(const char* filename);
		void WriteImage(const char* filename);
		void WriteImage(const char* filename, int iZ);
		void Build(int iWidth, int iHeight, int iDepth, pType iPxType);
		void Build(int model, bins binFmt, int iDepth);

		void* GetDataPointerToPlane(int iZ=0);
		int GetImageXdim();
		int GetImageYdim();
		int GetImageDepth();
		pType GetImagePixelType();
		float PlaneAvg(int iZ=0);
		void FixFlood();
		void FindMedianofPlanes();
		void FindAverageofPlanes();
		void LinearizeData();
		void SubtractDark();
		void FloodCorrection();
		void DefectCorrection(int DefectFlags=31);
		void SubImageDefectCorrection(int startCol, int startRow, int width, int height,int CorrectionsFlag=31);
		void FullCorrection(int DefectFlags=31);
		void UnscrambleImage();
		void AddImage();

		void LoadDarkImage(const DexImage &dark);
		void LoadDarkImage(const char* filename);
		void LoadFloodImage(const DexImage &flood);
		void LoadFloodImage(const char* filename);
		void LoadDefectMap(const DexImage &defect);
		void LoadDefectMap(const char* filename);

		DexImage GetDarkImage();
		DexImage GetFloodImage();
		DexImage GetDefectMap();
		DexImage GetImagePlane(int iZ);

		DexImageTypes GetImageType();
		void SetImageType(DexImageTypes type);

		void SetDarkOffset(int offset);
		int GetDarkOffset();

		void SetLinearizationStarts(vector<unsigned int> linearizationStarts);
		vector<unsigned int> GetLinearizationStarts();

		void SetImageParameters(bins binningMode, int modelNumber);

		int GetImageModel();
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
