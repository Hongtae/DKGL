﻿//
//  File: DKApplication_Win32.h
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

#ifdef GetUserName
#undef GetUserName
#endif

namespace DKGL
{
	namespace Private
	{
		class DKApplication_Win32 : public DKApplicationInterface
		{
		public:
			DKApplication_Win32(DKApplication* app);
			~DKApplication_Win32(void);

			int Run(DKArray<char*>& args);
			void Terminate(int exitCode);

			// perform operation on main-thread. (main thread is Running-RunLoop)
			// operation could be processed through OnInitialize() to OnTerminate().
			void PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone);

			DKLogger& DefaultLogger(void);
			DKString EnvironmentPath(SystemPath);
			DKString ModulePath(void);

			DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc);
			DKObject<DKData> LoadStaticResource(const DKString& res);

			DKRect DisplayBounds(int displayId) const;
			DKRect ScreenContentBounds(int displayId) const;

			DKString HostName(void) const;
			DKString OSName(void) const;
			DKString UserName(void) const;

			DKApplication* mainApp;
			DWORD threadId;
		};
	}
}

#endif
