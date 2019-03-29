//
//  File: DKUtils.cpp
//  Platform: Win32, Linux
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#define _CRT_RAND_S
#include <windows.h>
#include <shlobj.h>
#elif defined(__linux__)
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <fnmatch.h>
#include <iostream>
#include <fstream>
#elif defined(__APPLE__) && defined(__MACH__)
#else
#error "Unknown OS"
#endif

#include "DKUtils.h"

extern "C" DKGL_API const char* DKVersion()
{
	return "DKGL 2.0.0";
}
extern "C" DKGL_API const char* DKCopyright()
{
	return "Copyright (c) 2004-2019 Hongtae Kim. (tiff2766@gmail.com), All rights reserved.";
}

namespace DKFoundation
{
#ifdef _WIN32
	DKGL_API uint32_t DKRandom()
	{
		uint32_t value;
		if (rand_s(&value) == 0)
		{
			return value;
		}
		// rand_s error??
		static struct _Rand_init
		{
			_Rand_init()
			{
				DWORD tick = ::GetTickCount();
				srand((unsigned int)tick);
			}
		} init;

		uint32_t h = rand();
		uint32_t l = rand();
		return ((h << 16) & 0xffff0000) | ( l & 0x0000ffff);
	}
#elif defined(__linux__)
	DKGL_API uint32_t DKRandom()
	{
		static struct InitSeed
		{
			InitSeed() { srandom(time(0)); }
		} init;

		return random();
	}
#endif

#ifdef _WIN32
	// temporary folder for current user.
	DKGL_API DKString DKTemporaryDirectory()
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
	
	DKGL_API DKArray<DKString> DKProcessArguments()
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
	
	DKGL_API DKMap<DKString, DKString> DKProcessEnvironments()
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

	DKGL_API uint32_t DKNumberOfCpuCores()
	{
		static int ncpu = []()->int
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			DWORD numLogicalProcessors = sysInfo.dwNumberOfProcessors;
			DWORD numPhysicalProcessors = numLogicalProcessors;

			DWORD buffSize = 0;
			if (!GetLogicalProcessorInformationEx(RelationProcessorCore, 0, &buffSize) &&
				GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{
				uint8_t* buffer = new uint8_t[buffSize];

				if (GetLogicalProcessorInformationEx(RelationProcessorCore, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)buffer, &buffSize))
				{
					DWORD numCores = 0;
					DWORD offset = 0;
					do
					{
						SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* processorInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)&buffer[offset];
						offset += processorInfo->Size;
						numCores += processorInfo->Processor.GroupCount;
					} while (offset < buffSize);
					if (numCores > 0 && numCores < numLogicalProcessors)
					{
						numPhysicalProcessors = numCores;
					}
				}
				delete[] buffer;
			}
			return numPhysicalProcessors;
		}();

		if (ncpu > 1)
			return ncpu;
		return 1;
	}

	DKGL_API uint32_t DKNumberOfProcessors()
	{
        DWORD numCores = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
        if (!numCores)
        {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            numCores = sysinfo.dwNumberOfProcessors;
        }
        if (numCores > 1)
            return numCores;
		return 1;
	}
#elif defined(__linux__)
	DKGL_API DKString DKTemporaryDirectory()
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
	
	DKGL_API DKArray<DKString> DKProcessArguments()
	{
		DKArray<DKString> args;

		std::string line;
		std::ifstream file("/proc/self/cmdline");
		if (file.is_open())
		{
			while (getline(file, line))
			{
				DKString s(line);
				DKString::StringArray sa = s.SplitByWhitespace();
				while (DKString& str : sa)
				{
					str.TrimWhitespaces();
					if (str.Length() > 0)
						args.Add(str);
				}
			}
			file.close();
		}
		return args;
	}
	
	DKGL_API DKMap<DKString, DKString> DKProcessEnvironments()
	{
		extern char **environ;

		DKMap<DKString, DKString> env;
		for (int i = 0; environ[i]; ++i)
		{
			DKString tmp(environ[i]);
			long x = tmp.Find(L'=');
			if (x > 0)
			{
				DKString key = tmp.Left(x);
				DKString value = tmp.Right(x+1);

				env.Update(key, value);
			}
		}
		return env;
	}

	DKGL_API uint32_t DKNumberOfCpuCores()
	{
		static int ncpu = []()->int
		{
			int count = 0;
			DIR* cpuDir = opendir("/sys/devices/system/cpu"); // linux only
			if (cpuDir)
			{
				const struct dirent* dirEntry;
				while ((dirEntry = readdir(cpuDir)))
				{
					if (fnmatch("cpu[0-9]*", dirEntry->d_name, 0) == 0)
						count++;
				}
				closedir(cpuDir);
				return count;
			}
			return -1;
		}();

		if (ncpu > 1)
			return ncpu;
		return 1;
	}

	DKGL_API uint32_t DKNumberOfProcessors()
	{
		static int ncpu = sysconf(_SC_NPROCESSORS_CONF);

		if (ncpu > 1)
			return ncpu;
		return 1;
	}
#endif
}
