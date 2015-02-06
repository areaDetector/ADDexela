/*! \file */
#pragma once

#ifndef DEX_BUILD
#ifdef _DEBUG
#pragma comment(lib,"DexelaException-d.lib")
#else
#pragma comment(lib,"DexelaException.lib")
#endif
#endif

#include "dexdefs.h"
#include <exception>

using namespace std;

/// <summary>
/// This class contains information about any possible error's in the API. In the event of a problem a DexelaException will be thrown. 
/// \n<b>Note: </b>It is suggested that you wrap your code in a try-catch block to ensure that if any errors occur you can detect (and properly handle them) in your code.
/// </summary>
class DllExport DexelaException :
	public exception
{
public:
	DexelaException(const char* message, Derr code, int line, const char* filename, const char* function, int transportEr, const char* transportMessage);
	DexelaException(const DexelaException& ex, const char* function);
	~DexelaException(void) throw();
	const char* what() const throw ();
	Derr GetCode();
	int GetTransportError();
	const char* GetFileName();
	int GetLineNumber();
	const char* GetFunctionName();
	const char* GetTransportMessage();
	

	static void LoadErrorStrings(const char* filename);

private:
	const char* _msg;
	Derr _code;
	const char* _filename;
	int _line;
	const char* _func;
	int _transEr;
	const char*_transMsg;
};

#define rethrowEr(EX) \
	throw DexelaException(EX,__FUNCTION__);\

#define throwNewEr(MSG,CODE,TRANSER,TRANSMSG) \
	throw DexelaException(MSG,CODE,__LINE__,__FILE__,__FUNCTION__,TRANSER, TRANSMSG);\

