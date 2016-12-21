//
//  File: DKLog.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"

namespace DKFoundation
{
	typedef void(*DKLogCallbackProc)(const DKString &);

	/// @brief a logger class.
	/// you can sublcass DKLogger to handle log text.
	struct DKLogger
	{
		virtual ~DKLogger(void) {}
		virtual void Log(const DKString&) = 0;

		virtual void OnBind(void) {}
		virtual void OnUnbind(void) {}
	};

	DKGL_API void DKLoggerSet(DKLogger*);
	DKGL_API DKLogger* DKLoggerCurrent(void);

	/// if current logger is 'cmp', then set 'repl' to current logger.
	DKGL_API bool DKLoggerCompareAndReplace(DKLogger* cmp, DKLogger* repl);

	DKGL_API void DKLogInit(DKLogCallbackProc proc);	///< deprecated

	DKGL_API void DKLog(const DKString& str);
	DKGL_API void DKLog(const char* fmt, ...);
}

#ifdef DKGL_DEBUG_ENABLED
#define DKLOG_DEBUG(...)	DKLog(__VA_ARGS__)
#else
#define DKLOG_DEBUG(...)	(void)0
#endif
