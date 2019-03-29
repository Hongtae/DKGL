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
#include "DKLogger.h"

namespace DKFoundation
{
	DKGL_API void DKLog(DKLogCategory c, const DKString& str)
	{
#ifndef DKGL_DEBUG_ENABLED
		if (c == DKLogCategory::Debug) return;
#endif
		if (!DKLogger::Broadcast(c, str))
			fprintf(stderr, "%ls", (const wchar_t*)str);
	}
	DKGL_API void DKLog(DKLogCategory c, const char* fmt, ...)
	{
#ifndef DKGL_DEBUG_ENABLED
		if (c == DKLogCategory::Debug) return;
#endif
		va_list ap;
		va_start(ap, fmt);
		DKLog(c, DKString::FormatV(fmt, ap));
		va_end(ap);
	}
}
