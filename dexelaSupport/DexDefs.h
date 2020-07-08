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

#include "DexDefines.h"
#include "windows.h"

////////////////////Enumerations//////////////////////////

/// <summary>
/// An enumeration of detector interface types.
/// </summary>
typedef enum //DetectorInterface 
{
	/// <summary>
    /// CameraLink
    /// </summary>
	CL,
	/// <summary>
    /// Gigabit Ethernet
    /// </summary>
    GIGE    
}DetectorInterface;

/// <summary>
/// An enumeration of detector interface types.
/// </summary>
typedef enum //TransportLib
{
	/// <summary>
    /// Pleora type GigE transport library
    /// </summary>
	Pleora,
	/// <summary>
    /// Epix FG CameraLink transport library
    /// </summary>
    Epix    
}TransportLib;

/// <summary>
/// A structure to hold device information.
/// </summary>
typedef struct //DevInfo
{
    /// <summary>
    /// The Device Model Number
    /// </summary>
    int model;
    /// <summary>
    /// The Device Serial Number
    /// </summary>
    int serialNum;
    /// <summary>
	/// The interface type (e.g. GigE or CameraLink)
    /// </summary>
    DetectorInterface iface;
	/// <summary>
	/// Pointer to the parameter needed for opening detector
	/// </summary>
	char param[50];
	/// <summary>
	/// Unit number for cameralink detectors
	/// </summary>
	int unit;
	/// <summary>
	/// Low level tranport library
	/// <summary/>
	TransportLib transport;
}DevInfo;

/// <summary>
/// An enumeration of the different bin levels available
/// </summary>
typedef enum //bins
{
    /// <summary>
    /// Unbinned
    /// </summary>
    x11 = 1,
    /// <summary>
    ///  Binned vertically by 2
    /// </summary>
    x12,
    /// <summary>
    ///  Binned vertically by 4
    /// </summary>
    x14,
    /// <summary>
    ///  Binned horizontally by 2
    /// </summary>
    x21,
    /// <summary>
    /// Binned horizontally by 2 and vertically by 2
    /// </summary>
    x22,
    /// <summary>
    /// Binned horizontally by 2 and vertically by 4
    /// </summary>
    x24,
    /// <summary>
    /// Binned horizontally by 4
    /// </summary>
    x41,
    /// <summary>
    /// Binned horizontally by 4 and vertically by 2
    /// </summary>
    x42,
    /// <summary>
    /// Binned horizontally by 4 and vertically by 4
    /// </summary>
    x44,
	/// <summary>
    /// Digital 2x2 binning
    /// </summary>
    ix22,
	/// <summary>
	/// Digital 4x4 binning
	/// </summary>
	ix44,
    /// <summary>
	/// Indcates that the binning mode is not known (used mainly in the <see cref="DexImage"/> class)
    /// </summary>
    BinningUnknown

}bins;

/// <summary>
/// An enumeration of file types
/// </summary>
typedef enum //FileType 
{
	/// <summary>
	/// SMV
	/// </summary>
	SMV,
	/// <summary>
	/// TIFF
	/// </summary>
	TIF,
	/// <summary>
	/// HIS
	/// </summary>
	HIS,
	/// <summary>
	/// Unknown/Unsupported
	/// </summary>
	UNKNOWN
}FileType;


/// <summary>Enumration for error codes returned from the API functions</summary>
typedef enum //Derr
{
    /// <summary>The operation was successful</summary>
    SUCCESS,
    /// <summary>The image pointer was NULL</summary>
    NULL_IMAGE,
    /// <summary>The image pixel type was wrong for the operation requested</summary>
    WRONG_TYPE,
    /// <summary>The image dimesions were wrong for the operation requested</summary>
    WRONG_DIMS,
    /// <summary>One or more parameters were incorrect</summary>
    BAD_PARAM,
    /// <summary>The communications channel is not open or could not be openned</summary>
    BAD_COMMS,
    /// <summary>An invalid trigger source was requested</summary>
    BAD_TRIGGER,
    /// <summary>The communications channel failed to open</summary>
    BAD_COMMS_OPEN,
    /// <summary>A failure in a detector write command occurred</summary>
    BAD_COMMS_WRITE,
    /// <summary>A failure in a detector read command occurred</summary>
    BAD_COMMS_READ,
    /// <summary>An error occurred openning or reading from a file</summary>
    BAD_FILE_IO,
    /// <summary>The software failed to open the PC driver or frame grabber</summary>
    BAD_BOARD,
    /// <summary>A function call was not able to reserve the memory it required</summary>
    OUT_OF_MEMORY,
	/// <summary>Exposure Acquisition failed</summary>
    EXPOSURE_FAILED,
	/// <summary>Incorrect bin level specified</summary>
	BAD_BIN_LEVEL
}Derr;


/// <summary>
/// An enumeration of the available full well modes
/// </summary>
typedef enum //FullWellModes
{
    /// <summary>
    /// The low noise reduced dynamic range mode
    /// </summary>
    Low=0,
    /// <summary>
    /// The normal full well mode
    /// </summary>
    High
}FullWellModes;


/// <summary>
/// Enumneration of pixel types
/// </summary>
typedef enum //pType
{
    /// <summary>
    /// A pixel type of 16-bit unsigned short
    /// </summary>
    u16 = 2,
    /// <summary>
    /// A pixel type of 32-bit floating point
    /// </summary>
    flt = 4,
	/// <summary>
	/// A pixel type of 32-bit unsigned int
	/// </summary
	u32 = 6
}pType;

/// <summary>
/// An enumeration of exposure modes.
/// </summary>
typedef enum //ExposureModes
{
    /// <summary>
    /// The detector should clear the sensor and wait for exposure time to pass before reading the detector image.
    /// </summary>
    Expose_and_read,
    /// <summary>
    /// The detector should take a sequence of images with no gaps.
    /// </summary>
	Sequence_Exposure,
	/// <summary>
    /// The detector should take a sequence of images with a specified gap no less than the minimum exposure timeforthe bin level.
    /// </summary>
	Frame_Rate_exposure,
	/// <summary>
    /// The detector should take a number of images with preset exposure times without a gap..
    /// </summary>
	Preprogrammed_exposure
}ExposureModes;


/// <summary>
/// An enumeration of exposure trigger sources.
/// </summary>
typedef enum //ExposureTriggerSource
{
     /// <summary>
    /// Trigger on negative edge
    /// </summary>
    Ext_neg_edge_trig,
    /// <summary>
    /// Trigger using software
    /// </summary>
    Internal_Software,
    /// <summary>
    /// Detector exposure duration and trigger contorlled externally
    /// </summary>
    Ext_Duration_Trig
}ExposureTriggerSource;

/// <summary>
/// Structure to hold the detector current status.
/// </summary>
typedef struct //DetStatus
{

    /// <summary>
    /// The currently set exposure mode
    /// </summary>
    ExposureModes exposureMode;
    /// <summary>
    /// The currently set Full Well mode
    /// </summary>
    FullWellModes fullWellMode;
    /// <summary>
    /// The currently set exposure time
    /// </summary>
    float exposureTime;
    /// <summary>
    /// The curently set bin level
    /// </summary>
    bins binLevel;
    /// <summary>
    /// The currenlty set Trigger Source
    /// </summary>
    ExposureTriggerSource triggerSource;
    /// <summary>
    /// True if the detector test mode is set to on
    /// </summary>
    BOOL testMode;
} DetStatus;


/// <summary>
/// An enumeration of ReadOut modes.
/// </summary>
typedef enum //ReadoutModes
{
    /// <summary>
    /// The sensor is continuously read-out using the minimum read-out time. On request an image will be transmitted. A frame request can be an external trigger pulse, internal trigger or software trigger
    /// </summary>
	ContinuousReadout,

	/// <summary>
    /// The sensor is only read out (using the minimum frame time) on request. The read-out will be followed by the transmission of the image. A frame request can be an external trigger pulse, internal trigger or software trigger
    /// </summary>
    IdleMode
} ReadoutModes;


/// <summary>
/// An enumeration of the different image types.
/// </summary>
typedef enum //DexImageTypes
{
	/// <summary>
    /// A data image
    /// </summary>
	Data = 0,
	/// <summary>
	/// An offset (dark) image
    /// </summary>
	Offset = 1,
	/// <summary>
	/// An gain (flood) image
    /// </summary>
	Gain = 2,
	/// <summary>
	/// A defect-map image
    /// </summary>
	Defect = 3,
	/// <summary>
	/// The type of the image is not known
    /// </summary>
	UnknownType = 0xFF
}DexImageTypes;

