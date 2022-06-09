//
//  File: DKScreen.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKFrame.h"
#include "DKAudioDevice.h"
#include "DKCanvas.h"
#include "DKGraphicsDeviceContext.h"

namespace DKFramework
{
	/// @brief screen object, which render it's frames (DKFrame) to window surface.
	///
	/// screen has one EventLoop internally, to process events, messages, operations
	/// asynchronous. (delivers event to target frame)
	class DKGL_API DKScreen
	{
	public:
        DKScreen();
		DKScreen(DKGraphicsDeviceContext*, DKDispatchQueue*);
		~DKScreen();

		/// Pause/Resume thread
		void Pause();
		void Resume();

        void SetWindow(DKWindow*);
		DKWindow* Window()		{ return window; }
        void SetRootFrame(DKFrame*);
		DKFrame* RootFrame()	{ return rootFrame; }

        /// create canvas object for root frame (screen-surface, non-render-target)
		DKObject<DKCanvas> CreateCanvas();
		DKSize Resolution() const;
        float ContentScaleFactor() const;

		/// make specified frame can access keyboard device exclusively.
        bool SetKeyFrame(int deviceId, DKFrame* frame);
        /// make specified frame can access mouse device exclusively.
        bool SetFocusFrame(int deviceId, DKFrame* frame);
        /// revoke access keyboard device from specified frame. (no event generated)
        bool RemoveKeyFrame(int deviceId, DKFrame* frame);
        /// revoke access mouse device from specified frame. (no event generated)
        bool RemoveFocusFrame(int deviceId, DKFrame* frame);
        /// query frame which can access device exclusively.
        DKFrame* KeyFrame(int deviceId);
        const DKFrame* KeyFrame(int deviceId) const;
        DKFrame* FocusFrame(int deviceId);
        const DKFrame* FocusFrame(int deviceId) const;

        /// Revoke exclusive access to keyboard.
        void RemoveKeyFrameForAnyDevices(DKFrame* frame, bool notify);
        /// Revoke exclusive access to mouse from frame.
        void RemoveFocusFrameForAnyDevices(DKFrame* frame, bool notify);
        /// revoke exclusive access to keyboard from all frames.
        void RemoveAllKeyFramesForAnyDevices(bool notify);
        /// revoke exclusive access to mouse from all frames.
        void RemoveAllFocusFramesForAnyDevices(bool notify);

        /// query mouse hover frame for mouse-deviceId
        const DKFrame* HoverFrame(int deviceId) const;
        /// release hover frame for all devices.
        void LeaveHoverFrame(DKFrame* frame);

        /// convert coordinates
        DKPoint WindowToScreen(const DKPoint& pt) const;
        DKPoint ScreenToWindow(const DKPoint& pt) const;
        DKSize WindowToScreen(const DKSize& size) const;
        DKSize ScreenToWindow(const DKSize& size) const;
        DKRect WindowToScreen(const DKRect& rect) const;
        DKRect ScreenToWindow(const DKRect& rect) const;

        bool ProcessKeyboardEvent(const DKWindow::KeyboardEvent&);
        bool ProcessMouseEvent(const DKWindow::MouseEvent&);
        bool ProcessWindowEvent(const DKWindow::WindowEvent&);

        DKGraphicsDeviceContext* GraphicsDevice();
        DKCommandQueue* CommandQueue();
	private:
        DKObject<DKDispatchQueue> dispatchQueue;
        DKObject<DKWindow> window;
        DKObject<DKFrame> rootFrame;

        DKMap<int, DKFrame*> keyFrames; // keyboard captors
        DKMap<int, DKFrame*> focusFrames; // mouse 
        struct FrameMouseDevice { DKFrame* frame; DKWindow::MouseEvent::Device device; };
        DKMap<int, FrameMouseDevice> hoverFrames; // mouse hover frames

        DKObject<DKGraphicsDeviceContext> graphicsDevice;
        DKObject<DKAudioDevice> audioDevice;
        DKObject<DKCommandQueue> commandQueue;
        DKObject<DKSwapChain> swapChain;

        DKSize screenResolution;

        bool activated;
        bool visible;

        bool running;
        bool paused;

        float frameInterval;
        float inactiveFrameInterval;
        DKObject<DKThread> thread;
        DKCondition condition;
        DKLock frameGuard;

        DKScreen(const DKScreen&) = delete;
        DKScreen& operator = (const DKScreen&) = delete;
	};
}
