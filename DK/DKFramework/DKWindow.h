//
//  File: DKWindow.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKVKey.h"

////////////////////////////////////////////////////////////////////////////////
// DKWindow
// System GUI window class. You can create GUI window with this class.
// You can also create proxy-window for window that was created by
// other frameworks or libraries.
//
// Call Create() to create your own window, or call CreateProxy() to proxy-
// window for wrapping DKWindow to existing window.
// A proxy window is very useful to use small part of application, which need
// to render with DK,
//
// origin of content coordinates is lower left corner.
//
// Note:
//   This class using system provided GUI system window.
//   Microsoft Windows, Apple Mac OS X, Apple iOS are supported by default.
//   You need to implement window interface class to use on other platforms.
//   (see DKFramework/Interface/DKWindowInterface.h)
//
////////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKWindowInterface;
	class DKLIB_API DKWindow
	{
	public:
		DKWindow(void);
		~DKWindow(void);

		enum Style : int // Window style
		{
			StyleTitle = 1,
			StyleCloseButton = 1 << 1,
			StyleMinimizeButton = 1 << 2,
			StyleMaximizeButton = 1 << 3,
			StyleResizableBorder = 1 << 4,
			StyleAcceptFileDrop = 1 << 8,
			StyleGeneralWindow = 0xff,	// all above without StyleAcceptFileDrop
		};

		enum EventMouse // mouse event
		{
			EventMouseNothing = 0,
			EventMouseDown,
			EventMouseUp,
			EventMouseMove,
			EventMouseWheel,
		};
		enum EventKeyboard  // keyboard event
		{
			EventKeyboardNothing = 0,
			EventKeyboardDown,
			EventKeyboardUp,
			EventKeyboardTextInput,
			EventKeyboardTextInputCandidate, // text composition in progress
		};
		enum EventWindow // window event
		{
			EventWindowCreated = 0,
			EventWindowClosed,
			EventWindowHidden,
			EventWindowShown,
			EventWindowActivated,
			EventWindowInactivated,
			EventWindowMinimized,
			EventWindowMoved,				
			EventWindowResized,			
			EventWindowUpdate,
		};
		// function or function object type for event handlers.
		typedef void WindowProc(EventWindow, DKSize, DKPoint);
		typedef void KeyboardProc(EventKeyboard, int, DKVirtualKey, DKFoundation::DKString);
		typedef void MouseProc(EventMouse, int, int, DKPoint, DKVector2);

		typedef DKFoundation::DKFunctionSignature<WindowProc> WindowEventHandler;
		typedef DKFoundation::DKFunctionSignature<KeyboardProc> KeyboardEventHandler;
		typedef DKFoundation::DKFunctionSignature<MouseProc> MouseEventHandler;

		// Window Callback
		// Callback function is required for some events that cannot be
		// processed asynchronously.
		struct WindowCallback
		{
			template <typename T> using Function = DKFoundation::DKObject<DKFoundation::DKFunctionSignature<T>>;
			using StringArray = DKFoundation::DKArray<DKFoundation::DKString>;

			Function<void (DKWindow*, const DKPoint&, const StringArray&)> filesDropped;
			Function<DKSize (DKWindow*)> contentMinSize;
			Function<DKSize (DKWindow*)> contentMaxSize;
			Function<bool (DKWindow*)> closeRequest;
		};

		// Create window.
		// if origin is undefinedOrigin, then using OS default value.
		// if contentSize is smaller than 1, then using OS default value.
		static const DKPoint undefinedOrigin;
		static DKFoundation::DKObject<DKWindow> Create(const DKFoundation::DKString& title,		// window title
													   const DKSize& contentSize,				// content size (system coordinates)
													   const DKPoint& origin = undefinedOrigin,	// window origin (system coordinates)
													   int style = StyleGeneralWindow,			// window style
													   const WindowCallback& cb = WindowCallback());
		// Create proxy window. (can be used to interface of existing window)
		static DKFoundation::DKObject<DKWindow> CreateProxy(void* systemHandle);
		void UpdateProxy(void);							// update proxy window status
		bool IsProxy(void) const;
		void SetCallback(const WindowCallback& cb);
		const WindowCallback& Callback(void) const;

		void Close(void);
		void Show(void);
		void Hide(void);
		void Activate(void);
		void Minimize(void);
		void Resize(const DKSize&);
		void Resize(const DKSize&, const DKPoint&);
		void SetOrigin(const DKPoint&);
		void SetTitle(const DKFoundation::DKString&);
		DKFoundation::DKString Title(void) const;

		// add event handler.
		// (a event that can be processed asynchronously, and dont need to response)
		void AddObserver(void* context, WindowEventHandler*, KeyboardEventHandler*, MouseEventHandler*, DKFoundation::DKRunLoop*);
		void RemoveObserver(void* context);

		// control keyboard state
		// Keyboard has two states: raw-key input mode, text composition mode.
		bool IsTextInputEnabled(int deviceId) const;
		void SetTextInputEnabled(int deviceId, bool enabled);
		bool KeyState(int deviceId, const DKVirtualKey& k) const;
		void SetKeyState(int deviceId, const DKVirtualKey& k, bool down);
		void ResetKeyState(int deviceId);
		void ResetKeyStateForAllDevices(void);
		static DKFoundation::DKString GetVKName(DKVirtualKey lKey); // get key name for debugging

		// control mouse state
		void ShowMouse(int deviceId, bool bShow);
		void HoldMouse(int deviceId, bool bHold); // hold position, but still receiving delta.
		bool IsMouseVisible(int deviceId) const;
		bool IsMouseHeld(int deviceId) const;
		DKPoint MousePosition(int deviceId) const;
		void SetMousePosition(int deviceId, const DKPoint& pt);

		// window state
		DKPoint Origin(void) const;				// window's origin (OS coords unit)
		DKSize ContentSize(void) const;			// content size (OS coords unit)
		double ContentScaleFactor(void) const;	// content unit, pixel ratio

		bool IsVisible(void) const					{return visible;}
		bool IsActive(void) const					{return activated;}
		bool IsValid(void) const;

		// platform handle:
		// HWD for Win32, NSView/NSWindow for Cocoa, UIView for CocoaTouch.
		void* PlatformHandle(void) const;

		// call event handler manually.
		void PostMouseEvent(EventMouse type, int deviceId, int buttonId, const DKPoint& pos, const DKVector2& delta, bool sync = false);
		void PostKeyboardEvent(EventKeyboard type, int deviceId, DKVirtualKey key, const DKFoundation::DKString& textInput, bool sync = false);
		void PostWindowEvent(EventWindow type, const DKSize& contentSize, const DKPoint& windowOrigin, bool sync = false);

	private:
		DKFoundation::DKCallback<WindowProc, void*, DKFoundation::DKSpinLock>		windowEventHandlers;
		DKFoundation::DKCallback<KeyboardProc, void*, DKFoundation::DKSpinLock>		keyboardEventHandlers;
		DKFoundation::DKCallback<MouseProc, void*, DKFoundation::DKSpinLock>		mouseEventHandlers;

		struct KeyboardState
		{
			bool textInputEnabled;  // keyboard input state (key input or text input)
			unsigned char keyStateBits[DKVK_MAXVALUE / 8 + 1]; // save raw-key state (1:down, 0:up)
		};
		mutable DKFoundation::DKMap<int, KeyboardState>		keyboardStateMap;
		KeyboardState& GetKeyboardState(int deviceId) const;	// Get key states without lock
		DKFoundation::DKSpinLock keyboardLock;

		DKPoint origin;			// window's origin (including border, system coordinates)
		DKSize contentSize;		// content size
		bool activated;			// true if window is activated
		bool visible;

		WindowCallback callback;
		DKWindowInterface*	impl;
	};
}
