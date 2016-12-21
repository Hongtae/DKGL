//
//  File: DKWindow.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKVKey.h"

namespace DKFramework
{
	class DKWindowInterface;
	/// @brief System GUI window class.
	/// 
	/// You can create GUI window with this class.
	/// You can also create proxy-window for window that was created by
	/// other frameworks or libraries.
	///
	/// Call Create() to create your own window, or call CreateProxy() to proxy-
	/// window for wrapping DKWindow to existing window.
	/// A proxy window is very useful to use small part of application, which need
	/// to render with DK,
	///
	/// origin of content coordinates is lower left corner.
	///
	/// @note
	///   This class using system provided GUI system window.
	///   Microsoft Windows, Apple Mac OS X, Apple iOS are supported by default.
	///   You need to implement window interface class to use on other platforms.
	///   (see DKFramework/Interface/DKWindowInterface.h)
	/// @see DKWindowInterface
	class DKGL_API DKWindow
	{
	public:
		DKWindow(void);
		~DKWindow(void);

		/// Window Style
		enum Style : uint32_t
		{
			StyleTitle = 1,
			StyleCloseButton = 1 << 1,
			StyleMinimizeButton = 1 << 2,
			StyleMaximizeButton = 1 << 3,
			StyleResizableBorder = 1 << 4,
			StyleAutoResize = 1 << 5,	///< resize on rotate or DPI change, etc.
			StyleGeneralWindow = 0xff,	///< includes all but StyleAcceptFileDrop
			StyleAcceptFileDrop = 1 << 8, ///< enables file drag & drop
		};

		/// Mouse, (Multi) Touch, Table Stylus pen event.
		struct MouseEvent
		{
			enum Type
			{
				ButtonDown = 0,
				ButtonUp,
				Move,
				Wheel,
				Pointing,
			};
			enum Device
			{
				GenericMouse = 0,
				Stylus,
				Touch,
				Unknown,
			};
			Type type;
			Device device;
			int deviceId;
			int buttonId;
			DKPoint location;	///< location in window
			DKVector2 delta;
			float pressure;		///< for stylus-pen
			float tilt;///< radian value of stylus-pen and screen angle. 0 is parallel, PI/2 is perpendicular to the surface.
		};

		/// Keyboard and text event.
		struct KeyboardEvent
		{
			enum Type
			{
				KeyDown = 0,
				KeyUp,
				TextInput,
				TextComposition, ///< text composition in progress
			};
			Type type;
			int deviceId;
			DKVirtualKey key;
			DKString text;
		};

		/// Window event, reposition, resize etc.
		struct WindowEvent
		{
			enum Type
			{
				WindowCreated = 0,	///< this event will not be distributed automatically.
				WindowClosed,
				WindowHidden,
				WindowShown,
				WindowActivated,
				WindowInactivated,
				WindowMinimized,
				WindowMoved,
				WindowResized,
				WindowUpdate,
			};
			Type type;
			DKRect windowRect;	///< on parent space coords (can be screen space)
			DKRect contentRect;	///< on parent space coords (can be screen space)
			double contentScaleFactor;
		};

		// function or function object type for event handlers.
		typedef void WindowProc(const WindowEvent&); ///< Windoe event handler function type
		typedef void KeyboardProc(const KeyboardEvent&); ///< Keyboard event handler function type
		typedef void MouseProc(const MouseEvent&); ///< Mouse event handler function type

		typedef DKFunctionSignature<WindowProc> WindowEventHandler;
		typedef DKFunctionSignature<KeyboardProc> KeyboardEventHandler;
		typedef DKFunctionSignature<MouseProc> MouseEventHandler;

		/// @brief Window Callback
		/// @details Callback function is required for some events that cannot be
		/// processed asynchronously.
		struct WindowCallback
		{
			template <typename T> using Function = DKObject<DKFunctionSignature<T>>;
			using StringArray = DKArray<DKString>;

			Function<void (DKWindow*, const DKPoint&, const StringArray&)> filesDropped;
			Function<DKSize (DKWindow*)> contentMinSize;
			Function<DKSize (DKWindow*)> contentMaxSize;
			Function<bool (DKWindow*)> closeRequest;
		};

		/// Create window with system default size, position and hidden state.
		static DKObject<DKWindow> Create(const DKString& name,		// window title
										 int style = StyleGeneralWindow,			// window style
										 const WindowCallback& cb = WindowCallback());
		/// Create proxy window. (can be used to interface of existing window)
		/// You have to post all events manually.
		/// @param systemHandle HWND for Win32, NSView for macOS, UIView subclass for iOS
		/// @note
		///  on iOS systemHandle should be UIView subclass object.
		///  [UIView layerClass] should return CAMetalLayer, otherwise 
		///  function will fail.
		static DKObject<DKWindow> CreateProxy(void* systemHandle);
		void UpdateProxy(void);							///< update proxy window status
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
		void SetTitle(const DKString&);
		DKString Title(void) const;

		/// add event handler.
		/// (a event that can be processed asynchronously, and dont need to response)
		using EventHandlerContext = const void*;
		void AddEventHandler(EventHandlerContext context, WindowEventHandler*, KeyboardEventHandler*, MouseEventHandler*);
		void RemoveEventHandler(EventHandlerContext context);

		// control keyboard state
		// Keyboard has two states: raw-key input mode, text composition mode.
		bool IsTextInputEnabled(int deviceId) const;
		void SetTextInputEnabled(int deviceId, bool enabled); ///< change keyboard for text input mode
		bool KeyState(int deviceId, const DKVirtualKey& k) const;
		void SetKeyState(int deviceId, const DKVirtualKey& k, bool down);
		void ResetKeyState(int deviceId);
		void ResetKeyStateForAllDevices(void);
		static DKString GetVKName(DKVirtualKey lKey); ///< get key name for debugging

		// control mouse state
		void ShowMouse(int deviceId, bool bShow);
		void HoldMouse(int deviceId, bool bHold); ///< hold position, but still receiving delta.
		bool IsMouseVisible(int deviceId) const;
		bool IsMouseHeld(int deviceId) const;
		DKPoint MousePosition(int deviceId) const;
		void SetMousePosition(int deviceId, const DKPoint& pt);

		// window state
		DKRect WindowRect(void) const;			///< window's rect (OS coords unit)
		DKRect ContentRect(void) const;			///< content rect on window (OS coords unit)
		double ContentScaleFactor(void) const;	///< content unit scale, pixel ratio

		bool IsVisible(void) const	{ return visible; }
		bool IsActive(void) const	{ return activated; }
		bool IsValid(void) const;

		/// HWD for Win32, NSView for Cocoa, UIView for CocoaTouch.
		/// On macOS/iOS, You can reuse view (NSView/UIView).
		/// You can detach from window (NSWindow/UIWindow) and attach to other window.
		void* PlatformHandle(void) const;

		// call event handler manually.
		void PostMouseEvent(const MouseEvent&);	///< call mouse event handlers manually
		void PostKeyboardEvent(const KeyboardEvent&); ///< call keyboard event handlers manually
		void PostWindowEvent(const WindowEvent&); ///< call window event handlers manually

	private:
		DKSpinLock handlerLock;
		DKMap<EventHandlerContext, DKObject<WindowEventHandler>> windowEventHandlers;
		DKMap<EventHandlerContext, DKObject<KeyboardEventHandler>> keyboardEventHandlers;
		DKMap<EventHandlerContext, DKObject<MouseEventHandler>> mouseEventHandlers;

		struct KeyboardState
		{
			unsigned char keyStateBits[DKVK_MAXVALUE / 8 + 1]; // save raw-key state (1:down, 0:up)
		};
		mutable DKMap<int, KeyboardState>		keyboardStateMap;
		KeyboardState& GetKeyboardState(int deviceId) const;	// Get key states without lock
		DKSpinLock keyboardLock;

		DKRect windowRect;		// window's origin, size (including border, system coordinates)
		DKRect contentRect;		// content origin, size in window space (or parent view space)
		bool activated;			// true if window is activated
		bool visible;

		WindowCallback callback;
		DKWindowInterface*	impl;
	};
}
