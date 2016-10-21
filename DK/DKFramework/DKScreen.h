//
//  File: DKScreen.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKWindow.h"
#include "DKRenderer.h"
#include "DKPoint.h"
#include "DKSize.h"
#include "DKAudioDevice.h"
#include "DKOpenGLContext.h"
#include "DKFrame.h"

////////////////////////////////////////////////////////////////////////////////
// DKScreen
// screen object, which render it's frames (DKFrame) to window surface.
// screen has one RunLoop internally, to process events, messages, operations
// asynchronous. (delivers event to target frame)
////////////////////////////////////////////////////////////////////////////////


namespace DKGL
{
	class DKGL_API DKScreen : public DKRunLoop
	{
		friend class DKObject<DKScreen>;
	public:
		DKScreen(void);
		~DKScreen(void);

		bool Run(DKWindow* window, DKFrame* rootFrame);

		DKRunLoop* RunLoop(void) const;

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

		void Stop(bool wait);		// terminate render-thread

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
		virtual void Initialize(void);
		virtual void Finalize(void);
		void OnIdle(void) override;
		void PerformOperation(const DKOperation* operation) override;

		void OnWindowEvent(DKWindow::EventWindow type, DKSize contentSize, DKPoint windowOrigin);
		void OnMouseEvent(DKWindow::EventMouse type, int deviceId, int buttonId, DKPoint pos, DKVector2 delta);
		void OnKeyboardEvent(DKWindow::EventKeyboard type, int deviceId, DKVirtualKey key, DKString text);

		void Stop(void) override { Stop(true); }
	private:
		DKObject<DKThread>			renderThread;
		DKObject<DKRenderer>		renderer;
		DKMap<int, DKFrame*>		keyboardHolders;
		DKMap<int, DKFrame*>		mouseHolders;
		DKObject<DKWindow>		window;
		DKObject<DKFrame>			rootFrame;

		DKSet<DKFrame*, DKSpinLock>		autoUnloadFrames; // unload when screen being terminated.

		bool						suspended;
		bool						activated;
		bool						visible;
		double						tickDelta;
		DKDateTime	tickDate;
		DKTimeTick	tickCount;
		DKTimer		timer;
		double						activeFrameLatency;
		double						inactiveFrameLatency;
		DKSize						screenResolution;

		DKObject<DKAudioDevice> alContext;
		DKObject<DKOpenGLContext> glContext;
		
		void RenderScreen(bool invalidate);
	};
}
