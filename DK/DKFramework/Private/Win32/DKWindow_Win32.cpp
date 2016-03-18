//
//  File: DKWindow_Win32.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../../DKWindow.h"
#include "DKWindow_Win32.h"
#include <math.h>

#ifdef _WIN32

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;

#define WM_SHOWCURSOR				(WM_USER + 0x1175)
#define WM_UPDATEMOUSECAPTURE		(WM_USER + 0x1180)

#define DKGL_UPDATEKEYBOARDMOUSE	10

#define DKGL_WNDCLASS	L"DKGL_Win32_WndClass"

namespace DKFramework
{
	namespace Private
	{
		// EnableSystemKey(): to prevent window-key inactivate our window.
		bool EnableSystemKey(DKWindow* window, bool enable); // block window-key if 'enable' is true.
		LRESULT CALLBACK ApplicationEventProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	}

	DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
	{
		return new DKWindow_Win32(win);
	}
}

#pragma comment(lib, "Imm32.lib")

DKWindow_Win32::DKWindow_Win32(DKWindow* window)
: ownerWindow(window)
, windowHandle(NULL)
, baseThreadId(0)
, isActive(false)
, isVisible(false)
, isMinimized(false)
, isMouseHeld(false)
, isTextInputMode(false)
, isProxyWindow(false)
, isMouseLButtonDown(false)
, isMouseRButtonDown(false)
, isMouseWButtonDown(false)
, isResizing(false)
{
	memset(savedKeyboardState, 0, 256);

	// register window-class once
	static const WNDCLASSW	wc = { CS_OWNDC, (WNDPROC)ApplicationEventProc, 0, 0, ::GetModuleHandleW(NULL),
		LoadIconA(NULL, IDI_APPLICATION), LoadCursorA(NULL, IDC_ARROW),
		NULL, NULL, DKGL_WNDCLASS };

	static ATOM a = RegisterClassW(&wc);

	DKASSERT_DESC(a != 0, "Failed to register WndClass");
}

DKWindow_Win32::~DKWindow_Win32(void)
{
	DKASSERT_DESC_DEBUG(windowHandle == NULL, "Window must be destroyed before instance being released.");
}

bool DKWindow_Win32::CreateProxy(void* systemHandle)
{
	if (::IsWindow((HWND)systemHandle))
	{
		windowHandle = (HWND)systemHandle;
		isProxyWindow = true;

		baseThreadId = 0;

		RECT rc;
		::GetClientRect(windowHandle, &rc);
		contentSize = DKSize(rc.right - rc.left, rc.bottom - rc.top);
		::GetWindowRect(windowHandle, &rc);
		windowOrigin = DKPoint(rc.left, rc.top);

		ownerWindow->PostWindowEvent(DKWindow::EventWindowCreated, contentSize, windowOrigin, false);

		return true;
	}
	return false;
}

void DKWindow_Win32::UpdateProxy(void)
{
	if (isProxyWindow)
	{
		RECT rc;
		::GetClientRect(windowHandle, &rc);
		float width = rc.right - rc.left;
		float height = rc.bottom - rc.top;


		if (width > 0 && height > 0 && (width != contentSize.width || height != contentSize.height))
		{
			// size has been changed.
			contentSize = DKSize(width, height);
			::GetWindowRect(windowHandle, &rc);
			windowOrigin = DKPoint(rc.left, rc.top);

			ownerWindow->PostWindowEvent(DKWindow::EventWindowResized, contentSize, windowOrigin, false);
		}
	}
}

bool DKWindow_Win32::IsProxy(void) const
{
	return isProxyWindow;
}

bool DKWindow_Win32::Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style)
{
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

	int width = floor(size.width + 0.5f);
	int height = floor(size.height + 0.5f);
	if (width > 0 && height > 0)
	{
		RECT rc = { 0, 0, width, height };
		::AdjustWindowRectEx(&rc, dwStyle, NULL, dwStyleEx);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
	}
	else
	{
		width = CW_USEDEFAULT;
		height = CW_USEDEFAULT;
	}
	int x = CW_USEDEFAULT;
	int y = CW_USEDEFAULT;

	if (width != CW_USEDEFAULT && height != CW_USEDEFAULT &&
		origin.x + size.width > 0 && origin.y + size.height > 0)
	{
		x = floor(origin.x + 0.5f);
		y = floor(origin.y + 0.5f);
	}

	windowHandle = CreateWindowExW(dwStyleEx, DKGL_WNDCLASS, (const wchar_t*)title, dwStyle,
		x, y, width, height,
		NULL, NULL, GetModuleHandleW(NULL), 0);
	if (windowHandle == NULL)
	{
		DKLog("CreateWindow failed.\n");
		return false;
	}

	// save content area.
	windowOrigin = origin;
	contentSize = size;

	::SetLastError(0);
	if (!::SetWindowLongPtrW(windowHandle, GWLP_USERDATA, (LONG_PTR)this))
	{
		DWORD dwError = ::GetLastError();
		if (dwError)
		{
			// error!
			LPVOID lpMsgBuf;

			::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

			DKLog("SetWindowLongPtr failed with error %d: %ls", dwError, (const wchar_t*)lpMsgBuf);
			::LocalFree(lpMsgBuf);

			::DestroyWindow(windowHandle);
			windowHandle = NULL;
			return false;
		}
	}

	if (style & DKWindow::StyleAcceptFileDrop)
		DragAcceptFiles(windowHandle, TRUE);

	baseThreadId = ::GetCurrentThreadId();

	isProxyWindow = false;

	ownerWindow->PostWindowEvent(DKWindow::EventWindowCreated, contentSize, windowOrigin, false);

	::SetTimer(windowHandle, DKGL_UPDATEKEYBOARDMOUSE, 10, 0);

	return true;
}

void DKWindow_Win32::Destroy(void)
{
	isActive = false;
	if (windowHandle)
	{
		if (isProxyWindow)
		{
			ownerWindow->PostWindowEvent(DKWindow::EventWindowClosed, contentSize, windowOrigin, false);
			windowHandle = NULL;
			::InvalidateRect(windowHandle, 0, TRUE);
		}
		else
		{
			::KillTimer(windowHandle, DKGL_UPDATEKEYBOARDMOUSE);

			// set GWLP_USERDATA to 0, to forwarding messages to DefWindowProc.
			::SetWindowLongPtrW(windowHandle, GWLP_USERDATA, 0);
			::SetWindowPos(windowHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

			//::DestroyWindow(windowHandle);

			// Post WM_CLOSE to destroy window from DefWindowProc().
			::PostMessageW(windowHandle, WM_CLOSE, 0, 0);

			DKLog("Window %x (Handle:%x) destroyed\n", this, windowHandle);
			windowHandle = NULL;
			baseThreadId = 0;

			ownerWindow->PostWindowEvent(DKWindow::EventWindowClosed, contentSize, windowOrigin, false);
		}
	}
}

void DKWindow_Win32::ShowMouse(int deviceId, bool bShow)
{
	// hiding or showing mouse must be called on thread where the window
	// has been created. If other thread need to control of mouse visibility
	// use custom message or AttachThreadInput() with synchronization.

	/*
	if (baseThreadId == ::GetCurrentThreadId())
	{
		int nCount;
		if (bShow)
		{
			nCount = ::ShowCursor(TRUE);
			while (nCount <= 0)
				nCount = ::ShowCursor(TRUE);
		}
		else
		{
			nCount = ::ShowCursor(FALSE);
			while (nCount >= 0)
				nCount = ::ShowCursor(FALSE);

		}
		DKLog("ShowMouse(%d) counter = %d\n", bShow, nCount);
	}
	else
	{
		::PostMessageW(m_window, WM_SHOWCURSOR, bShow, 0);
	}
	*/


	if (isProxyWindow)
		return;

	if (deviceId != 0)
		return;

	::PostMessageW(windowHandle, WM_SHOWCURSOR, bShow, 0);
}

bool DKWindow_Win32::IsMouseVisible(int deviceId) const
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

void DKWindow_Win32::HoldMouse(int deviceId, bool hold)
{
	if (isProxyWindow)
		return;

	if (deviceId != 0)
		return;

	isMouseHeld = hold;

	mousePosition = MousePosition(0);
	SetMousePosition(0, mousePosition);
	mousePositionHeld = MousePosition(0);

	::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
}

bool DKWindow_Win32::IsMouseHeld(int deviceId) const
{
	if (deviceId == 0)
	{
		return isMouseHeld;
	}
	return false;
}

void DKWindow_Win32::Show(void)
{
	if (windowHandle)
	{
		if (::IsIconic(windowHandle))
			::ShowWindow(windowHandle, SW_RESTORE);
		else
			::ShowWindow(windowHandle, SW_SHOWNA);
	}
}

void DKWindow_Win32::Hide(void)
{
	if (windowHandle)
		::ShowWindow(windowHandle, SW_HIDE);
}

void DKWindow_Win32::Activate(void)
{
	if (windowHandle)
	{
		if (::IsIconic(windowHandle))
			::ShowWindow(windowHandle, SW_RESTORE);
		
		::ShowWindow(windowHandle, SW_SHOW);
		::SetWindowPos(windowHandle, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::SetForegroundWindow(windowHandle);
	}
}

void DKWindow_Win32::Minimize(void)
{
	if (windowHandle)
		::ShowWindow(windowHandle, SW_MINIMIZE);
}

void DKWindow_Win32::Resize(const DKSize& s, const DKPoint* pt)
{
	int w = floor(s.width + 0.5);
	int h = floor(s.height + 0.5);

	if (windowHandle)
	{
		DWORD style = ((DWORD)GetWindowLong(windowHandle, GWL_STYLE));
		DWORD styleEx = ((DWORD)GetWindowLong(windowHandle, GWL_EXSTYLE));
		BOOL menu = ::GetMenu(windowHandle) != NULL;

		RECT rc = { 0, 0, w, h };
		if (::AdjustWindowRectEx(&rc, style, menu, styleEx))
		{
			w = rc.right - rc.left;
			h = rc.bottom - rc.top;
			if (pt)
			{
				int x = floor(pt->x + 0.5f);
				int y = floor(pt->y + 0.5f);
				::SetWindowPos(windowHandle, HWND_TOP, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			}
			else
			{
				::SetWindowPos(windowHandle, HWND_TOP, 0, 0, w, h, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
			}
		}
	}
}

void DKWindow_Win32::SetOrigin(const DKPoint& pt)
{
	int x = floor(pt.x + 0.5f);
	int y = floor(pt.y + 0.5f);

	if (windowHandle)
		::SetWindowPos(windowHandle, HWND_TOP, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
}

void DKWindow_Win32::SetTitle(const DKString& title)
{
	::SetWindowTextW(windowHandle, (const wchar_t*)title);
}

DKString DKWindow_Win32::Title(void) const
{
	DKString ret = "";
	int len = ::GetWindowTextLengthW(windowHandle);
	if (len > 0)
	{
		wchar_t* title = (wchar_t*)DKMemoryDefaultAllocator::Alloc(sizeof(wchar_t)* (len + 2));
		len = ::GetWindowTextW(windowHandle, title, len + 1);
		title[len] = 0;
		ret = title;
		DKMemoryDefaultAllocator::Free(title);
	}
	return ret;
}

void DKWindow_Win32::SetMousePosition(int deviceId, const DKPoint& pt)
{
	if (deviceId != 0)
		return;

	DKPoint pt2(pt);
	ConvertCoordinateOrigin(pt2);
	POINT ptScreen;
	ptScreen.x = floor(pt2.x + 0.5f);
	ptScreen.y = floor(pt2.y + 0.5f);
	::ClientToScreen(windowHandle, &ptScreen);
	::SetCursorPos(ptScreen.x, ptScreen.y);
	mousePosition = pt;
}

DKPoint DKWindow_Win32::MousePosition(int deviceId) const
{
	if (deviceId != 0)
		return DKPoint(-1, -1);

	POINT pt;
	::GetCursorPos(&pt);
	::ScreenToClient(windowHandle, &pt);
	DKPoint pt2(pt.x, pt.y);
	ConvertCoordinateOrigin(pt2);
	return pt2;
}

void DKWindow_Win32::EnableTextInput(int deviceId, bool bTextInput)
{
	if (deviceId != 0)
		return;

	if (isTextInputMode == bTextInput)
		return;

	isTextInputMode = bTextInput;
}

void DKWindow_Win32::UpdateKeyboard(void)
{
	if (!isActive)
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
		if (lKey == DKVK_NONE)
			continue;

		if ((keyStateCurrent[i] & 0x80) != (savedKeyboardState[i] & 0x80))
		{
			if (keyStateCurrent[i] & 0x80)
			{
				ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, lKey, L"", false);
			}
			else
			{
				ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, lKey, L"", false);
			}
		}
	}
	if ((keyStateCurrent[VK_CAPITAL] & 0x01) != (savedKeyboardState[VK_CAPITAL] & 0x01))
	{
		if (keyStateCurrent[VK_CAPITAL] & 0x01)
		{
			ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_CAPSLOCK, L"", false);
		}
		else
		{
			ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_CAPSLOCK, L"", false);
		}
	}
	memcpy(savedKeyboardState, keyStateCurrent, 256);
}

void DKWindow_Win32::UpdateMouse(void)
{
	if (!isActive)
		return;

	// check mouse has gone out of window region.
	if (GetCapture() != windowHandle)
	{
		POINT ptMouse;
		::GetCursorPos(&ptMouse);
		::ScreenToClient(windowHandle, &ptMouse);

		RECT rc;
		::GetClientRect(windowHandle, &rc);
		if (ptMouse.x < rc.left || ptMouse.x > rc.right || ptMouse.y > rc.bottom || ptMouse.y < rc.top)
			::PostMessageW(windowHandle, WM_MOUSEMOVE, 0, MAKELPARAM(ptMouse.x, ptMouse.y));
	}
}

void DKWindow_Win32::ResetKeyboard(void)
{
	for (int i = 0; i < 256; i++)
	{
		if (i == VK_CAPITAL)
			continue;

		DKVirtualKey lKey = ConvertVKey(i);
		if (lKey == DKVK_NONE)
			continue;

		if (savedKeyboardState[i] & 0x80)
		{
			ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, lKey, L"", false);
		}
	}

	if (savedKeyboardState[VK_CAPITAL] & 0x01)
	{
		ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_CAPSLOCK, L"", false);
	}

	::GetKeyboardState(savedKeyboardState);	// to empty keyboard queue
	memset(savedKeyboardState, 0, sizeof(savedKeyboardState));
}

void DKWindow_Win32::ResetMouse(void)
{
	POINT ptMouse;
	::GetCursorPos(&ptMouse);
	::ScreenToClient(windowHandle, &ptMouse);
	DKPoint pt(ptMouse.x, ptMouse.y);
	ConvertCoordinateOrigin(pt);
	mousePosition = pt;
}

////////////////////////////////////////////////////////////////////////////////
// Window message handler
LRESULT DKWindow_Win32::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATEAPP:
		break;
	case WM_DESTROY:
		break;
	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			if (!isActive)
			{
				isActive = true;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowActivated, this->contentSize, this->windowOrigin, false);
				ResetKeyboard();
				ResetMouse();  // to prevent mouse cursor popped.
			}
		}
		else
		{
			if (isActive)
			{
				ResetKeyboard();	// release all keys
				ResetMouse();
				isActive = false;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowInactivated, this->contentSize, this->windowOrigin, false);
			}
		}
		break;
	case WM_SHOWWINDOW:
		if (wParam)
		{
			if (!isVisible)
			{
				isVisible = true;
				isMinimized = false;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowShown, this->contentSize, this->windowOrigin, false);
			}
		}
		else
		{
			if (isVisible)
			{
				isVisible = false;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowHidden, this->contentSize, this->windowOrigin, false);
			}
		}
		break;
	case WM_ENTERSIZEMOVE:
		isResizing = true;
		break;
	case WM_EXITSIZEMOVE:
		isResizing = false;
		break;
	case WM_SIZE:
		if (wParam == SIZE_MAXHIDE)
		{
			if (isVisible)
			{
				isVisible = false;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowHidden, this->contentSize, this->windowOrigin, false);
			}
		}
		else if (wParam == SIZE_MINIMIZED)
		{
			if (!isMinimized)
			{
				isMinimized = true;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowMinimized, this->contentSize, this->windowOrigin, false);
			}
		}
		else
		{
			if (isMinimized || !isVisible)
			{
				isMinimized = false;
				isVisible = true;
				ownerWindow->PostWindowEvent(DKWindow::EventWindowShown, this->contentSize, this->windowOrigin, false);
			}
			else
			{
				DKSize size = DKSize(LOWORD(lParam), HIWORD(lParam));
				size.width = floor(size.width + 0.5f);
				size.height = floor(size.height + 0.5f);
				this->contentSize = size;

				RECT rc;
				::GetWindowRect(this->windowHandle, &rc);
				this->windowOrigin = DKPoint(rc.left, rc.top);

				ownerWindow->PostWindowEvent(DKWindow::EventWindowResized, this->contentSize, this->windowOrigin, false);
			}
		}
		break;
	case WM_MOVE:
		if (!isResizing)
		{
			ownerWindow->PostWindowEvent(DKWindow::EventWindowMoved, this->contentSize, this->windowOrigin, false);
		}
		break;
	case WM_GETMINMAXINFO:
		if (true)
		{
			const DKWindow::WindowCallback& cb = ownerWindow->Callback();
			if (cb.contentMinSize || cb.contentMaxSize)
			{
				DWORD style = ((DWORD)GetWindowLong(windowHandle, GWL_STYLE));
				DWORD styleEx = ((DWORD)GetWindowLong(windowHandle, GWL_EXSTYLE));
				BOOL menu = ::GetMenu(windowHandle) != NULL;

				if (cb.contentMinSize)
				{
					DKSize s = cb.contentMinSize->Invoke(ownerWindow);
					LONG w = floor(s.width + 0.5f);
					LONG h = floor(s.height + 0.5f);
					RECT rc = { 0, 0, Max(w, 0), Max(h, 0) };

					if (::AdjustWindowRectEx(&rc, style, menu, styleEx))
					{
						MINMAXINFO* mm = (MINMAXINFO*)lParam;
						mm->ptMinTrackSize.x = rc.right - rc.left;
						mm->ptMinTrackSize.y = rc.bottom - rc.top;
					}
				}
				if (cb.contentMaxSize)
				{
					DKSize s = cb.contentMaxSize->Invoke(ownerWindow);
					LONG w = floor(s.width + 0.5f);
					LONG h = floor(s.height + 0.5f);
					RECT rc = { 0, 0, Max(w, 0), Max(h, 0) };

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
		}
		break;
	case WM_TIMER:
		if (wParam == DKGL_UPDATEKEYBOARDMOUSE)
		{
			UpdateKeyboard();
			UpdateMouse();
		}
		break;
	case WM_MOUSEMOVE:
		if (isActive)
		{
			DKPoint pos((MAKEPOINTS(lParam)).x, (MAKEPOINTS(lParam)).y);
			ConvertCoordinateOrigin(pos);

			LONG px = (LONG)floor(pos.x + 0.5f);
			LONG py = (LONG)floor(pos.y + 0.5f);
			if (px != (LONG)floor(mousePosition.x + 0.5f) ||
				py != (LONG)floor(mousePosition.y + 0.5f))
			{
				DKVector2 delta = pos.Vector() - mousePosition.Vector();

				bool postEvent = true;
				if (isMouseHeld)
				{
					if (px == (LONG)floor(mousePositionHeld.x + 0.5f) && py == (LONG)floor(mousePositionHeld.y + 0.5f))
						postEvent = false;
					else
					{
						SetMousePosition(0, mousePosition);
						// In Windows8 (or later) with scaled-DPI mode, setting mouse position generate inaccurate result.
						// We need to keep new position in hold-mouse state. (non-movable mouse)
						mousePositionHeld = MousePosition(0);
					}
				}
				else
				{
					mousePosition = pos;
				}

				if (postEvent)
				{
					ownerWindow->PostMouseEvent(DKWindow::EventMouseMove, 0, 0, mousePosition, delta, false);
				}
			}
		}
		break;
	case WM_LBUTTONDOWN:
		if (true) {
			isMouseLButtonDown = true;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseDown, 0, 0, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_RBUTTONDOWN:
		if (true) {
			isMouseRButtonDown = true;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseDown, 0, 1, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_MBUTTONDOWN:
		if (true) {
			isMouseWButtonDown = true;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseDown, 0, 2, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_LBUTTONUP:
		if (true) {
			isMouseLButtonDown = false;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseUp, 0, 0, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_RBUTTONUP:
		if (true) {
			isMouseRButtonDown = false;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseUp, 0, 1, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_MBUTTONUP:
		if (true) {
			isMouseWButtonDown = false;
			DKPoint pt(MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
			ConvertCoordinateOrigin(pt);
			ownerWindow->PostMouseEvent(DKWindow::EventMouseUp, 0, 2, pt, DKVector2(0, 0), false);
			::PostMessageW(windowHandle, WM_UPDATEMOUSECAPTURE, 0, 0);
		}
		break;
	case WM_MOUSEWHEEL:
		if (true) {
			POINT origin = { 0, 0 };
			::ClientToScreen(this->windowHandle, &origin);
			DKPoint pt(MAKEPOINTS(lParam).x - origin.x, MAKEPOINTS(lParam).y - origin.y);
			ConvertCoordinateOrigin(pt);

			int nDelta = GET_WHEEL_DELTA_WPARAM(wParam);

			DKVector2 delta(0, 0);
			if (nDelta > 0)
				delta.y = 1;
			else if (nDelta < 0)
				delta.y = -1;
			ownerWindow->PostMouseEvent(DKWindow::EventMouseWheel, 0, 2, pt, delta, false);
		}
		break;
	case WM_CHAR:
		if (true) {
			UpdateKeyboard();	// synchronize key states

			wchar_t c = (wchar_t)wParam;
			DKString strText(c);
			ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, strText, false);
		}
		break;
	case WM_IME_COMPOSITION:
		if (true) {
			UpdateKeyboard();	// synchronize key states

			if (lParam & GCS_RESULTSTR)		// composition finished.
			{
				// Result characters will be received via WM_CHAR,
				// reset input-candidate characters here.
				ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);
			}
			if (lParam & GCS_COMPSTR)		// composition in progress.
			{
				HIMC hIMC = ImmGetContext(windowHandle);
				if (hIMC)
				{
					if (isTextInputMode)
					{
						long nStrLen = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, 0, 0);
						if (nStrLen)
						{
							unsigned char *pStr = (unsigned char*)malloc(nStrLen + 4);
							memset(pStr, 0, nStrLen + 4);

							ImmGetCompositionStringW(hIMC, GCS_COMPSTR, pStr, nStrLen + 2);
							DKString strInputCandidate((const wchar_t*)pStr);

							free(pStr);

							ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, strInputCandidate, false);
							//	DKLog("WM_IME_COMPOSITION: '%ls'\n", strInputCandidate));
						}
						else	// composition character's length become 0. (erased)
						{
							ownerWindow->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);
						}
					}
					else	// not text-input mode.
					{
						ImmNotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
					}

					ImmReleaseContext(windowHandle, hIMC);
				}
			}
		}
		break;
	case WM_PAINT:
		if (!isResizing)
		{
			ownerWindow->PostWindowEvent(DKWindow::EventWindowUpdate, this->contentSize, this->windowOrigin, false);
		}
		break;
	case WM_DROPFILES:
		if (wParam)
		{
			HDROP hd = (HDROP)wParam;
			const DKWindow::WindowCallback& cb = ownerWindow->Callback();
			if (cb.filesDropped)
			{
				UINT numFiles = DragQueryFileW(hd, 0xFFFFFFFF, NULL, 0);

				DKWindow::WindowCallback::StringArray fileNames;
				fileNames.Reserve(numFiles);

				for (UINT i = 0; i < numFiles; ++i)
				{
					UINT len = DragQueryFileW(hd, i, NULL, 0);
					LPWSTR file = (LPWSTR)malloc( sizeof(WCHAR) * (len+2) );

					UINT r = DragQueryFileW(hd, i, file, len+1);
					file[r] = 0;

					fileNames.Add(file);

					free(file);
				}
				if (fileNames.Count() > 0)
				{
					DKPoint dragPoint(0,0);

					POINT pt;
					if (DragQueryPoint(hd, &pt))
					{
						dragPoint.x = pt.x;
						dragPoint.y = pt.y;
						this->ConvertCoordinateOrigin(dragPoint);
					}

					cb.filesDropped->Invoke(ownerWindow, dragPoint, fileNames);
				}
			}
			DragFinish(hd);
			return 0;
		}
		break;
	case WM_CLOSE:
		if (true)
		{
			const DKWindow::WindowCallback& cb = ownerWindow->Callback();
			if (cb.closeRequest)
			{
				if (!cb.closeRequest->Invoke(ownerWindow))
					return 0;
			}
			Destroy();
		}
		break;
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
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		break;
	case WM_SHOWCURSOR:
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
		break;
	case WM_UPDATEMOUSECAPTURE:
		if (::GetCapture() == windowHandle)
		{
			if (!isMouseLButtonDown && !isMouseRButtonDown && !isMouseWButtonDown && !isMouseHeld)
				::ReleaseCapture();
		}
		else
		{
			if (isMouseLButtonDown || isMouseRButtonDown || isMouseWButtonDown || isMouseHeld)
				::SetCapture(windowHandle);
		}
		break;
	}
	return ::DefWindowProcW(windowHandle, uMsg, wParam, lParam);
}

bool DKWindow_Win32::IsValid(void) const
{
	if (windowHandle && ::IsWindow(windowHandle))
		return true;
	return false;
}

DKVirtualKey DKWindow_Win32::ConvertVKey(int key)
{

	switch (key)
	{
	case 0x03:	return DKVK_F15;			// VK_F15 (ctrl+break)
	case 0x08:	return DKVK_BACKSPACE;		// VK_BACK
	case 0x09:	return DKVK_TAB;			// VK_TAB
	case 0x0D:	return DKVK_RETURN;			// VK_RETURN
	//case 0x10:	return DKVK_SHIFT;			// VK_SHIFT
	//case 0x11:	return DKVK_CONTROL;		// VK_CONTROL
	//case 0x12:	return DKVK_OPTION;			// VK_MENU
	case 0x13:	return DKVK_F15;			// VK_PAUSE
	case 0x14:	return DKVK_CAPSLOCK;		// VK_CAPITAL, CAPSLOCK
	case 0x1B:	return DKVK_ESCAPE;			// VK_ESCAPE
	case 0x1C:	return DKVK_NONE;			// VK_CONVERT
	case 0x1D:	return DKVK_NONE;			// VK_NONCONVERT
	case 0x1E:	return DKVK_NONE;			// VK_ACCEPT
	case 0x1F:	return DKVK_NONE;			// VK_MODECHANGE
	case 0x20:	return DKVK_SPACE;			// VK_SPACE
	case 0x21:	return DKVK_PAGE_UP;		// VK_PRIOR
	case 0x22:	return DKVK_PAGE_DOWN;		// VK_NEXT
	case 0x23:	return DKVK_END;			// VK_END
	case 0x24:	return DKVK_HOME;			// VK_HOME
	case 0x25:	return DKVK_LEFT;			// VK_LEFT
	case 0x26:	return DKVK_UP;				// VK_UP
	case 0x27:	return DKVK_RIGHT;			// VK_RIGHT
	case 0x28:	return DKVK_DOWN;			// VK_DOWN
	case 0x29:	return DKVK_NONE;			// VK_SELECT
	case 0x2A:	return DKVK_NONE;			// VK_PRINT
	case 0x2B:	return DKVK_NONE;			// VK_EXECUTE
	case 0x2C:	return DKVK_F13;			// VK_SNAPSHOT, PRINT SCREEN KEY
	case 0x2D:	return DKVK_INSERT;			// VK_INSERT
	case 0x2E:	return DKVK_DELETE;			// VK_DELETE
	case 0x2F:	return DKVK_NONE;			// VK_HELP
	case 0x30:	return DKVK_0;				// 0
	case 0x31:	return DKVK_1;				// 1
	case 0x32:	return DKVK_2;				// 2
	case 0x33:	return DKVK_3;				// 3
	case 0x34:	return DKVK_4;				// 4
	case 0x35:	return DKVK_5;				// 5
	case 0x36:	return DKVK_6;				// 6
	case 0x37:	return DKVK_7;				// 7
	case 0x38:	return DKVK_8;				// 8
	case 0x39:	return DKVK_9;				// 9

	case 0x41:	return DKVK_A;				// A
	case 0x42:	return DKVK_B;				// B
	case 0x43:	return DKVK_C;				// C
	case 0x44:	return DKVK_D;				// D
	case 0x45:	return DKVK_E;				// E
	case 0x46:	return DKVK_F;				// F
	case 0x47:	return DKVK_G;				// G
	case 0x48:	return DKVK_H;				// H
	case 0x49:	return DKVK_I;				// I
	case 0x4A:	return DKVK_J;				// J
	case 0x4B:	return DKVK_K;				// K
	case 0x4C:	return DKVK_L;				// L
	case 0x4D:	return DKVK_M;				// M
	case 0x4E:	return DKVK_N;				// N
	case 0x4F:	return DKVK_O;				// O
	case 0x50:	return DKVK_P;				// P
	case 0x51:	return DKVK_Q;				// Q
	case 0x52:	return DKVK_R;				// R
	case 0x53:	return DKVK_S;				// S
	case 0x54:	return DKVK_T;				// T
	case 0x55:	return DKVK_U;				// U
	case 0x56:	return DKVK_V;				// V
	case 0x57:	return DKVK_W;				// W
	case 0x58:	return DKVK_X;				// X
	case 0x59:	return DKVK_Y;				// Y
	case 0x5A:	return DKVK_Z;				// Z
	case 0x5B:	return DKVK_LEFT_COMMAND;		// VK_LWIN
	case 0x5C:	return DKVK_RIGHT_COMMAND;		// VK_RWIN
	case 0x5D:	return DKVK_NONE;			// VK_APPS
	case 0x5F:	return DKVK_NONE;			// VK_SLEEP
	case 0x60:	return DKVK_PAD_0;			// VK_NUMPAD0
	case 0x61:	return DKVK_PAD_1;			// VK_NUMPAD1
	case 0x62:	return DKVK_PAD_2;			// VK_NUMPAD2
	case 0x63:	return DKVK_PAD_3;			// VK_NUMPAD3
	case 0x64:	return DKVK_PAD_4;			// VK_NUMPAD4
	case 0x65:	return DKVK_PAD_5;			// VK_NUMPAD5
	case 0x66:	return DKVK_PAD_6;			// VK_NUMPAD6
	case 0x67:	return DKVK_PAD_7;			// VK_NUMPAD7
	case 0x68:	return DKVK_PAD_8;			// VK_NUMPAD8
	case 0x69:	return DKVK_PAD_9;			// VK_NUMPAD9
	case 0x6A:	return DKVK_PAD_ASTERISK;	// VK_MULTIPLY
	case 0x6B:	return DKVK_PAD_PLUS;		// VK_ADD
	case 0x6C:	return DKVK_NONE;			// VK_SEPARATOR
	case 0x6D:	return DKVK_PAD_MINUS;		// VK_SUBTRACT
	case 0x6E:	return DKVK_PAD_PERIOD;		// VK_DECIMAL
	case 0x6F:	return DKVK_PAD_SLASH;		// VK_DIVIDE
	case 0x70:	return DKVK_F1;				// VK_F1
	case 0x71:	return DKVK_F2;				// VK_F2
	case 0x72:	return DKVK_F3;				// VK_F3
	case 0x73:	return DKVK_F4;				// VK_F4
	case 0x74:	return DKVK_F5;				// VK_F5
	case 0x75:	return DKVK_F6;				// VK_F6
	case 0x76:	return DKVK_F7;				// VK_F7
	case 0x77:	return DKVK_F8;				// VK_F8
	case 0x78:	return DKVK_F9;				// VK_F9
	case 0x79:	return DKVK_F10;			// VK_F10
	case 0x7A:	return DKVK_F11;			// VK_F11
	case 0x7B:	return DKVK_F12;			// VK_F12
	case 0x7C:	return DKVK_F13;			// VK_F13
	case 0x7D:	return DKVK_F14;			// VK_F14
	case 0x7E:	return DKVK_F15;			// VK_F15
	case 0x7F:	return DKVK_F16;			// VK_F16
	case 0x80:	return DKVK_F17;			// VK_F17
	case 0x81:	return DKVK_F18;			// VK_F18
	case 0x82:	return DKVK_F19;			// VK_F19
	case 0x83:	return DKVK_F20;			// VK_F20
	case 0x84:	return DKVK_NONE;			// VK_F21
	case 0x85:	return DKVK_NONE;			// VK_F22
	case 0x86:	return DKVK_NONE;			// VK_F23
	case 0x87:	return DKVK_NONE;			// VK_F24

	case 0x90:	return DKVK_NUMLOCK;		// VK_NUMLOCK
	case 0x91:	return DKVK_F14;			// VK_SCROLL, SCROLL LOCK

	case 0xA0:	return DKVK_LEFT_SHIFT;		// VK_LSHIFT
	case 0xA1:	return DKVK_RIGHT_SHIFT;	// VK_RSHIFT
	case 0xA2:	return DKVK_LEFT_CONTROL;	// VK_LCONTROL
	case 0xA3:	return DKVK_RIGHT_CONTROL;	// VK_RCONTROL
	case 0xA4:	return DKVK_LEFT_OPTION;	// VK_LMENU
	case 0xA5:	return DKVK_RIGHT_OPTION;	// VK_RMENU

	case 0xBA:	return DKVK_SEMICOLON;		// VK_OEM_1, ;
	case 0xBB:	return DKVK_EQUAL;			// VK_OEM_PLUS, =
	case 0xBC:	return DKVK_COMMA;			// VK_OEM_COMMA, .
	case 0xBD:	return DKVK_HYPHEN;			// VK_OEM_MINUS, -
	case 0xBE:	return DKVK_PERIOD;			// VK_OEM_PERIOD
	case 0xBF:	return DKVK_SLASH;			// VK_OEM_2, /?
	case 0xC0:	return DKVK_ACCENT_TILDE;	// VK_OEM_3, `~

	case 0xDB:	return DKVK_OPEN_BRACKET;	// VK_OEM_4, [
	case 0xDC:	return DKVK_BACKSLASH;		// VK_OEM_5, backslash
	case 0xDD:	return DKVK_CLOSE_BRACKET;	// VK_OEM_6, ]
	case 0xDE:	return DKVK_QUOTE;			// VK_OEM_7, '
	case 0xDF:	return DKVK_NONE;			// VK_OEM_8

	case 0xE2:	return DKVK_BACKSLASH;		// VK_OEM_102, backslash for 102-keyboard

	case 0xE5:	return DKVK_NONE;			// VK_PROCESSKEY, IME-key

	default:	return DKVK_NONE;

	}
	return DKVK_NONE;
}


DKPoint& DKWindow_Win32::ConvertCoordinateOrigin(DKPoint& pt) const
{
	if (windowHandle)
	{
		RECT rc;
		::GetClientRect(windowHandle, &rc);
		float y = rc.bottom - rc.top - pt.y;
		pt.y = y;

		return pt;
	}
	return pt;
}

#endif // ifdef WIN32
