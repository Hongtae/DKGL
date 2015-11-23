//
//  File: DKApplicationImpl.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <Sddl.h>
#include <shlobj.h>

#include "../../DKApplication.h"
#include "DKApplicationImpl.h"
#include "DKWindowImpl.h"
#include "DKLoggerImpl.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		// operation list for RunOperationOnMainThread()
		// invoked by ForegroundIdleProc Hook of main thread.
		static DKCondition mainThreadOperationCond;
		static DKArray<DKObject<DKOperation>> mainThreadOperations;

		static void PerformMainThreadOperations(void)
		{
			DKCriticalSection<DKCondition> guard(mainThreadOperationCond);
			for (DKOperation* op : mainThreadOperations)
			{
				op->Perform();
			}
			mainThreadOperations.Clear();
			mainThreadOperationCond.Broadcast();
		}
		static HHOOK idleHook = NULL;
		static LRESULT CALLBACK ForegroundIdleProc(int code, WPARAM wParam, LPARAM lParam)
		{
			if (code == HC_ACTION)
				PerformMainThreadOperations();
			return CallNextHookEx(idleHook, code, wParam, lParam);
		}


		static HHOOK keyboardHook = NULL;
		static bool systemKeyDisabled = false;

		static DKSet<DKWindow*> activatedWindowsSet;
		static DKSpinLock activatedWindowsLock;

		// Hook window-key for prevent a window being disabled when window-key pressed.
		// To use window-key, save key-state with SetKeyboardState().
		static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
		{
			if (nCode == HC_ACTION && systemKeyDisabled)
			{
				KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *)lParam;
				if (pkbhs->vkCode == VK_LWIN || pkbhs->vkCode == VK_RWIN)
				{
					static BYTE keyState[256];
					// To use window-key as regular key, update keyState.
					if (wParam == WM_KEYDOWN)
					{
						GetKeyboardState(keyState);
						keyState[pkbhs->vkCode] = 0x80;
						SetKeyboardState(keyState);
					}
					else if (wParam == WM_KEYUP)
					{
						GetKeyboardState(keyState);
						keyState[pkbhs->vkCode] = 0x00;
						SetKeyboardState(keyState);
					}
					return 1;
				}
			}
			return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
		}

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
				MonitorHandleWithIndex target;
				if (EnumDisplayMonitors(0, 0, EnumMonitorProc, (LPARAM)&target))
				{
					return target.hMonitor;
				}
			}
			return NULL;
		}

		static bool IsSystemKeyEnabled(void)
		{
			return !systemKeyDisabled;
		}

		static bool EnableSystemKey(DKWindow* window, bool enable)
		{
			DKCriticalSection<DKSpinLock> guard(activatedWindowsLock);

			if (enable)
				activatedWindowsSet.Remove(window);
			else
				activatedWindowsSet.Insert(window);

			systemKeyDisabled = activatedWindowsSet.Count() > 0;

			if (keyboardHook == NULL)
			{
				DKLog(
					"[DKApplication] Warning: Cannot handle keyboard. (State changed. but will not work!)\n"
					"[DKApplication] Warning: This feature disabled when processor is being debugged.\n");
			}
			else
			{
				if (systemKeyDisabled)
					DKLog("[DKApplication] System Key Disabled.\n");
				else
					DKLog("[DKApplication] System Key Enabled.\n");
			}

			return !systemKeyDisabled;
		}

		LRESULT CALLBACK ApplicationEventProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			if (hWnd)		// window message
			{
				DKWindowImpl* pWindow = (DKWindowImpl*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
				if (pWindow && pWindow->PlatformHandle() == hWnd)
				{
					return pWindow->WindowProc(uMsg, wParam, lParam);
				}
				else
				{
					return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
				}
			}
			else if (DKApplication::Instance())		// thread message
			{
				DKLog("Application Event: %x\n", uMsg);
			}
			return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}
}

using namespace DKFramework;
using namespace DKFramework::Private;

////////////////////////////////////////////////////////////////////////////////
// DKApplicationImpl implementation
DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app)
{
	return new DKApplicationImpl(app);
}

DKApplicationImpl::DKApplicationImpl(DKApplication* app)
	: mainApp(app)
	, threadId(0)
{
}

DKApplicationImpl::~DKApplicationImpl(void)
{
}

int DKApplicationImpl::Run(DKArray<char*>& args)
{
	// 2011-07-05: If keyboard hook installed, debugger become extremely slow.
	//             Don't install hook if debugger has been attached.

	// 2013-12-26: Don't use keyboard hook at all.
	if ( 0 && ::IsDebuggerPresent() == 0)
	{
		// install keyboard hook
		if (keyboardHook)
		{
			DKLog("Error: Keyboard hook state invalid. (already installed?)\n");
			//	DKERROR_THROW("Failed to install hook procedure");
			//	return 0;
		}
		keyboardHook = ::SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandleW(0), 0);
		if (keyboardHook == NULL)
		{
			DKLog("ERROR: SetWindowsHookEx Failed.\n");
			//	DKERROR_THROW("Failed to install hook procedure");
			//	return 0;
		}
	}

	threadId = ::GetCurrentThreadId();

	// idle hooking
	idleHook = ::SetWindowsHookExW(WH_FOREGROUNDIDLE, ForegroundIdleProc, NULL, threadId);

	AppInitialize(mainApp);

	// before entering main-loop, process operations.
	PerformMainThreadOperations();

	MSG	msg;
	BOOL ret;
	while ((ret = GetMessageW(&msg, NULL, 0, 0)) != 0)
	{
		if (ret == -1)
		{
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		PerformMainThreadOperations();
	}

	PerformMainThreadOperations();

	AppFinalize(mainApp);

	threadId = 0;

	if (idleHook)
		::UnhookWindowsHookEx(idleHook);

	if (keyboardHook)
		::UnhookWindowsHookEx(keyboardHook);
	keyboardHook = NULL;

	// process operations
	PerformMainThreadOperations();

	// process messages to clear queue.
	// queue must be cleared before entering new run-loop.
	while (::PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return static_cast<int>(msg.wParam);
}

void DKApplicationImpl::PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone)
{
	if (op == NULL)
		return;

	if (waitUntilDone && threadId == ::GetCurrentThreadId())
	{
		op->Perform();
		return;
	}

	struct MainThreadOperation : public DKOperation
	{
		void Perform(void) const
		{
			op->Perform();
			processed = true;
		}
		DKObject<DKOperation> op;
		mutable bool processed;
	};

	DKObject<MainThreadOperation> op2 = DKObject<MainThreadOperation>::New();
	op2->op = op;
	op2->processed = false;

	DKCriticalSection<DKCondition> guard(mainThreadOperationCond);
	mainThreadOperations.Add(op2.SafeCast<DKOperation>());

	PostThreadMessage(threadId, WM_NULL, 0, 0);

	if (waitUntilDone)
	{
		while (op2->processed == false)
			mainThreadOperationCond.Wait();
	}
}

void DKApplicationImpl::Terminate(int exitCode)
{
	::PostThreadMessageW(threadId, WM_QUIT, static_cast<WPARAM>(exitCode), 0);
}

DKLogger& DKApplicationImpl::DefaultLogger(void)
{
	static DKLoggerImpl logger;
	return logger;
}

DKString DKApplicationImpl::EnvironmentPath(SystemPath s)
{
	wchar_t path[MAX_PATH];
	auto GetWindowsFolder = [&path](std::initializer_list<int> folders)-> DKString
	{
		ITEMIDLIST* pidl;
		for (int fid : folders)
		{
			if (SHGetSpecialFolderLocation(NULL, fid | CSIDL_FLAG_CREATE, &pidl) == NOERROR &&
				SHGetPathFromIDListW( pidl, path))
			{
				return DKString(path);
			}
		}
		return L"C:\\";
	};

	switch (s)
	{
		case SystemPath::SystemPathSystemRoot:		// system root, (boot volume)
			::GetWindowsDirectoryW(path, MAX_PATH);
			path[2] = NULL;
			return DKString(path);
			break;
		case SystemPath::SystemPathAppRoot:			// root directory of executable
		case SystemPath::SystemPathAppResource:
		case SystemPath::SystemPathAppExecutable:
			for (DWORD len = ::GetModuleFileNameW(::GetModuleHandle(NULL), path, MAX_PATH); len > 0 ; len--)
			{
				if (path[len-1] == L'\\')
				{
					path[len-1] = L'\0';
					return DKString(path);
				}
			}
			return L"C:\\";
			break;
		case SystemPath::SystemPathAppData:			// application's data
			return GetWindowsFolder({CSIDL_APPDATA,CSIDL_LOCAL_APPDATA,CSIDL_COMMON_APPDATA});
			break;
		case SystemPath::SystemPathUserHome:		// user's home dir
			return GetWindowsFolder({CSIDL_PROFILE,CSIDL_MYDOCUMENTS,CSIDL_DESKTOPDIRECTORY});
			break;
		case SystemPath::SystemPathUserDocuments:	// user's documents dir
			return GetWindowsFolder({CSIDL_MYDOCUMENTS,CSIDL_PROFILE,CSIDL_DESKTOPDIRECTORY});
			break;
		case SystemPath::SystemPathUserPreferences:	// user's setting(config) dir
			return GetWindowsFolder({CSIDL_LOCAL_APPDATA,CSIDL_APPDATA,CSIDL_PROFILE});
			break;
		case SystemPath::SystemPathUserCache:		// user's cache dir
			return GetWindowsFolder({CSIDL_LOCAL_APPDATA,CSIDL_APPDATA,CSIDL_PROFILE});
			break;
		case SystemPath::SystemPathUserTemp:		// user's temporary dir
			{
				DWORD ret = ::GetTempPathW(MAX_PATH, path);
				if (ret > MAX_PATH || ret == 0)
					return GetWindowsFolder({CSIDL_PROFILE,CSIDL_MYDOCUMENTS,CSIDL_DESKTOPDIRECTORY});
				return DKString(path);
			}
			break;
	}
	return path;
}

DKString DKApplicationImpl::ModulePath(void)
{
	wchar_t path[MAX_PATH];
	::GetModuleFileNameW(::GetModuleHandle(NULL), path, MAX_PATH);
	return DKString(path);
}

DKObject<DKData> DKApplicationImpl::LoadResource(const DKString& res, DKAllocator& alloc)
{
	DKObject<DKData> ret = NULL;
	HRSRC hRes = ::FindResourceW(0, (const wchar_t*)res, (LPCWSTR)RT_RCDATA);
	if (hRes)
	{
		int contentSize = ::SizeofResource(0, hRes);
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

DKObject<DKData> DKApplicationImpl::LoadStaticResource(const DKString& res)
{
	DKObject<DKData> ret = NULL;
	HRSRC hRes = ::FindResourceW(0, (const wchar_t*)res, (LPCWSTR)RT_RCDATA);
	if (hRes)
	{
		int contentSize = ::SizeofResource(0, hRes);
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

DKRect DKApplicationImpl::DisplayBounds(int displayId) const
{
	MONITORINFO	mi;
	memset(&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);

	RECT& rcMonitor = mi.rcMonitor;
	HMONITOR hMonitor = GetMonitorHandleByIndex(displayId);

	if (hMonitor && GetMonitorInfo(hMonitor, &mi))
		return DKRect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);
	else if (GetWindowRect(GetDesktopWindow(), &rcMonitor))
		return DKRect(rcMonitor.left, rcMonitor.top, rcMonitor.right - rcMonitor.left, rcMonitor.bottom - rcMonitor.top);

	return DKRect(0,0,0,0);
}

DKRect DKApplicationImpl::ScreenContentBounds(int displayId) const
{
	MONITORINFO	mi;
	memset(&mi, 0, sizeof(MONITORINFO));
	mi.cbSize = sizeof(MONITORINFO);

	RECT& rcWork = mi.rcWork;
	HMONITOR hMonitor = GetMonitorHandleByIndex(displayId);

	if (hMonitor && GetMonitorInfo(hMonitor, &mi))
		return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
	else if (SystemParametersInfoW(SPI_GETWORKAREA, NULL, &rcWork, NULL))
		return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);
	else if (GetClientRect(GetDesktopWindow(), &rcWork))
		return DKRect(rcWork.left, rcWork.top, rcWork.right - rcWork.left, rcWork.bottom - rcWork.top);

	return DKRect(0,0,0,0);
}

DKString DKApplicationImpl::HostName(void) const
{
	wchar_t comName[1024];
	DWORD len = 1024;
	if (GetComputerNameW(comName, &len))
		return comName;

	return L"Unknown";
}

DKString DKApplicationImpl::OSName(void) const
{
	DKString ret("Microsoft Windows");

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
	return ret;
}

DKString DKApplicationImpl::UserName(void) const
{
	wchar_t userName[1024];
	DWORD len = 1024;
	if (GetUserNameW(userName, &len))
		return userName;

	return L"";
}

#endif
