//
//  File: AppEventLoop.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include "AppEventLoop.h"

using namespace DKFramework;
using namespace DKFramework::Private::Win32;

AppEventLoop::AppEventLoop(DKApplication* app)
	: appInstance(app)
	, threadId(0)
	, running(false)
{

}

AppEventLoop::~AppEventLoop(void)
{

}

bool AppEventLoop::Run(void)
{
	if (BindThread())
	{
		running = true;
		threadId = GetCurrentThreadId();
		DKApplicationInterface::AppInitialize(appInstance);

		UINT_PTR timerId = 0;
		MSG	msg;
		BOOL ret;

		PostMessageW(NULL, WM_NULL, 0, 0); // To process first enqueued events.

		// Note: If user calls the PostQuitMessage(),
		//       all AppEventLoops will be terminated and will not be usable.
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
			while (this->running && Dispatch()) {}
			if (this->running)
			{
				double intv = PendingEventInterval();
				if (intv >= 0.0)
				{
					// install timer for next pending event.
					UINT elapse = static_cast<UINT>(intv * 1000);
					timerId = SetTimer(NULL, timerId, elapse, NULL);
				}
			}
			else
				break;
		}

		if (timerId)
			KillTimer(NULL, timerId);

		DKApplicationInterface::AppFinalize(appInstance);

		threadId = 0;
		running = false;
		UnbindThread();
		return true;
	}
	return false;
}

void AppEventLoop::Stop(void)
{
	// Don't use PostQuitMessage!
	if (threadId)
	{
		this->Post(DKFunction([this]() {
			this->running = false;
		})->Invocation());
		PostThreadMessageW(threadId, WM_NULL, 0, 0);
	}
}

#endif // _WIN32
