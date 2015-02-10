//
//  File: DKUtils.cpp
//  Platform: Win32, Linux
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#define _CRT_RAND_S
#include <windows.h>
#include <shlobj.h>
#else
#include <time.h>
#include <stdlib.h>
#endif

#include "DKUtils.h"

extern "C" DKLIB_API const char* DKVersion(void)
{
	return "DK 1.0.0";
}
extern "C" DKLIB_API const char* DKCopyright(void)
{
	return "Copyright (c) 2004-2015 Hongtae Kim. (tiff2766@gmail.com), All rights reserved.";
}

namespace DKFoundation
{
#ifdef _WIN32
	DKLIB_API unsigned int DKRandom(void)
	{
		unsigned int value;
		if (rand_s(&value) == 0)
		{
			return value;
		}
		// rand_s error??
		static struct _Rand_init
		{
			_Rand_init(void)
			{
				DWORD tick = ::GetTickCount();
				srand((unsigned int)tick);
			}
		} init;

		unsigned int h = rand();
		unsigned int l = rand();
		return ((h << 16) & 0xffff0000) | ( l & 0x0000ffff);
	}
#elif defined(__linux__)
	DKLIB_API unsigned int DKRandom(void)
	{
		static struct InitSeed
		{
			InitSeed(void) { srandom(time(0)); }
		} init;

		return random();
	}
#endif

#ifdef _WIN32
	// temporary folder for current user.
	DKLIB_API DKString DKTemporaryDirectory(void)
	{
		wchar_t path[MAX_PATH];
		DWORD ret = ::GetTempPathW(MAX_PATH, path);
		if (ret > MAX_PATH || ret == 0)
		{
			// error?
			ITEMIDLIST* pidl;
			if (SHGetSpecialFolderLocation(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, &pidl) == NOERROR &&
				SHGetPathFromIDListW(pidl, path))
			{
				return DKString(path);
			}
			DKERROR_THROW("Cannot get location of temporary directory!");
			return L"C:\\";
		}
		return DKString(path);
	}
	
	DKLIB_API DKArray<DKString> DKProcessArguments(void)
	{
		DKArray<DKString> args;
		int numArgs = 0;
		LPWSTR* str = CommandLineToArgvW(GetCommandLineW(), &numArgs);
		if (str)
		{
			for (int i = 0; i < numArgs; ++i)
			{
				args.Add( str[i] );
			}
			LocalFree(str);
		}
		return args;
	}
	
	DKLIB_API DKMap<DKString, DKString> DKProcessEnvironments(void)
	{
		DKMap<DKString, DKString> env;
		LPWSTR str = (LPWSTR)GetEnvironmentStringsW();
		if (str)
		{
			while (*str)
			{
				/* key=value */
				DKString tmp(str);
				long i = tmp.Find(L'=');
				if (i > 0)
				{
					DKString key = tmp.Left(i);
					DKString val = tmp.Right(i+1);

					env.Update(key, val);
				}
				
				size_t len = wcslen(str);
				str += len + 1;
			}
			FreeEnvironmentStringsW((LPWCH)str);
		}
		return env;
	}
#elif defined(__linux__)
	DKLIB_API DKString DKTemporaryDirectory(void)
	{
		std::initializer_list<const char*> tmpKeys = {"TMPDIR", "TMP", "TEMP", "TEMPDIR"};
		const char* tmpDir = NULL;
		for (const char* k : tmpKeys)
		{
			tmpDir = getenv(k);
			if (tmpDir)
				break;
		}
		if (tmpDir == NULL)
			tmpDir = "/tmp";

		return DKString(tmpDir);
	}
	
	DKLIB_API DKArray<DKString> DKProcessArguments(void)
	{
		return DKArray<DKString>();
	}
	
	DKLIB_API DKMap<DKString, DKString> DKProcessEnvironments(void)
	{
		return DKMap<DKString, DKString>();
	}
#endif
}
