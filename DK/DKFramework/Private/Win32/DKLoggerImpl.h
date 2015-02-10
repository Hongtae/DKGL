//
//  File: DKLoggerImpl.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include "../../Interface/DKApplicationInterface.h"

using namespace DKFoundation;

namespace DKFramework
{
	struct DKLoggerImpl : public DKLogger
	{
		void Log(const DKString&) override;
	};
}

#endif
