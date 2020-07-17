// ******************************************************
//
// Copyright (c) 2020, Varex Imaging Corp., All rights reserved
//
// ******************************************************
//
// Exception handling class that is used throughout the DexelaDetector API
//
// ******************************************************

/// \file

#pragma once

#include "dexdefs.h"
#include <exception>

/// <summary>
/// This class contains information about any possible error's in the API. In the event of a problem a DexelaException will be thrown.
/// \n<b>Note: </b>It is suggested that you wrap your code in a try-catch block to ensure that if any errors occur you can detect (and properly handle them) in your code.
/// </summary>
class DllExport DexelaException : public std::exception
{
public:
	DexelaException(const char *message, Derr code, int line,
									const char *filename, const char *function, int transportEr,
									const char *transportMessage);
	DexelaException(const DexelaException &ex, const char *function);
	DexelaException(const DexelaException &ex);
	DexelaException &operator=(const DexelaException &ex);
	~DexelaException(void) throw();
	const char *what() const throw();
	Derr GetCode();
	int GetTransportError();
	const char *GetFileName();
	int GetLineNumber();
	const char *GetFunctionName();
	const char *GetTransportMessage();

	static void LoadErrorStrings(const char *filename);

private:
	const char *_msg;
	Derr _code;
	const char *_filename;
	int _line;
	const char *_func;
	int _transEr;
	const char *_transMsg;
};

#define __FILENAME__ \
	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define rethrowEr(EX) throw DexelaException(EX, __FUNCTION__);

#define throwNewEr(MSG, CODE, TRANSER, TRANSMSG)                         \
	throw DexelaException(MSG, CODE, __LINE__, __FILENAME__, __FUNCTION__, \
												TRANSER, TRANSMSG);
