//
//  File: DKLog.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>

#include "DKLog.h"
#include "DKString.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	namespace Private
	{
		static DKSpinLock spinLock;
		static DKObject<DKLogger> defaultLogger = NULL;
	}

	DKGL_API void DKLoggerSet(DKLogger* logger)
	{
		DKCriticalSection<DKSpinLock> guard(Private::spinLock);
		if (Private::defaultLogger)
			Private::defaultLogger->OnUnbind();
		Private::defaultLogger = logger;
		if (Private::defaultLogger)
			Private::defaultLogger->OnBind();
	}
	DKGL_API DKLogger* DKLoggerCurrent(void)
	{
		DKCriticalSection<DKSpinLock> guard(Private::spinLock);
		return Private::defaultLogger;
	}
	DKGL_API bool DKLoggerCompareAndReplace(DKLogger* cmp, DKLogger* repl)
	{
		DKCriticalSection<DKSpinLock> guard(Private::spinLock);
		if (Private::defaultLogger == cmp)
		{
			if (Private::defaultLogger)
				Private::defaultLogger->OnUnbind();
			Private::defaultLogger = repl;
			if (Private::defaultLogger)
				Private::defaultLogger->OnBind();
			return true;
		}
		return false;
	}

	DKGL_API void DKLogInit(DKLogCallbackProc proc)
	{
		struct Logger : public DKLogger
		{
			Logger(DKLogCallbackProc p) : proc(p) {}
			DKLogCallbackProc proc;
			void Log(const DKString& msg) override
			{
				proc(msg);
			}
		};
		if (proc)
			DKLoggerSet(DKObject<Logger>::New(proc));
		else
			DKLoggerSet(NULL);
	}
	DKGL_API void DKLog(const DKString& str)
	{
		Private::spinLock.Lock();
		if (Private::defaultLogger)
		{
			Private::defaultLogger->Log(str);
			Private::spinLock.Unlock();
			return;
		}
		Private::spinLock.Unlock();
		fprintf(stderr, "%ls", (const wchar_t*)str);
	}
	DKGL_API void DKLog(const char* fmt, ...)
	{
		if (fmt == NULL || fmt[0] == '\0')
			return;
		va_list ap;
		va_start(ap, fmt);
		DKLog(DKString::FormatV(fmt, ap));
		va_end(ap);
	}
}
