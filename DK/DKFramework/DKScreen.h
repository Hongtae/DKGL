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

		DKCommandQueue* CommandQueue() { return commandQueue;} ///< main command queue
		DKEventLoop* EventLoop() {return eventLoop;} ///< event loop for render thread

	private:
		void DrawFrame();
		static void EventLoopIdle(DKScreen*, DKEventLoop*);

		DKObject<DKEventLoop> eventLoop;
		DKObject<DKWindow> window;
		DKObject<DKFrame> rootFrame;
		DKObject<DKThread> thread;	// rendering thread
		DKObject<DKCommandQueue> commandQueue;
		DKObject<DKGraphicsDevice> graphicsDevice;
		DKObject<DKAudioDevice> audioDevice;
	};
}
