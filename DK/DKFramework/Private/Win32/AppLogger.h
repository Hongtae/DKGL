//
//  File: AppLogger.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#ifdef _WIN32
#include <Windows.h>
#include "../../Interface/DKApplicationInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			class AppLogger : public DKLogger
			{
			public:
				AppLogger(void);
				~AppLogger(void);

				void OnBind(void) override;
				void OnUnbind(void) override;

				void Log(const DKString&) override;

				void WriteLog(const char* str); // MBCS, not UTF-8
				void WriteLog(const wchar_t* str);
				HANDLE console;
			};
		}
	}
}
#endif // _WIN32
