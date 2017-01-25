//
//  File: AppLogger.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <wincon.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include "AppLogger.h"

using namespace DKFramework;
using namespace DKFramework::Private::Win32;

AppLogger::AppLogger(void)
	: console(NULL)
{

}

AppLogger::~AppLogger(void)
{
	if (console)
	{
		if (!::IsDebuggerPresent())
			system("pause");
		//FreeConsole();
	}
}

void AppLogger::OnBind(void)
{
	if (console == NULL &&
		AllocConsole())
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
		setvbuf(stderr, NULL, _IONBF, 0);

		std::ios::sync_with_stdio();

		console = hOut;
	}
}

void AppLogger::OnUnbind(void)
{
	//if (console)
	//{
	//	FreeConsole();
	//	console = NULL;
	//}
}

void AppLogger::Log(const DKString& msg)
{
	DKString msg2 = DKString::Format("[PID:%u TID:%u] %ls",
		(uint32_t)GetCurrentProcessId(),
		(uint32_t)GetCurrentThreadId(),
		(const wchar_t*)msg);
	if (!msg2.HasSuffix("\n"))
		msg2 += "\n";

	::OutputDebugStringW((const wchar_t*)msg2);
	WriteLog((const wchar_t*)(msg2));
}

void AppLogger::WriteLog(const char* str)
{
	if (console)
	{
		DWORD dwWritten = 0;
		WriteConsoleA(console, str, (DWORD)strlen(str), &dwWritten, 0);
		return;
	}
	else
		printf(str);
}

void AppLogger::WriteLog(const wchar_t* str)
{
	if (console)
	{
		DWORD dwWritten = 0;
		WriteConsoleW(console, str, (DWORD)wcslen(str), &dwWritten, 0);
		return;
	}
	else
		printf("%ls", str);
}
#endif // _WIN32