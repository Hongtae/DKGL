//
//  File: Application.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <Sddl.h>
#include <shlobj.h>

#include "Application.h"

#pragma comment(lib, "version.lib")

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			struct MonitorHandleWithIndex
			{
				HMONITOR hMonitor;
				int index;
			};
			static BOOL CALLBACK EnumMonitorProc(HMONITOR hMonitor, HDC hDC, LPRECT lprc, LPARAM pData)
			{
				MonitorHandleWithIndex* mi = reinterpret_cast<MonitorHandleWithIndex*>(pData);
				if (mi->index == 0)
				{
					mi->hMonitor = hMonitor;
					return FALSE;
				}
				mi->hMonitor = NULL;
				mi->index--;
				return TRUE;
			}
			static HMONITOR GetMonitorHandleByIndex(int index)
			{
				if (index >= 0)
				{
					MonitorHandleWithIndex target = { NULL, index };
					if (EnumDisplayMonitors(0, 0, EnumMonitorProc, (LPARAM)&target))
					{
						return target.hMonitor;
					}
				}
				return NULL;
			}
			static bool GetOSVersionFromKernel32DLL(DKString& name, DKString& version)
			{
				bool result = false;
				DWORD bufferSize = GetFileVersionInfoSizeW(L"kernel32.dll", 0);
				if (bufferSize > 0)
				{
					// 1033-1200 / English-UTF16 / Hex: 409-4b0 
					LPCWSTR subBlockPName = L"\\StringFileInfo\\040904b0\\ProductName";
					LPCWSTR subBlockPVer = L"\\StringFileInfo\\040904b0\\ProductVersion";
					DKString prdName, prdVersion;

					void* buffer = DKMalloc(bufferSize);
					do
					{
						if (!GetFileVersionInfoW(L"kernel32.dll", NULL, bufferSize, buffer))
							break;

						UINT len = 0; // for string value, number of characters (not bytes)
						void* ptr = 0;
						if (!VerQueryValueW(buffer, subBlockPName, &ptr, &len))
							break;

						prdName = DKString(ptr, len * sizeof(wchar_t), DKStringEncoding::UTF16);
						if (!VerQueryValueW(buffer, subBlockPVer, &ptr, &len))
							break;

						prdVersion = DKString(ptr, len * sizeof(wchar_t), DKStringEncoding::UTF16);

						name = std::move(prdName);
						version = std::move(prdVersion);
						result = true;
					} while (0);
					DKFree(buffer);
				}
				return result;
			}
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private::Win32;

DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app, int argc, char* argv[])
{
	return new Application(app);
}

Application::Application(DKApplication* app)
	: mainLoop(app)
{
}

Application::~Application(void)
{
}

DKEventLoop* Application::EventLoop(void)
{
	return &mainLoop;
}

DKLogger* Application::DefaultLogger(void)
{
	static AppLogger logger;
	return &logger;
}

DKString Application::DefaultPath(SystemPath sp)
{
	wchar_t path[MAX_PATH];
	auto GetWindowsFolder = [&path](std::initializer_list<int> folders)->DKString
	{
		ITEMIDLIST* pidl;
		for (int fid : folders)
		{
			if (SHGetSpecialFolderLocation(NULL, fid | CSIDL_FLAG_CREATE, &pidl) == NOERROR &&
				SHGetPathFromIDListW(pidl, path))
			{
				return DKString(path);
			}
		}
		return L"C:\\";
	};

	switch (sp)
	{
	case SystemPath::SystemRoot:		// system root, (boot volume)
		::GetWindowsDirectoryW(path, MAX_PATH);
		path[2] = NULL;
		return DKString(path);
		break;
	case SystemPath::AppRoot:			// root directory of executable
	case SystemPath::AppResource:
	case SystemPath::AppExecutable:
		for (DWORD len = ::GetModuleFileNameW(::GetModuleHandle(NULL), path, MAX_PATH); len > 0; len--)
		{
			if (path[len - 1] == L'\\')
			{
				path[len - 1] = L'\0';
				return DKString(path);
			}
		}
		return L"C:\\";
		break;
	case SystemPath::AppData:			// application's data
		return GetWindowsFolder({ CSIDL_APPDATA, CSIDL_LOCAL_APPDATA, CSIDL_COMMON_APPDATA });
		break;
	case SystemPath::UserHome:		// user's home dir
		return GetWindowsFolder({ CSIDL_PROFILE, CSIDL_MYDOCUMENTS, CSIDL_DESKTOPDIRECTORY });
		break;
	case SystemPath::UserDocuments:	// user's documents dir
		return GetWindowsFolder({ CSIDL_MYDOCUMENTS, CSIDL_PROFILE, CSIDL_DESKTOPDIRECTORY });
		break;
	case SystemPath::UserPreferences:	// user's setting(config) dir
		return GetWindowsFolder({ CSIDL_LOCAL_APPDATA, CSIDL_APPDATA, CSIDL_PROFILE });
		break;
	case SystemPath::UserCache:		// user's cache dir
		return GetWindowsFolder({ CSIDL_LOCAL_APPDATA, CSIDL_APPDATA, CSIDL_PROFILE });
		break;
	case SystemPath::UserTemp:		// user's temporary dir
		if (1)
		{
			DWORD ret = ::GetTempPathW(MAX_PATH, path);
			if (ret > MAX_PATH || ret == 0)
				return GetWindowsFolder({ CSIDL_PROFILE, CSIDL_MYDOCUMENTS, CSIDL_DESKTOPDIRECTORY });
			return DKString(path);
		}
		break;
	}
	return path;
}

DKString Application::ProcessInfoString(ProcessInfo pi)
{
	switch (pi)
	{
	case ProcessInfo::HostName:
		return []()->DKString
		{
			wchar_t comName[1024];
			DWORD len = 1024;
			if (GetComputerNameW(comName, &len))
				return comName;

			return L"Unknown";
		}();
		break;
	case ProcessInfo::OsName:
		if (1)
		{
			DKString ret, name, version;
			if (GetOSVersionFromKernel32DLL(name, version))
			{
				ret = name + L" " + version;
			}
			else
			{
				ret = DKString("Microsoft Windows");

				OSVERSIONINFOEXW ver;
				memset(&ver, 0, sizeof(OSVERSIONINFOEXW));
				ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
				if (GetVersionExW((LPOSVERSIONINFOW)&ver))
				{
					ret += DKString::Format(" %d.%d", ver.dwMajorVersion, ver.dwMinorVersion);
					if (ver.szCSDVersion[0])
						ret += DKString::Format(" %s", ver.szCSDVersion);
					ret += DKString::Format(" (build:%d)", ver.dwBuildNumber);
				}
			}
			return ret;
		}
		break;
	case ProcessInfo::UserName:
		return []()->DKString
		{
			wchar_t userName[1024];
			DWORD len = 1024;
			if (GetUserNameW(userName, &len))
				return userName;

			return L"";
		}();
		break;
	case ProcessInfo::ModulePath:
		return []()->DKString
		{
			wchar_t path[MAX_PATH];
			::GetModuleFileNameW(::GetModuleHandle(NULL), path, MAX_PATH);
			return DKString(path);
		}();
		break;
	}
	return "";
}

DKObject<DKData> Application::LoadResource(const DKString& res, DKAllocator& alloc)
{
	DKObject<DKData> ret = NULL;
	HRSRC hRes = ::FindResourceW(0, (const wchar_t*)res, (LPCWSTR)RT_RCDATA);
	if (hRes)
	{
		DWORD contentSize = ::SizeofResource(0, hRes);
		if (contentSize > 0)
		{
			HGLOBAL hMem = ::LoadResource(0, hRes);
			if (hMem)
			{
				LPVOID data = ::LockResource(hMem);
				if (data)
					ret = DKBuffer::Create(data, contentSize, alloc).SafeCast<DKData>();
			}
		}
	}
	return ret;
}

DKObject<DKData> Application::LoadStaticResource(const DKString& res)
{
	DKObject<DKData> ret = NULL;
	HRSRC hRes = ::FindResourceW(0, (const wchar_t*)res, (LPCWSTR)RT_RCDATA);
	if (hRes)
	{
		DWORD contentSize = ::SizeofResource(0, hRes);
		if (contentSize > 0)
		{
			HGLOBAL hMem = ::LoadResource(0, hRes);
			if (hMem)
			{
				LPVOID data = LockResource(hMem);
				ret = DKData::StaticData(data, contentSize);
			}
		}
	}
	return ret;
}

DKRect Application::DisplayBounds(int displayId) const
{
	MONITORINFO	mi;
	memset(&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);

	RECT& rcMonitor = mi.rcMonitor;
	HMONITOR hMonitor = GetMonitorHandleByIndex(displayId);

	if (hMonitor && GetMonitorInfo(hMonitor, &mi))
		return DKRect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);
	else if (displayId == 0)
	{
		if (GetWindowRect(GetDesktopWindow(), &rcMonitor))
			return DKRect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);
	}
	return DKRect(0, 0, 0, 0);
}

DKRect Application::ScreenContentBounds(int displayId) const
{
	MONITORINFO	mi;
	memset(&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);

	RECT& rcWork = mi.rcWork;
	HMONITOR hMonitor = GetMonitorHandleByIndex(displayId);

	if (hMonitor && GetMonitorInfo(hMonitor, &mi))
		return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
	else if (displayId == 0)
	{
		if (SystemParametersInfoW(SPI_GETWORKAREA, NULL, &rcWork, NULL))
			return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
		else if (GetClientRect(GetDesktopWindow(), &rcWork))
			return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
	}
	return DKRect(0, 0, 0, 0);
}
#endif // _WIN32
