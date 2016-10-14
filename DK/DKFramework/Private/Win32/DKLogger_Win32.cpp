//
//  File: DKApplicationImpl.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <wincon.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>

#include "DKLogger_Win32.h"

namespace DKGL
{
	namespace Private
	{
		void __cdecl CleanupLogWindow(void)
		{
			system("pause");
			FreeConsole();
		}
		HANDLE LogWindowInit(void)
		{
			if (AllocConsole())
			{
				HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
				HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
				HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

				// stdio redirect
				int fin = _open_osfhandle((intptr_t)hIn, _O_TEXT);
				int fout = _open_osfhandle((intptr_t)hOut, _O_TEXT);
				int ferr = _open_osfhandle((intptr_t)hErr, _O_TEXT);

				*stdin = *_fdopen(fin, "r");
				*stdout = *_fdopen(fout, "w");
				*stderr = *_fdopen(ferr, "w");
				setvbuf( stderr, NULL, _IONBF, 0 );

				std::ios::sync_with_stdio();

				atexit(CleanupLogWindow);

				return hOut;
			}
			return NULL;
		}
		HANDLE InitLog(void)
		{
			static HANDLE hLogWindow = LogWindowInit();
			return hLogWindow;
		}
		inline void WriteLog(const char* str)
		{
			HANDLE hLogWindow = InitLog();
			if (hLogWindow)
			{
				DWORD dwWritten = 0;
				WriteConsoleA(hLogWindow, str, (DWORD)strlen(str), &dwWritten, 0);
				return;
			}
			else
				printf(str);
		}
		inline void WriteLog(const wchar_t* str)
		{
			HANDLE hLogWindow = InitLog();
			if (hLogWindow)
			{
				DWORD dwWritten = 0;
				WriteConsoleW(hLogWindow, str, (DWORD)wcslen(str), &dwWritten, 0);
				return;
			}
			else
				printf("%ls",str);
		}
	}
}

using namespace DKGL;

void DKLogger_Win32::Log(const DKString& msg)
{
	::OutputDebugStringW((const wchar_t*)msg);
	Private::WriteLog((const wchar_t*)msg);
}

#endif
