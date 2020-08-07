//
//  File: DKScreen.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKFrame.h"
#include "DKCommandQueue.h"
#include "DKGraphicsDevice.h"
#include "DKAudioDevice.h"
#include "DKCanvas.h"

namespace DKFramework
{
	/// @brief screen object, which render it's frames (DKFrame) to window surface.
	///
	/// screen has one EventLoop internally, to process events, messages, operations
	/// asynchronous. (delivers event to target frame)
	class DKGL_API DKScreen
	{
	public:
		DKScreen(DKWindow*, DKFrame*);
		~DKScreen();

		/// Create rendering thread and start event-loop
		void Start();
		void Pause();
		void Resume();
		void Stop();

		DKWindow* Window()		{ return window; }
		DKFrame* RootFrame()	{ return rootFrame; }

		DKObject<DKCanvas> CreateCanvas() const;
		DKSize Resolution() const;

		// key frame, focus frame
		// make specified frame can access device exclusively.
        bool SetKeyFrame(int deviceId, DKFrame* frame);
        bool SetFocusFrame(int deviceId, DKFrame* frame);
        // revoke access device from specified frame. (no event generated)
        bool RemoveKeyFrame(int deviceId, DKFrame* frame);
        bool RemoveFocusFrame(int deviceId, DKFrame* frame);
        // query frame which can access device exclusively.
        DKFrame* KeyFrame(int deviceId);
        const DKFrame* KeyFrame(int deviceId) const;
        DKFrame* FocusFrame(int deviceId);
        const DKFrame* FocusFrame(int deviceId) const;

        // revoke access to any devices from frame. (event generated)
        void RemoveKeyFrameForAnyDevices(DKFrame* frame, bool notify);
        void RemoveFocusFrameForAnyDevices(DKFrame* frame, bool notify);
        // revoke access to any devices from all frames. (event generated)
        void RemoveAllKeyFramesForAnyDevices(bool notify);
        void RemoveAllFocusFramesForAnyDevices(bool notify);

        // convert coordinates
        DKPoint WindowToScreen(const DKPoint& pt) const;
        DKPoint ScreenToWindow(const DKPoint& pt) const;
        DKSize WindowToScreen(const DKSize& size) const;
        DKSize ScreenToWindow(const DKSize& size) const;
        DKRect WindowToScreen(const DKRect& rect) const;
        DKRect ScreenToWindow(const DKRect& rect) const;


		DKCommandQueue* CommandQueue() { return commandQueue;} ///< main command queue
		DKEventLoop* EventLoop() {return eventLoop;} ///< event loop for render thread

	private:
		void Draw() const;
		static void EventLoopIdle(DKScreen*, DKEventLoop*);

		DKObject<DKEventLoop> eventLoop;
		DKObject<DKWindow> window;
		DKObject<DKFrame> rootFrame;

        DKMap<int, DKFrame*> keyboardHolders;
        DKMap<int, DKFrame*> mouseHolders;

		DKObject<DKThread> thread;	// rendering thread
		DKObject<DKCommandQueue> commandQueue;
		DKObject<DKGraphicsDevice> graphicsDevice;
		DKObject<DKAudioDevice> audioDevice;

		DKSize screenResolution;
	};
}
