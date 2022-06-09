//
//  File: Window.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include <ShellScalingAPI.h>
#include "Window.h"
#include "DropTarget.h"
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "Shcore.lib")

// Window Class
#define WNDCLASS_DKWINDOW				L"_DK_WndClass"

// TIMER ID
#define TIMERID_UPDATEKEYBOARDMOUSE		10

// WINDOW MESSAGE
#define WM_DKWINDOW_SHOWCURSOR				(WM_USER + 0x1175)
#define WM_DKWINDOW_UPDATEMOUSECAPTURE		(WM_USER + 0x1180)

namespace DKFoundation::Private
{
    DKString GetWin32ErrorString(DWORD dwError); // defined DKError.cpp
}
using namespace DKFoundation::Private;

namespace DKFramework::Private::Win32
{
    // declared in AppEventLoop.cpp
    extern DKAtomicNumber32 numActiveWindows;

	float DPIScaleForWindow(HWND hwnd)
	{
		static UINT (WINAPI *getDpiForWindow)(HWND) = []
		{
			HMODULE module = ::LoadLibraryW(L"User32.dll");
			if (module)
				return reinterpret_cast<UINT(WINAPI*)(HWND)>(::GetProcAddress(module, "GetDpiForWindow"));
			return (UINT(WINAPI*)(HWND))nullptr;
		}();

        float scale = 1.0f;
		if (getDpiForWindow)
		{
			UINT dpi = getDpiForWindow(hwnd);
			if (dpi)
				scale = float(dpi) / 96.0f;
		}
        else
        {
            HMONITOR monitor = ::MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            if (monitor)
            {
                UINT xDPI = 96;
                UINT yDPI = 96;
                if (::GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &xDPI, &yDPI) == S_OK)
                    scale = float(xDPI) / 96.0f;
            }
        }
        return scale;
	}
}
using namespace DKFramework;
using namespace DKFramework::Private::Win32;

DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
{
	return new Window(win);
}

using WindowEvent = DKWindow::WindowEvent;
using KeyboardEvent = DKWindow::KeyboardEvent;
using MouseEvent = DKWindow::MouseEvent;

Window::Window(DKWindow* userInstance)
	: instance(userInstance)
	, dropTarget(NULL)
	, hWnd(NULL)
	, windowRect({ 0, 0, 1, 1 })
	, contentRect({ 0, 0, 1, 1 })
	, contentScaleFactor(1.0)
	, textCompositionMode(false)
	, proxyWindow(false)
	, activated(false)
	, visible(false)
	, minimized(false)
	, holdMouse(false)
	, resizing(false)
	, autoResize(false)
	, mouseButtonDown({ 0 })
{
	OleInitialize(NULL);

	// register window-class once
	static const WNDCLASSW	wc = {
		CS_OWNDC,
		(WNDPROC)WindowProc,
		0,
		0, 
		::GetModuleHandleW(NULL),
		LoadIconA(NULL, IDI_APPLICATION),
		LoadCursorA(NULL, IDC_ARROW),
		NULL,
		NULL,
		WNDCLASS_DKWINDOW
	};

	static ATOM a = RegisterClassW(&wc);

	DKASSERT_DESC(a != 0, "Failed to register WndClass");

	memset(keyboardStates, 0, 256);
}

Window::~Window()
{
	DKASSERT_DESC_DEBUG(hWnd == NULL,
		"Window must be destroyed before instance being released.");

	OleUninitialize();
}

bool Window::Create(const DKString& title, uint32_t style)
{
	DKASSERT_DEBUG(hWnd == NULL);

	DWORD dwStyle = 0;
	if (style & DKWindow::StyleTitle)
		dwStyle |= WS_CAPTION;
	if (style & DKWindow::StyleCloseButton)
		dwStyle |= WS_SYSMENU;
	if (style & DKWindow::StyleMinimizeButton)
		dwStyle |= WS_MINIMIZEBOX;
	if (style & DKWindow::StyleMaximizeButton)
		dwStyle |= WS_MAXIMIZEBOX;
	if (style & DKWindow::StyleResizableBorder)
		dwStyle |= WS_THICKFRAME;

	DWORD dwStyleEx = 0;

	hWnd = CreateWindowExW(dwStyleEx, WNDCLASS_DKWINDOW,
		(const wchar_t*)title, dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandleW(NULL), 0);

	if (hWnd == NULL)
	{
		DKLog("CreateWindow failed.\n");
		return false;
	}

	::SetLastError(0);

	if (!::SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)this))
	{
		DWORD dwError = ::GetLastError();
		if (dwError)
		{
			// error!
			DKLog("SetWindowLongPtr failed with error %d: %ls", dwError, (const wchar_t*)GetWin32ErrorString(dwError));

			::DestroyWindow(hWnd);
			hWnd = NULL;
			return false;
		}
	}

	if (style & DKWindow::StyleAcceptFileDrop)
	{
		this->dropTarget = new DropTarget(instance);
		RegisterDragDrop(hWnd, dropTarget);
	}
	if (style & DKWindow::StyleAutoResize)
		this->autoResize = true;

	this->proxyWindow = false;

	RECT rc1, rc2;
	::GetClientRect(hWnd, &rc1);
	::GetWindowRect(hWnd, &rc2);
	this->contentRect = DKRect(rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top);
	this->windowRect = DKRect(rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top);

	this->contentScaleFactor = DPIScaleForWindow(hWnd);

	instance->PostWindowEvent(
	{
		WindowEvent::WindowCreated,
        instance,
		windowRect,
		contentRect,
		contentScaleFactor
	});

	::SetTimer(hWnd, TIMERID_UPDATEKEYBOARDMOUSE, 10, 0);
	return true;
}

bool Window::CreateProxy(void* systemHandle)
{
	DKASSERT_DEBUG(hWnd == NULL);
	if (::IsWindow((HWND)systemHandle))
	{
		hWnd = (HWND)systemHandle;
		proxyWindow = true;

		RECT rc1, rc2;
		::GetClientRect(hWnd, &rc1);
		::GetWindowRect(hWnd, &rc2);
		this->contentRect = DKRect(rc1.left, rc1.top, rc1.right - rc1.left, rc1.bottom - rc1.top);
		this->windowRect = DKRect(rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top);

		this->contentScaleFactor = DPIScaleForWindow(hWnd);
		
		instance->PostWindowEvent(
		{
			WindowEvent::WindowCreated,
            instance,
			windowRect,
			contentRect,
			contentScaleFactor
		});
		return true;
	}
	return false;
}

bool Window::IsProxy() const
{
	return proxyWindow;
}

void Window::UpdateProxy()
{
	DKASSERT_DEBUG(hWnd);
	if (proxyWindow)
	{
		RECT client, window;
		::GetClientRect(hWnd, &client);

		float scaleFactor = DPIScaleForWindow(hWnd);

		LONG width = client.right - client.left;
		LONG height = client.bottom - client.top;

		if (scaleFactor != this->contentScaleFactor ||
			width != (LONG)(floor(this->contentRect.size.width + 0.5f)) ||
			height != (LONG)(floor(this->contentRect.size.height + 0.5f)))
		{
			::GetWindowRect(hWnd, &window);

			this->windowRect = DKRect(window.left, window.top, window.right - window.left, window.bottom - window.top);
			this->contentRect = DKRect(client.left, client.top, client.right - client.left, client.bottom - client.top);
			this->contentScaleFactor = scaleFactor;

			instance->PostWindowEvent(
			{
				WindowEvent::WindowResized,
                instance,
				windowRect,
				contentRect,
				contentScaleFactor
			});
		}
	}
}

void Window::Destroy()
{
	activated = false;
	if (hWnd)
	{
		if (proxyWindow)
		{
			instance->PostWindowEvent(
			{
				WindowEvent::WindowClosed,
                instance,
				windowRect,
				contentRect,
				contentScaleFactor
			});
			::InvalidateRect(hWnd, 0, TRUE);
		}
		else
		{
			if (this->dropTarget)
			{
				RevokeDragDrop(hWnd);
				this->dropTarget->Release();
				this->dropTarget = nullptr;				
			}

			::KillTimer(hWnd, TIMERID_UPDATEKEYBOARDMOUSE);

			// set GWLP_USERDATA to 0, to forwarding messages to DefWindowProc.
			::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
			::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			//::DestroyWindow(windowHandle);

			// Post WM_CLOSE to destroy window from DefWindowProc().
			::PostMessageW(hWnd, WM_CLOSE, 0, 0);

			DKLog("Window %x (Handle:%x) destroyed\n", this, hWnd);

			instance->PostWindowEvent(
			{
				WindowEvent::WindowClosed,
                instance,
				windowRect,
				contentRect,
				contentScaleFactor
			});
		}
		hWnd = NULL;
	}
}

void* Window::PlatformHandle() const
{
	return hWnd;
}

bool Window::IsValid() const
{
	if (hWnd && ::IsWindow(hWnd))
		return true;
	return false;
}

void Window::ShowMouse(int deviceId, bool show)
{
	// hiding or showing mouse must be called on thread where the window
	// has been created. If other thread need to control of mouse visibility
	// use custom message or AttachThreadInput() with synchronization.

	if (proxyWindow)
		return;

	if (deviceId != 0)
		return;

	::PostMessageW(hWnd, WM_DKWINDOW_SHOWCURSOR, show, 0);
}

bool Window::IsMouseVisible(int deviceId) const
{
	if (deviceId == 0)
	{
		CURSORINFO info;
		if (GetCursorInfo(&info))
		{
			return info.flags != 0;
		}
	}
	return false;
}

void Window::HoldMouse(int deviceId, bool hold)
{
	if (proxyWindow)
		return;

	if (deviceId != 0)
		return;

	holdMouse = hold;

	mousePosition = MousePosition(0);
	holdingMousePosition = mousePosition;

	::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
}

bool Window::IsMouseHeld(int deviceId) const
{
	if (deviceId == 0)
	{
		return holdMouse;
	}
	return false;
}

void Window::Show()
{
	if (hWnd)
	{
		if (::IsIconic(hWnd))
			::ShowWindow(hWnd, SW_RESTORE);
		else
			::ShowWindow(hWnd, SW_SHOWNA);
	}
}

void Window::Hide()
{
	if (hWnd)
		::ShowWindow(hWnd, SW_HIDE);
}

void Window::Activate()
{
	if (hWnd)
	{
		if (::IsIconic(hWnd))
			::ShowWindow(hWnd, SW_RESTORE);

		::ShowWindow(hWnd, SW_SHOW);
		::SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::SetForegroundWindow(hWnd);
	}
}

void Window::Minimize()
{
	if (hWnd)
		::ShowWindow(hWnd, SW_MINIMIZE);
}

void Window::SetOrigin(DKPoint pt)
{
	int x = floor(pt.x + 0.5f);
	int y = floor(pt.y + 0.5f);

	if (hWnd)
		::SetWindowPos(hWnd, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

void Window::Resize(DKSize s, const DKPoint* pt)
{
	int w = floor(s.width + 0.5);
	int h = floor(s.height + 0.5);

	if (hWnd)
	{
		DWORD style = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));
		DWORD styleEx = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
		BOOL menu = ::GetMenu(hWnd) != NULL;

		RECT rc = { 0, 0, w, h };
		if (::AdjustWindowRectEx(&rc, style, menu, styleEx))
		{
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;
			if (pt)
			{
				int x = floor(pt->x + 0.5f);
				int y = floor(pt->y + 0.5f);
				::SetWindowPos(hWnd, HWND_TOP, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			}
			else
			{
				::SetWindowPos(hWnd, HWND_TOP, 0, 0, w, h, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			}
		}
	}
}

float Window::ContentScaleFactor() const
{
	return contentScaleFactor;
}

void Window::SetTitle(const DKString& title)
{
	::SetWindowTextW(hWnd, (const wchar_t*)title);
}

DKString Window::Title() const
{
	DKString ret = "";
	int len = ::GetWindowTextLengthW(hWnd);
	if (len > 0)
	{
		wchar_t* title = (wchar_t*)DKMalloc(sizeof(wchar_t)* (len + 2));
		len = ::GetWindowTextW(hWnd, title, len + 1);
		title[len] = 0;
		ret = title;
		DKFree(title);
	}
	return ret;
}

void Window::SetMousePosition(int deviceId, DKPoint pt)
{
	if (deviceId != 0)
		return;

	POINT ptScreen;
	ptScreen.x = floor(pt.x + 0.5f);
	ptScreen.y = floor(pt.y + 0.5f);
	::ClientToScreen(hWnd, &ptScreen);
	::SetCursorPos(ptScreen.x, ptScreen.y);
	mousePosition = pt;
}

DKPoint Window::MousePosition(int deviceId) const
{
	if (deviceId != 0)
		return DKPoint(-1, -1);

	POINT pt;
	::GetCursorPos(&pt);
	::ScreenToClient(hWnd, &pt);
	return DKPoint(pt.x, pt.y);
}

void Window::EnableTextInput(int deviceId, bool enable)
{
	if (deviceId != 0)
		return;

	if (textCompositionMode == enable)
		return;

	textCompositionMode = enable;
}

bool Window::IsTextInputEnabled(int deviceId) const
{
	if (deviceId == 0)
		return textCompositionMode;
	return false;
}

void Window::UpdateKeyboard()
{
	if (!activated)
		return;

	// update keyboard.
	// WM_KEYDOWN, WM_KEYUP is only for English (or non-IME-Composition languages)
	// When IME composition in progress, VK_PROCESSKEY messages are used instead.
	// so we need update raw-key states.

	BYTE keyStateCurrent[256];	// key-state buffer
	memset(keyStateCurrent, 0, sizeof(keyStateCurrent));
	::GetKeyboardState(keyStateCurrent);

	for (int i = 0; i < 256; i++)
	{
		if (i == VK_CAPITAL)
			continue;

		DKVirtualKey lKey = ConvertVKey(i);
		if (lKey == DKVirtualKey::None)
			continue;

		if ((keyStateCurrent[i] & 0x80) != (keyboardStates[i] & 0x80))
		{
			if (keyStateCurrent[i] & 0x80)
			{
				instance->PostKeyboardEvent({ KeyboardEvent::KeyDown, instance, 0,lKey, ""});
			}
			else
			{
				instance->PostKeyboardEvent({ KeyboardEvent::KeyUp, instance, 0, lKey, ""});
			}
		}
	}
	if ((keyStateCurrent[VK_CAPITAL] & 0x01) != (keyboardStates[VK_CAPITAL] & 0x01))
	{
		if (keyStateCurrent[VK_CAPITAL] & 0x01)
		{
			instance->PostKeyboardEvent({ KeyboardEvent::KeyDown, instance, 0, DKVirtualKey::Capslock, ""});
		}
		else
		{
			instance->PostKeyboardEvent({ KeyboardEvent::KeyUp, instance, 0, DKVirtualKey::Capslock, ""});
		}
	}
	memcpy(keyboardStates, keyStateCurrent, 256);
}

void Window::UpdateMouse()
{
	if (!activated)
		return;

	// check mouse has gone out of window region.
	if (GetCapture() != hWnd)
	{
		POINT ptMouse;
		::GetCursorPos(&ptMouse);
		::ScreenToClient(hWnd, &ptMouse);

		RECT rc;
		::GetClientRect(hWnd, &rc);
		if (ptMouse.x < rc.left || ptMouse.x > rc.right || ptMouse.y > rc.bottom || ptMouse.y < rc.top)
			::PostMessageW(hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(ptMouse.x, ptMouse.y));
	}
}

void Window::ResetKeyboard()
{
	for (int i = 0; i < 256; i++)
	{
		if (i == VK_CAPITAL)
			continue;

		DKVirtualKey lKey = ConvertVKey(i);
		if (lKey == DKVirtualKey::None)
			continue;

		if (keyboardStates[i] & 0x80)
		{
			instance->PostKeyboardEvent({ KeyboardEvent::KeyUp, instance, 0, lKey, "" });
		}
	}

	if (keyboardStates[VK_CAPITAL] & 0x01)
	{
		instance->PostKeyboardEvent({ KeyboardEvent::KeyUp, instance, 0, DKVirtualKey::Capslock, ""});
	}

	::GetKeyboardState(keyboardStates);	// to empty keyboard queue
	memset(keyboardStates, 0, sizeof(keyboardStates));
}

void Window::ResetMouse()
{
	POINT ptMouse;
	::GetCursorPos(&ptMouse);
	::ScreenToClient(hWnd, &ptMouse);
	mousePosition = DKPoint(ptMouse.x, ptMouse.y);
}

LRESULT Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd)
	{
		Window* window = (Window*)::GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		if (window && window->hWnd == hWnd)
		{
			switch (uMsg)
			{
			case WM_ACTIVATE:
				if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
				{
					if (!window->activated)
					{
						numActiveWindows.Increment();
						window->activated = true;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowActivated,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
						window->ResetKeyboard();
						window->ResetMouse();  // to prevent mouse cursor popped.
					}
				}
				else
				{
					if (window->activated)
					{
						numActiveWindows.Decrement();
						window->ResetKeyboard();	// release all keys
						window->ResetMouse();
						window->activated = false;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowInactivated,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				return 0;
			case WM_SHOWWINDOW:
				if (wParam)
				{
					if (!window->visible)
					{
						window->visible = true;
						window->minimized = false;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowShown,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				else
				{
					if (window->visible)
					{
						window->visible = false;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowHidden,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				return 0;
			case WM_ENTERSIZEMOVE:
				window->resizing = true;
				return 0;
			case WM_EXITSIZEMOVE:
				window->resizing = false;
				if (true)
				{
					RECT rcClient, rcWindow;
					::GetClientRect(hWnd, &rcClient);
					::GetWindowRect(hWnd, &rcWindow);
					bool resized = false;
					bool moved = false;
					if ((rcClient.right - rcClient.left) != (LONG)floor(window->contentRect.size.width + 0.5f) ||
						(rcClient.bottom - rcClient.top) != (LONG)floor(window->contentRect.size.height + 0.5f))
						resized = true;

					if (rcWindow.left != (LONG)floor(window->windowRect.origin.x + 0.5f) ||
						rcWindow.top != (LONG)floor(window->windowRect.origin.y + 0.5f))
						moved = true;

					if (resized || moved)
					{
						window->windowRect = DKRect(rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top);
						window->contentRect = DKRect(rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
						if (resized)
							window->instance->PostWindowEvent({
                                WindowEvent::WindowResized,
                                window->instance,
                                window->windowRect,
                                window->contentRect,
                                window->contentScaleFactor
                            });
						if (moved)
							window->instance->PostWindowEvent({
                                WindowEvent::WindowMoved,
                                window->instance,
                                window->windowRect,
                                window->contentRect,
                                window->contentScaleFactor
                            });
					}
				}
				return 0;
			case WM_SIZE:
				if (wParam == SIZE_MAXHIDE)
				{
					if (window->visible)
					{
						window->visible = false;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowHidden,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				else if (wParam == SIZE_MINIMIZED)
				{
					if (!window->minimized)
					{
						window->minimized = true;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowMinimized,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				else
				{
					if (window->minimized || !window->visible)
					{
						window->minimized = false;
						window->visible = true;
						window->instance->PostWindowEvent({
                            WindowEvent::WindowShown,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
					else
					{
						DKSize size = DKSize(LOWORD(lParam), HIWORD(lParam));
						size.width = floor(size.width + 0.5f);
						size.height = floor(size.height + 0.5f);
						window->contentRect.size = size;

						RECT rc;
						::GetWindowRect(hWnd, &rc);
						window->windowRect = DKRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

						window->instance->PostWindowEvent({
                            WindowEvent::WindowResized,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor
                        });
					}
				}
				return 0;
			case WM_MOVE:
				if (!window->resizing)
				{
					int x = (int)(short)LOWORD(lParam);   // horizontal position 
					int y = (int)(short)HIWORD(lParam);   // vertical position 
					window->windowRect.origin = DKPoint(x, y);
					window->instance->PostWindowEvent({
                        WindowEvent::WindowMoved,
                        window->instance,
                        window->windowRect,
                        window->contentRect,
                        window->contentScaleFactor
                    });
				}
				return 0;
			case WM_DPICHANGED:
				if (true)
				{
					// Note: xDPI, yDPI are identical for Windows apps
					int xDPI = HIWORD(wParam);
					int yDPI = HIWORD(wParam);
					RECT* suggestedWindowRect = (RECT*)lParam;
					
					float scaleFactor = double(xDPI) / 96.0;
					window->contentScaleFactor = scaleFactor;

					if (window->autoResize)
					{
						::SetWindowPos(hWnd,
							NULL,
							suggestedWindowRect->left,
							suggestedWindowRect->top,
							suggestedWindowRect->right - suggestedWindowRect->left,
							suggestedWindowRect->bottom - suggestedWindowRect->top,
							SWP_NOZORDER | SWP_NOACTIVATE);
					}
					else
					{
						window->instance->PostWindowEvent({
                            WindowEvent::WindowResized,
                            window->instance,
                            window->windowRect,
                            window->contentRect,
                            window->contentScaleFactor,
                        });
					}
				}
				return 0;
			case WM_GETMINMAXINFO:
				if (true)
				{
					const DKWindow::WindowCallback& cb = window->instance->Callback();

					DWORD style = ((DWORD)GetWindowLong(hWnd, GWL_STYLE));
					DWORD styleEx = ((DWORD)GetWindowLong(hWnd, GWL_EXSTYLE));
					BOOL menu = ::GetMenu(hWnd) != NULL;

					if (true)
					{
						DKSize s = { 1, 1 };
						if (cb.contentMinSize)
							s = cb.contentMinSize->Invoke(window->instance);
						LONG w = floor(s.width + 0.5f);
						LONG h = floor(s.height + 0.5f);
						RECT rc = { 0, 0, Max(w, 1), Max(h, 1) };

						if (::AdjustWindowRectEx(&rc, style, menu, styleEx))
						{
							MINMAXINFO* mm = (MINMAXINFO*)lParam;
							mm->ptMinTrackSize.x = rc.right - rc.left;
							mm->ptMinTrackSize.y = rc.bottom - rc.top;
						}
					}
					if (cb.contentMaxSize)
					{
						DKSize s = cb.contentMaxSize->Invoke(window->instance);
						LONG w = floor(s.width + 0.5f);
						LONG h = floor(s.height + 0.5f);
						RECT rc = { 0, 0, Max(w, 1), Max(h, 1) };

						if (::AdjustWindowRectEx(&rc, style, menu, styleEx))
						{
							MINMAXINFO* mm = (MINMAXINFO*)lParam;
							if (w > 0)
								mm->ptMaxTrackSize.x = rc.right - rc.left;
							if (h > 0)
								mm->ptMaxTrackSize.y = rc.bottom - rc.top;
						}

					}
					return 0;
				}
				break;
			case WM_TIMER:
				if (wParam == TIMERID_UPDATEKEYBOARDMOUSE)
				{
					window->UpdateKeyboard();
					window->UpdateMouse();
					return 0;
				}
				break;
			case WM_MOUSEMOVE:
				if (window->activated)
				{
					DKPoint pos((MAKEPOINTS(lParam)).x, (MAKEPOINTS(lParam)).y);

					LONG px = (LONG)floor(pos.x + 0.5f);
					LONG py = (LONG)floor(pos.y + 0.5f);
					if (px != (LONG)floor(window->mousePosition.x + 0.5f) ||
						py != (LONG)floor(window->mousePosition.y + 0.5f))
					{
						DKVector2 delta = pos.Vector() - window->mousePosition.Vector();

						bool postEvent = true;
						if (window->holdMouse)
						{
							if (px == (LONG)floor(window->holdingMousePosition.x + 0.5f) &&
								py == (LONG)floor(window->holdingMousePosition.y + 0.5f))
								postEvent = false;
							else
							{
								window->SetMousePosition(0, window->mousePosition);
								// In Windows8 (or later) with scaled-DPI mode, setting mouse position generate inaccurate result.
								// We need to keep new position in hold-mouse state. (non-movable mouse)
								window->holdingMousePosition = window->MousePosition(0);
							}
						}
						else
						{
							window->mousePosition = pos;
						}

						if (postEvent)
						{
							window->instance->PostMouseEvent({
                                MouseEvent::Move,
                                window->instance,
                                MouseEvent::GenericMouse,
                                0,
                                0,
                                window->mousePosition,
                                delta,
                                0,
                                0});
						}
					}
				}
				return 0;
			case WM_LBUTTONDOWN:
				if (true)
				{
					window->mouseButtonDown.button1 = true;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonDown,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        0,
                        pt,
                        DKVector2(0,0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_LBUTTONUP:
				if (true)
				{
					window->mouseButtonDown.button1 = false;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonUp,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        0,
                        pt,
                        DKVector2(0,0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_RBUTTONDOWN:
				if (true)
				{
					window->mouseButtonDown.button2 = true;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonDown,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        1,
                        pt,
                        DKVector2(0, 0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_RBUTTONUP:
				if (true)
				{
					window->mouseButtonDown.button2 = false;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonUp,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        1,
                        pt,
                        DKVector2(0, 0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_MBUTTONDOWN:
				if (true)
				{
					window->mouseButtonDown.button3 = true;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonDown,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        2,
                        pt,
                        DKVector2(0,0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_MBUTTONUP:
				if (true)
				{
					window->mouseButtonDown.button3 = false;
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					window->instance->PostMouseEvent({
                        MouseEvent::ButtonUp,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        2,
                        pt,
                        DKVector2(0, 0),
                        0,
                        0});
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return 0;
			case WM_XBUTTONDOWN:
				if (true)
				{
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					WORD button = GET_XBUTTON_WPARAM(wParam);
					if (button == XBUTTON1)
					{
						window->mouseButtonDown.button4 = true;
						window->instance->PostMouseEvent({
                            MouseEvent::ButtonDown,
                            window->instance,
                            MouseEvent::GenericMouse,
                            0,
                            3,
                            pt,
                            DKVector2(0, 0),
                            0,
                            0});
					}
					else if (button == XBUTTON2)
					{
						window->mouseButtonDown.button5 = true;
						window->instance->PostMouseEvent({
                            MouseEvent::ButtonDown,
                            window->instance,
                            MouseEvent::GenericMouse,
                            0,
                            4,
                            pt,
                            DKVector2(0, 0),
                            0,
                            0});
					}
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return TRUE;
			case WM_XBUTTONUP:
				if (true)
				{
					DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
					WORD button = GET_XBUTTON_WPARAM(wParam);
					if (button == XBUTTON1)
					{
						window->mouseButtonDown.button4 = false;
						window->instance->PostMouseEvent({
                            MouseEvent::ButtonUp,
                            window->instance,
                            MouseEvent::GenericMouse,
                            0,
                            3,
                            pt,
                            DKVector2(0, 0),
                            0,
                            0});
					}
					else if (button == XBUTTON2)
					{
						window->mouseButtonDown.button5 = false;
						window->instance->PostMouseEvent({
                            MouseEvent::ButtonUp,
                            window->instance,
                            MouseEvent::GenericMouse,
                            0,
                            4,
                            pt,
                            DKVector2(0, 0),
                            0,
                            0});
					}
					::PostMessageW(hWnd, WM_DKWINDOW_UPDATEMOUSECAPTURE, 0, 0);
				}
				return TRUE;
			case WM_MOUSEWHEEL:
				if (true) {
					POINT origin = { 0, 0 };
					::ClientToScreen(hWnd, &origin);
					DKPoint pt(MAKEPOINTS(lParam).x - origin.x, MAKEPOINTS(lParam).y - origin.y);

					int nDelta = GET_WHEEL_DELTA_WPARAM(wParam);

					DKVector2 delta(0, 0);
					if (nDelta > 0)
						delta.y = 1;
					else if (nDelta < 0)
						delta.y = -1;
					window->instance->PostMouseEvent({
                        MouseEvent::Wheel,
                        window->instance,
                        MouseEvent::GenericMouse,
                        0,
                        2,
                        pt,
                        delta,
                        0,
                        0});
				}
				return 0;
			case WM_CHAR:
				window->UpdateKeyboard();	// synchronize key states
				if (window->textCompositionMode)
				{
					wchar_t c = (wchar_t)wParam;
					DKString text(c);
					window->instance->PostKeyboardEvent({ KeyboardEvent::TextInput, window->instance, 0, DKVirtualKey::None, text });
				}
				return 0;
			case WM_IME_STARTCOMPOSITION:
				return 0;
			case WM_IME_ENDCOMPOSITION:
				return 0;
			case WM_IME_COMPOSITION:
				window->UpdateKeyboard();	// synchronize key states

				if (lParam & GCS_RESULTSTR)		// composition finished.
				{
					// Result characters will be received via WM_CHAR,
					// reset input-candidate characters here.
					window->instance->PostKeyboardEvent({ KeyboardEvent::TextComposition, window->instance, 0, DKVirtualKey::None, "" });
				}
				if (lParam & GCS_COMPSTR)		// composition in progress.
				{
					HIMC hIMC = ImmGetContext(hWnd);
					if (hIMC)
					{
						if (window->textCompositionMode)
						{
							long textLength = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, 0, 0);
							if (textLength)
							{
								unsigned char *tmp = (unsigned char*)DKMalloc(textLength + 4);
								memset(tmp, 0, textLength + 4);

								ImmGetCompositionStringW(hIMC, GCS_COMPSTR, tmp, textLength + 2);
								DKString compositionText((const wchar_t*)tmp);
								DKFree(tmp);

								window->instance->PostKeyboardEvent({ KeyboardEvent::TextComposition, window->instance, 0, DKVirtualKey::None, compositionText });
								//	DKLog("WM_IME_COMPOSITION: '%ls'\n", strInputCandidate));
							}
							else	// composition character's length become 0. (erased)
							{
								window->instance->PostKeyboardEvent({ KeyboardEvent::TextComposition, window->instance, 0, DKVirtualKey::None, "" });
							}
						}
						else	// not text-input mode.
						{
							ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
						}

						ImmReleaseContext(hWnd, hIMC);
					}
				}
				break;
			case WM_PAINT:
				if (!window->resizing)
				{
					window->instance->PostWindowEvent({
                        WindowEvent::WindowUpdate,
                        window->instance,
                        window->windowRect,
                        window->contentRect,
                        window->contentScaleFactor,
                    });
				}
				break;
			case WM_CLOSE:
				if (true)
				{
					const DKWindow::WindowCallback& cb = window->instance->Callback();
					if (cb.closeRequest)
					{
						if (!cb.closeRequest->Invoke(window->instance))
							return 0;
					}
					window->Destroy();
				}
				return 0;
			case WM_COMMAND:
				break;
			case WM_SYSCOMMAND:
				switch (wParam)
				{
				case SC_CONTEXTHELP:	// help menu
				case SC_KEYMENU:		// alt-key
				case SC_HOTKEY:			// hotkey
					return 0;
					break;
				}
				break;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
				return 0;				// block ALT-key
			case WM_KEYDOWN:
			case WM_KEYUP:
				return 0;
			case WM_DKWINDOW_SHOWCURSOR:
				if (true) {
					// If we need to control mouse position from other thread,
					// we should call AttachThreadInput() to synchronize threads.
					// but we are not going to control position, but control visibility
					// only, we can use window message.
					if ((BOOL)wParam)
						while (::ShowCursor(TRUE) < 0);
					else
						while (::ShowCursor(FALSE) >= 0);
				}
				return 0;
			case WM_DKWINDOW_UPDATEMOUSECAPTURE:
				if (::GetCapture() == hWnd)
				{
					if (window->mouseButtonDown.buttons == 0 && !window->holdMouse)
						::ReleaseCapture();
				}
				else
				{
					if (window->mouseButtonDown.buttons || window->holdMouse)
						::SetCapture(hWnd);
				}
				return 0;
			}
		}
	}
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

DKVirtualKey Window::ConvertVKey(int key)
{
	switch (key)
	{
	case 0x03:	return DKVirtualKey::F15;			// VK_F15 (ctrl+break)
	case 0x08:	return DKVirtualKey::Backspace;		// VK_BACK
	case 0x09:	return DKVirtualKey::Tab;			// VK_TAB
	case 0x0D:	return DKVirtualKey::Return;		// VK_RETURN
//case 0x10:	return DKVirtualKey::SHIFT;			// VK_SHIFT
//case 0x11:	return DKVirtualKey::CONTROL;		// VK_CONTROL
//case 0x12:	return DKVirtualKey::OPTION;		// VK_MENU
	case 0x13:	return DKVirtualKey::F15;			// VK_PAUSE
	case 0x14:	return DKVirtualKey::Capslock;		// VK_CAPITAL, CAPSLOCK
	case 0x1B:	return DKVirtualKey::Escape;		// VK_ESCAPE
	case 0x1C:	return DKVirtualKey::None;			// VK_CONVERT
	case 0x1D:	return DKVirtualKey::None;			// VK_NONCONVERT
	case 0x1E:	return DKVirtualKey::None;			// VK_ACCEPT
	case 0x1F:	return DKVirtualKey::None;			// VK_MODECHANGE
	case 0x20:	return DKVirtualKey::Space;			// VK_SPACE
	case 0x21:	return DKVirtualKey::PageUp;		// VK_PRIOR
	case 0x22:	return DKVirtualKey::PageDown;		// VK_NEXT
	case 0x23:	return DKVirtualKey::End;			// VK_END
	case 0x24:	return DKVirtualKey::Home;			// VK_HOME
	case 0x25:	return DKVirtualKey::Left;			// VK_LEFT
	case 0x26:	return DKVirtualKey::Up;			// VK_UP
	case 0x27:	return DKVirtualKey::Right;			// VK_RIGHT
	case 0x28:	return DKVirtualKey::Down;			// VK_DOWN
	case 0x29:	return DKVirtualKey::None;			// VK_SELECT
	case 0x2A:	return DKVirtualKey::None;			// VK_PRINT
	case 0x2B:	return DKVirtualKey::None;			// VK_EXECUTE
	case 0x2C:	return DKVirtualKey::F13;			// VK_SNAPSHOT, PRINT SCREEN KEY
	case 0x2D:	return DKVirtualKey::Insert;		// VK_INSERT
	case 0x2E:	return DKVirtualKey::Delete;		// VK_DELETE
	case 0x2F:	return DKVirtualKey::None;			// VK_HELP
	case 0x30:	return DKVirtualKey::Num0;			// 0
	case 0x31:	return DKVirtualKey::Num1;			// 1
	case 0x32:	return DKVirtualKey::Num2;			// 2
	case 0x33:	return DKVirtualKey::Num3;			// 3
	case 0x34:	return DKVirtualKey::Num4;			// 4
	case 0x35:	return DKVirtualKey::Num5;			// 5
	case 0x36:	return DKVirtualKey::Num6;			// 6
	case 0x37:	return DKVirtualKey::Num7;			// 7
	case 0x38:	return DKVirtualKey::Num8;			// 8
	case 0x39:	return DKVirtualKey::Num9;			// 9

	case 0x41:	return DKVirtualKey::A;				// A
	case 0x42:	return DKVirtualKey::B;				// B
	case 0x43:	return DKVirtualKey::C;				// C
	case 0x44:	return DKVirtualKey::D;				// D
	case 0x45:	return DKVirtualKey::E;				// E
	case 0x46:	return DKVirtualKey::F;				// F
	case 0x47:	return DKVirtualKey::G;				// G
	case 0x48:	return DKVirtualKey::H;				// H
	case 0x49:	return DKVirtualKey::I;				// I
	case 0x4A:	return DKVirtualKey::J;				// J
	case 0x4B:	return DKVirtualKey::K;				// K
	case 0x4C:	return DKVirtualKey::L;				// L
	case 0x4D:	return DKVirtualKey::M;				// M
	case 0x4E:	return DKVirtualKey::N;				// N
	case 0x4F:	return DKVirtualKey::O;				// O
	case 0x50:	return DKVirtualKey::P;				// P
	case 0x51:	return DKVirtualKey::Q;				// Q
	case 0x52:	return DKVirtualKey::R;				// R
	case 0x53:	return DKVirtualKey::S;				// S
	case 0x54:	return DKVirtualKey::T;				// T
	case 0x55:	return DKVirtualKey::U;				// U
	case 0x56:	return DKVirtualKey::V;				// V
	case 0x57:	return DKVirtualKey::W;				// W
	case 0x58:	return DKVirtualKey::X;				// X
	case 0x59:	return DKVirtualKey::Y;				// Y
	case 0x5A:	return DKVirtualKey::Z;				// Z
	case 0x5B:	return DKVirtualKey::LeftCommand;	// VK_LWIN
	case 0x5C:	return DKVirtualKey::RightCommand;	// VK_RWIN
	case 0x5D:	return DKVirtualKey::None;			// VK_APPS
	case 0x5F:	return DKVirtualKey::None;			// VK_SLEEP
	case 0x60:	return DKVirtualKey::Pad0;			// VK_NUMPAD0
	case 0x61:	return DKVirtualKey::Pad1;			// VK_NUMPAD1
	case 0x62:	return DKVirtualKey::Pad2;			// VK_NUMPAD2
	case 0x63:	return DKVirtualKey::Pad3;			// VK_NUMPAD3
	case 0x64:	return DKVirtualKey::Pad4;			// VK_NUMPAD4
	case 0x65:	return DKVirtualKey::Pad5;			// VK_NUMPAD5
	case 0x66:	return DKVirtualKey::Pad6;			// VK_NUMPAD6
	case 0x67:	return DKVirtualKey::Pad7;			// VK_NUMPAD7
	case 0x68:	return DKVirtualKey::Pad8;			// VK_NUMPAD8
	case 0x69:	return DKVirtualKey::Pad9;			// VK_NUMPAD9
	case 0x6A:	return DKVirtualKey::PadAsterisk;	// VK_MULTIPLY
	case 0x6B:	return DKVirtualKey::PadPlus;		// VK_ADD
	case 0x6C:	return DKVirtualKey::None;			// VK_SEPARATOR
	case 0x6D:	return DKVirtualKey::PadMinus;		// VK_SUBTRACT
	case 0x6E:	return DKVirtualKey::PadPeriod;		// VK_DECIMAL
	case 0x6F:	return DKVirtualKey::PadSlash;		// VK_DIVIDE
	case 0x70:	return DKVirtualKey::F1;			// VK_F1
	case 0x71:	return DKVirtualKey::F2;			// VK_F2
	case 0x72:	return DKVirtualKey::F3;			// VK_F3
	case 0x73:	return DKVirtualKey::F4;			// VK_F4
	case 0x74:	return DKVirtualKey::F5;			// VK_F5
	case 0x75:	return DKVirtualKey::F6;			// VK_F6
	case 0x76:	return DKVirtualKey::F7;			// VK_F7
	case 0x77:	return DKVirtualKey::F8;			// VK_F8
	case 0x78:	return DKVirtualKey::F9;			// VK_F9
	case 0x79:	return DKVirtualKey::F10;			// VK_F10
	case 0x7A:	return DKVirtualKey::F11;			// VK_F11
	case 0x7B:	return DKVirtualKey::F12;			// VK_F12
	case 0x7C:	return DKVirtualKey::F13;			// VK_F13
	case 0x7D:	return DKVirtualKey::F14;			// VK_F14
	case 0x7E:	return DKVirtualKey::F15;			// VK_F15
	case 0x7F:	return DKVirtualKey::F16;			// VK_F16
	case 0x80:	return DKVirtualKey::F17;			// VK_F17
	case 0x81:	return DKVirtualKey::F18;			// VK_F18
	case 0x82:	return DKVirtualKey::F19;			// VK_F19
	case 0x83:	return DKVirtualKey::F20;			// VK_F20
	case 0x84:	return DKVirtualKey::None;			// VK_F21
	case 0x85:	return DKVirtualKey::None;			// VK_F22
	case 0x86:	return DKVirtualKey::None;			// VK_F23
	case 0x87:	return DKVirtualKey::None;			// VK_F24

	case 0x90:	return DKVirtualKey::Numlock;		// VK_NUMLOCK
	case 0x91:	return DKVirtualKey::F14;			// VK_SCROLL, SCROLL LOCK

	case 0xA0:	return DKVirtualKey::LeftShift;		// VK_LSHIFT
	case 0xA1:	return DKVirtualKey::RightShift;	// VK_RSHIFT
	case 0xA2:	return DKVirtualKey::LeftControl;	// VK_LCONTROL
	case 0xA3:	return DKVirtualKey::RightControl;	// VK_RCONTROL
	case 0xA4:	return DKVirtualKey::LeftOption;	// VK_LMENU
	case 0xA5:	return DKVirtualKey::RightOption;	// VK_RMENU

	case 0xBA:	return DKVirtualKey::Semicolon;		// VK_OEM_1, ;
	case 0xBB:	return DKVirtualKey::Equal;			// VK_OEM_PLUS, =
	case 0xBC:	return DKVirtualKey::Comma;			// VK_OEM_COMMA, .
	case 0xBD:	return DKVirtualKey::Hyphen;		// VK_OEM_MINUS, -
	case 0xBE:	return DKVirtualKey::Period;		// VK_OEM_PERIOD
	case 0xBF:	return DKVirtualKey::Slash;			// VK_OEM_2, /?
	case 0xC0:	return DKVirtualKey::AccentTilde;	// VK_OEM_3, `~

	case 0xDB:	return DKVirtualKey::OpenBracket;	// VK_OEM_4, [
	case 0xDC:	return DKVirtualKey::Backslash;		// VK_OEM_5, backslash
	case 0xDD:	return DKVirtualKey::CloseBracket;	// VK_OEM_6, ]
	case 0xDE:	return DKVirtualKey::Quote;			// VK_OEM_7, '
	case 0xDF:	return DKVirtualKey::None;			// VK_OEM_8

	case 0xE2:	return DKVirtualKey::Backslash;		// VK_OEM_102, backslash for 102-keyboard

	case 0xE5:	return DKVirtualKey::None;			// VK_PROCESSKEY, IME-key

	default:	return DKVirtualKey::None;

	}
	return DKVirtualKey::None;
}

#endif // _WIN32
