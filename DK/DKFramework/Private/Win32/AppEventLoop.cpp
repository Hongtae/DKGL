//
//  File: AppEventLoop.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2019 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include "AppEventLoop.h"

namespace DKFramework::Private::Win32
{
    // Note: 
    // We are using Keyboard-Hook instead of RawInput(WM_INPUT) to
    // disable the pop-up menu by pressing Win-Key.
    // Since RawInput works on application-wide, RawInput can conflict
    // with other frameworks.
    static HHOOK keyboardHook = NULL;
    static bool disableWindowKey = true;

    // number of active windows.
    // disable Window-Key only if numActiveWindows > 0.
    DKAtomicNumber32 numActiveWindows = 0;

    // Hook window-key for prevent a window being disabled when window-key pressed.
    // To use window-key, save key-state with SetKeyboardState().
    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        bool hook = disableWindowKey && numActiveWindows > 0;
        if (nCode == HC_ACTION && hook)
        {
            KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;
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
}
using namespace DKFramework;
using namespace DKFramework::Private::Win32;

AppEventLoop::AppEventLoop(DKApplication* app)
	: appInstance(app)
	, threadId(0)
	, running(false)
{
}

AppEventLoop::~AppEventLoop()
{
}


bool AppEventLoop::Run()
{
	if (threadId == 0 && running == false)
	{
		running = true;
		threadId = GetCurrentThreadId();
		DKApplicationInterface::AppInitialize(appInstance);

		if (::IsDebuggerPresent() == 0)
		{
			if (keyboardHook)
			{
				DKLogE("Error: Keyboard hook state invalid. (already installed?)\n");
				::UnhookWindowsHookEx(keyboardHook);
				keyboardHook = NULL;
			}

			bool installHook = false;
			const DKString disableWinKeyConfigKey = L"DisableWindowKey";
			DKPropertySet& config = DKPropertySet::SystemConfig();
			if (config.HasValue(disableWinKeyConfigKey) &&
				config.Value(disableWinKeyConfigKey).ValueType() == DKVariant::TypeInteger)
			{
				installHook = config.Value(disableWinKeyConfigKey).Integer() != 0;
			}

			if (installHook)
			{
				keyboardHook = ::SetWindowsHookExW(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandleW(0), 0);
				if (keyboardHook == NULL)
				{
					DKLogE("ERROR: SetWindowsHookEx Failed.\n");
					//	DKERROR_THROW("Failed to install hook procedure");
					//	return 0;
				}
			}
		}

        // Setup thread DPI
        static DPI_AWARENESS_CONTEXT(*setThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT) = []
        {
            HMODULE module = ::LoadLibraryW(L"User32.dll");
            if (module)
                return reinterpret_cast<DPI_AWARENESS_CONTEXT(*)(DPI_AWARENESS_CONTEXT)>(::GetProcAddress(module, "SetThreadDpiAwarenessContext"));
            return (DPI_AWARENESS_CONTEXT(*)(DPI_AWARENESS_CONTEXT))nullptr;
        }();
        if (setThreadDpiAwarenessContext)
            setThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

		UINT_PTR timerId = 0;
		MSG	msg;
		BOOL ret;

		PostMessageW(NULL, WM_NULL, 0, 0); // To process first enqueued events.

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
			if (this->running)
			{
				while (this->running && Execute()) {}
				if (this->running)
				{
					double intv = NextDispatchInterval();
					if (intv >= 0.0)
					{
						// install timer for next pending event.
						UINT elapse = static_cast<UINT>(intv * 1000);
						timerId = SetTimer(NULL, timerId, elapse, NULL);
					}
					else if (timerId)
					{
						KillTimer(NULL, timerId);
						timerId = 0;
					}
				}
				else
					PostQuitMessage(0);
			}
		}

		if (timerId)
			KillTimer(NULL, timerId);

		DKApplicationInterface::AppFinalize(appInstance);

		if (keyboardHook)
			::UnhookWindowsHookEx(keyboardHook);
		keyboardHook = NULL;

		threadId = 0;
		running = false;
		return true;
	}
	return false;
}

void AppEventLoop::Stop()
{
		// Don't use PostQuitMessage here
	if (threadId)
	{
		this->Submit(DKFunction([this]() {
			this->running = false;
		})->Invocation(), 0);
		PostThreadMessageW(threadId, WM_NULL, 0, 0);
	}
}

DKObject<DKDispatchQueue::ExecutionState> AppEventLoop::Submit(DKOperation* op, double delay) 
{
	DKObject<DKDispatchQueue::ExecutionState> es = DKDispatchQueue::Submit(op, delay);
	if (this->threadId)
		PostThreadMessageW(this->threadId, WM_NULL, 0, 0);
	return es;
}

bool AppEventLoop::IsRunning() const
{
	return this->running;
}

bool AppEventLoop::IsDispatchThread() const
{
	return GetCurrentThreadId() == threadId;
}

#endif // _WIN32
