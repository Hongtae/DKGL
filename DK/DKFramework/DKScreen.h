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
		~DKScreen(void);

		/// Create rendering thread and start event-loop
		void Start(void);
		void Pause(void);
		void Resume(void);
		void Stop(void);

		DKWindow* Window(void)		{ return window; }
		DKFrame* RootFrame(void)	{ return rootFrame; }

	private:
		void DrawFrame(void);
		static void EventLoopIdle(DKScreen*, DKEventLoop*);

		DKObject<DKEventLoop> eventLoop;
		DKObject<DKWindow> window;
		DKObject<DKFrame> rootFrame;
		DKObject<DKThread> thread;	// rendering thread
	};
}
