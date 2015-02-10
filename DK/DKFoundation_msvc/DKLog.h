//
//  File: DKLog.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

typedef void(*DKLogCallbackProc)(const DKFoundation::DKString &);

////////////////////////////////////////////////////////////////////////////////
// DKLogger
// a logger class.
// you can sublcass DKLogger to handle log text.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	struct DKLogger
	{
		virtual ~DKLogger(void) {}
		virtual void Log(const DKString&) = 0;
	};

	DKLIB_API void DKLoggerSet(DKLogger*);
	DKLIB_API DKLogger* DKLoggerCurrent(void);

	// if current logger is 'cmp', then set 'repl' to current logger.
	DKLIB_API bool DKLoggerCompareAndReplace(DKLogger* cmp, DKLogger* repl);

	DKLIB_API void DKLogInit(DKLogCallbackProc proc);	// deprecated

	DKLIB_API void DKLog(const DKFoundation::DKString& str);
	DKLIB_API void DKLog(const char* fmt, ...);
}

#ifdef DKLIB_DEBUG_ENABLED
#define DKLOG_DEBUG(...)	DKFoundation::DKLog(__VA_ARGS__)
#else
#define DKLOG_DEBUG(...)	(void)0
#endif
