//
//  File: DKLogger_Win32.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include "../../Interface/DKApplicationInterface.h"

namespace DKGL
{
	struct DKLogger_Win32 : public DKLogger
	{
		void Log(const DKString&) override;
	};
}

#endif
