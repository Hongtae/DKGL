//
//  File: DKWindow_Win32.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include "../../Interface/DKWindowInterface.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		class DKWindow_Win32 : public DKWindowInterface
		{
		public:
			DKWindow_Win32(class DKWindow* window);
			~DKWindow_Win32(void);
			bool Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style);
			bool CreateProxy(void* systemHandle);
			void UpdateProxy(void);
			bool IsProxy(void) const;
			void Destroy(void);
			void* PlatformHandle(void) const		{ return windowHandle; }

			void ShowMouse(int deviceId, bool bShow);
			bool IsMouseVisible(int deviceId) const;
			void HoldMouse(int deviceId, bool bHold);
			bool IsMouseHeld(int deviceId) const;

			void Show(void);
			void Hide(void);
			void Activate(void);
			void Minimize(void);

			void Resize(const DKSize&, const DKPoint* pt);
			void SetOrigin(const DKPoint&);

			DKSize ContentSize(void) const			{ return contentSize; }
			DKPoint Origin(void) const				{ return windowOrigin; }
			double ContentScaleFactor(void) const	{ return 1.0; }

			void SetTitle(const DKString& title);
			DKString Title(void) const;
			void SetMousePosition(int deviceId, const DKPoint& pt);
			DKPoint MousePosition(int deviceId) const;
			void EnableTextInput(int deviceId, bool bTextInput);
			bool IsValid(void) const;

			LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			HWND windowHandle;
			DKWindow* ownerWindow;
			DWORD baseThreadId;			// thread-id where window created.

			void UpdateKeyboard(void);
			void UpdateMouse(void);
			void ResetKeyboard(void);
			void ResetMouse(void);

			bool isTextInputMode;
			bool isActive;
			bool isVisible;
			bool isMinimized;
			bool isMouseHeld;
			bool isProxyWindow;
			bool isMouseLButtonDown;
			bool isMouseRButtonDown;
			bool isMouseWButtonDown;
			bool isResizing;

			DKSize contentSize;
			DKPoint windowOrigin;
			BYTE savedKeyboardState[256];		// key states
			DKPoint mousePosition;
			DKPoint mousePositionHeld;		// result of SetCursorPos() with hold-position state.

			static DKVirtualKey ConvertVKey(int key);
			DKPoint& ConvertCoordinateOrigin(DKPoint& pt) const;
		};
	}
}


#endif // ifdef WIN32

