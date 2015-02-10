//
//  File: DKScreen.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKRenderer.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKOpenALContext.h"
#include "DKOpenGLContext.h"
#include "DKFrame.h"

////////////////////////////////////////////////////////////////////////////////
// DKScreen
// screen object, which render it's frames (DKFrame) to window surface.
// screen has one RunLoop internally, to process events, messages, operations
// asynchronous. (delivers event to target frame)
////////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKLIB_API DKScreen : public DKFoundation::DKRunLoop
	{
		friend class DKFoundation::DKObject<DKScreen>;
	public:
		DKScreen(void);
		~DKScreen(void);

		bool Run(DKWindow* window, DKFrame* rootFrame);

		DKFoundation::DKRunLoop* RunLoop(void) const;

		// system window (initial render target)
		DKWindow* Window(void);
		const DKWindow* Window(void) const;

		// root frame (all events, drawing begins with root-frame)
		DKFrame* RootFrame(void);
		const DKFrame* RootFrame(void) const;

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

		// post render event. (will not render immediately)
		void Render(bool forced = false);
		void Suspend(void);               // suspend render
		bool IsSuspended(void) const;
		void Resume(void);                // resume render

		const DKSize& ScreenResolution(void) const;

		// convert coordinates
		DKPoint WindowToScreen(const DKPoint& pt) const;
		DKPoint ScreenToWindow(const DKPoint& pt) const;
		DKSize WindowToScreen(const DKSize& size) const;
		DKSize ScreenToWindow(const DKSize& size) const;
		DKRect WindowToScreen(const DKRect& rect) const;
		DKRect ScreenToWindow(const DKRect& rect) const;

		void RegisterAutoUnloadFrame(DKFrame* frame);   // Don't use. used internally.
		void UnregisterAutoUnloadFrame(DKFrame* frame); // Don't use. used internally.

		void SetActiveFrameLatency(double f);
		void SetInactiveFrameLatency(double f);
		double ActiveFrameLatency(void) const;
		double InactiveFrameLatency(void) const;
		
	protected:
		void OnInitialize(void) override;
		void OnTerminate(void) override;
		void OnIdle(void) override;
		void PerformOperation(const DKFoundation::DKOperation* operation) override;

		void OnWindowEvent(DKWindow::EventWindow type, DKSize contentSize, DKPoint windowOrigin);
		void OnMouseEvent(DKWindow::EventMouse type, int deviceId, int buttonId, DKPoint pos, DKVector2 delta);
		void OnKeyboardEvent(DKWindow::EventKeyboard type, int deviceId, DKVirtualKey key, DKFoundation::DKString text);

	private:
		DKFoundation::DKObject<DKRenderer>		renderer;
		DKFoundation::DKMap<int, DKFrame*>		keyboardHolders;
		DKFoundation::DKMap<int, DKFrame*>		mouseHolders;
		DKFoundation::DKObject<DKWindow>		window;
		DKFoundation::DKObject<DKFrame>			rootFrame;

		DKFoundation::DKSet<DKFrame*, DKFoundation::DKSpinLock>		autoUnloadFrames; // unload when screen being terminated.

		bool						suspended;
		bool						activated;
		bool						visible;
		double						tickDelta;
		DKFoundation::DKDateTime	tickDate;
		DKFoundation::DKTimeTick	tickCount;
		DKFoundation::DKTimer		timer;
		double						activeFrameLatency;
		double						inactiveFrameLatency;
		DKSize						screenResolution;

		DKFoundation::DKObject<DKOpenALContext> alContext;
		DKFoundation::DKObject<DKOpenGLContext> glContext;
		
		void RenderScreen(bool invalidate);
	};
}
