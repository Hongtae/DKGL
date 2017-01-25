//
//  File: Window.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#ifdef _WIN32
#include <Windows.h>
#include "../../Interface/DKWindowInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace Win32
		{
			class Window : public DKWindowInterface
			{
			public:
				Window(DKWindow*);
				~Window(void);

				bool Create(const DKString& title, uint32_t style);
				bool CreateProxy(void* systemHandle);
				bool IsProxy(void) const;
				void UpdateProxy(void);
				void Destroy(void);
				void* PlatformHandle(void) const;
				bool IsValid(void) const;

				void ShowMouse(int deviceId, bool show);
				bool IsMouseVisible(int deviceId) const;
				void HoldMouse(int deviceId, bool hold);
				bool IsMouseHeld(int deviceId) const;

				void Show(void);
				void Hide(void);
				void Activate(void);
				void Minimize(void);

				void SetOrigin(const DKPoint&);
				void Resize(const DKSize&, const DKPoint* optionalOrigin);
				double ContentScaleFactor(void) const;  // logical coords by pixel ratio.

				void SetTitle(const DKString& title);
				DKString Title(void) const;

				void SetMousePosition(int deviceId, const DKPoint& pt);
				DKPoint MousePosition(int deviceId) const;

				void EnableTextInput(int deviceId, bool enable);
				bool IsTextInputEnabled(int deviceId);

			private:
				static LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
				static DKVirtualKey ConvertVKey(int key);
				void UpdateKeyboard(void);
				void UpdateMouse(void);
				void ResetKeyboard(void);
				void ResetMouse(void);

				DKWindow* instance;
				IDropTarget* dropTarget;
				HWND hWnd;

				BYTE keyboardStates[256];
				DKRect windowRect;
				DKRect contentRect;
				double contentScaleFactor;

				DKPoint mousePosition;
				DKPoint holdingMousePosition;

				bool textCompositionMode;
				bool proxyWindow;
				bool activated;
				bool visible;
				bool minimized;
				bool holdMouse;
				bool resizing;
				bool autoResize;

				union {
					uint8_t buttons;
					struct {
						uint8_t button1 : 1;
						uint8_t button2 : 1;
						uint8_t button3 : 1;
						uint8_t button4 : 1;
						uint8_t button5 : 1;
						uint8_t button6 : 1;
						uint8_t button7 : 1;
						uint8_t button8 : 1;
					};
				} mouseButtonDown;
			};
		}
	}
}
#endif // _WIN32
