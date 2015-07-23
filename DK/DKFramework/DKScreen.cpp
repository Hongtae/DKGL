//
//  File: DKScreen.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKScreen.h"
#include "DKFrame.h"
#include "DKRenderer.h"
#include "DKMath.h"
#include "DKLinearTransform2.h"
#include "DKAffineTransform2.h"

using namespace DKFoundation;
using namespace DKFramework;

DKScreen::DKScreen(void)
	: renderer(NULL)
	, window(NULL)
	, rootFrame(NULL)
	, suspended(false)
	, activated(false)
	, visible(false)
	, tickDelta(0)
	, tickCount(0)
	, activeFrameLatency(1.0f / 60.0f)
	, inactiveFrameLatency(1.0f / 30.0f)
	, screenResolution(DKSize(0, 0))
{
}

DKScreen::~DKScreen(void)
{
	Terminate(true);
}

void DKScreen::SetActiveFrameLatency(double f)
{
	activeFrameLatency = f;
}

void DKScreen::SetInactiveFrameLatency(double f)
{
	inactiveFrameLatency = f;
}

double DKScreen::ActiveFrameLatency(void) const
{
	return activeFrameLatency;
}

double DKScreen::InactiveFrameLatency(void) const
{
	return inactiveFrameLatency;
}

bool DKScreen::Run(DKWindow* window, DKFrame* frame)
{
	if (this->IsRunning())
		return false;

	if (window && window->IsValid() && frame)
	{
		this->window = window;
		this->rootFrame = frame;

		this->visible = this->window->IsVisible();
		this->activated = this->window->IsActive();

		if (DKRunLoop::Run())
		{
			// wait until first frame be drawn.
			this->ProcessOperation(DKFunction(this, &DKScreen::RenderScreen)->Invocation(true));
			return true;
		}
	}
	return false;
}

DKRunLoop* DKScreen::RunLoop(void) const
{
	return static_cast<DKRunLoop*>(const_cast<DKScreen*>(this));
}

DKWindow* DKScreen::Window(void)
{
	return window;
}

const DKWindow* DKScreen::Window(void) const
{
	return window;
}

DKFrame* DKScreen::RootFrame(void)
{
	return rootFrame;
}

const DKFrame* DKScreen::RootFrame(void) const
{
	return rootFrame;
}

bool DKScreen::SetKeyFrame(int deviceId, DKFrame* frame)
{
	DKFrame* prevHolder = NULL;
	auto p = this->keyboardHolders.Find(deviceId);
	if (p)
		prevHolder = p->value;

	if (prevHolder == frame)
		return true;

	if (frame)
	{
		if (frame->CanHandleKeyboard() && frame->IsDescendantOf(rootFrame))
		{
			this->keyboardHolders.Update(deviceId, frame);
			if (prevHolder)
				prevHolder->OnKeyboardLost(deviceId);

			return true;
		}
		return false;
	}
	if (p)
	{
		this->keyboardHolders.Remove(deviceId);

		if (prevHolder)
			prevHolder->OnKeyboardLost(deviceId);
	}
	return true;
}

bool DKScreen::SetFocusFrame(int deviceId, DKFrame* frame)
{
	DKFrame* prevHolder = NULL;
	auto p = this->mouseHolders.Find(deviceId);
	if (p)
		prevHolder = p->value;

	if (prevHolder == frame)
		return true;

	if (frame)
	{
		if (frame->CanHandleMouse() && frame->IsDescendantOf(rootFrame))
		{
			this->mouseHolders.Update(deviceId, frame);
			if (prevHolder)
				prevHolder->OnMouseLost(deviceId);
			return true;
		}
		return false;
	}
	if (p)
	{
		this->mouseHolders.Remove(deviceId);
		if (prevHolder)
			prevHolder->OnMouseLost(deviceId);
	}
	return true;
}

bool DKScreen::RemoveKeyFrame(int deviceId, DKFrame* frame)
{
	auto p = this->keyboardHolders.Find(deviceId);
	if (p && p->value == frame)
	{
		this->keyboardHolders.Remove(deviceId);
		return true;
	}
	return false;
}

bool DKScreen::RemoveFocusFrame(int deviceId, DKFrame* frame)
{
	auto p = this->mouseHolders.Find(deviceId);
	if (p && p->value == frame)
	{
		this->mouseHolders.Remove(deviceId);
		return true;
	}
	return false;
}

DKFrame* DKScreen::KeyFrame(int deviceId)
{
	const DKMap<int, DKFrame*>::Pair* p = keyboardHolders.Find(deviceId);
	if (p)
		return p->value;
	return NULL;
}

const DKFrame* DKScreen::KeyFrame(int deviceId) const
{
	const DKMap<int, DKFrame*>::Pair* p = keyboardHolders.Find(deviceId);
	if (p)
		return p->value;
	return NULL;
}

DKFrame* DKScreen::FocusFrame(int deviceId)
{
	const DKMap<int, DKFrame*>::Pair* p = mouseHolders.Find(deviceId);
	if (p)
		return p->value;
	return NULL;
}

const DKFrame* DKScreen::FocusFrame(int deviceId) const
{
	const DKMap<int, DKFrame*>::Pair* p = mouseHolders.Find(deviceId);
	if (p)
		return p->value;
	return NULL;
}

void DKScreen::RemoveKeyFrameForAnyDevices(DKFrame* frame, bool notify)
{
	DKArray<int> devIds;
	devIds.Reserve(this->keyboardHolders.Count());
	this->keyboardHolders.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
	{
		if (pair.value == frame)
			devIds.Add(pair.key);
	});
	for (int devId : devIds)
		this->keyboardHolders.Remove(devId);
	if (frame && notify)
	{
		for (int devId : devIds)
			frame->OnKeyboardLost(devId);
	}
}

void DKScreen::RemoveFocusFrameForAnyDevices(DKFrame* frame, bool notify)
{
	DKArray<int> devIds;
	devIds.Reserve(this->mouseHolders.Count());
	this->mouseHolders.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
	{
		if (pair.value == frame)
			devIds.Add(pair.key);
	});
	for (int devId : devIds)
		this->mouseHolders.Remove(devId);
	if (frame && notify)
	{
		for (int devId : devIds)
			frame->OnMouseLost(devId);
	}
}

void DKScreen::RemoveAllKeyFramesForAnyDevices(bool notify)
{
	if (notify)
	{
		DKArray<DKMap<int, DKFrame*>::Pair> pairs;
		pairs.Reserve(this->keyboardHolders.Count());
		this->keyboardHolders.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
		{
			pairs.Add(pair);
		});
		this->keyboardHolders.Clear();
		for (DKMap<int, DKFrame*>::Pair& pair : pairs)
		{
			if (pair.value)
				pair.value->OnKeyboardLost(pair.key);
		}
	}
	else
	{
		this->keyboardHolders.Clear();
	}
}

void DKScreen::RemoveAllFocusFramesForAnyDevices(bool notify)
{
	if (notify)
	{
		DKArray<DKMap<int, DKFrame*>::Pair> pairs;
		pairs.Reserve(this->keyboardHolders.Count());
		this->mouseHolders.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
		{
			pairs.Add(pair);
		});
		this->mouseHolders.Clear();
		for (DKMap<int, DKFrame*>::Pair& pair : pairs)
		{
			if (pair.value)
				pair.value->OnMouseLost(pair.key);
		}
	}
	else
	{
		this->mouseHolders.Clear();
	}
}

void DKScreen::Render(bool forced)
{
	PostOperation(DKFunction(this, &DKScreen::RenderScreen)->Invocation(forced));
}

void DKScreen::RenderScreen(bool invalidate)
{
	tickDelta = timer.Reset();
	tickDate = DKDateTime::Now();
	tickCount++;

	rootFrame->transform = DKMatrix3::identity;
	rootFrame->transformInverse = DKMatrix3::identity;
	rootFrame->Update(tickDelta, tickCount, tickDate);
	rootFrame->renderer = this->renderer;

	if (invalidate)
		rootFrame->SetRedraw();

	if (this->screenResolution.width > 0 && this->screenResolution.height > 0 && rootFrame->RenderInternal())
	{
		glContext->Present();

#ifdef DKGL_DEBUG_ENABLED
		GLenum err = glGetError();
		if (err != GL_NO_ERROR)
		{
			DKDateTime current = DKDateTime::Now();
			DKString time = DKString::Format(L"%04d-%02d-%02d %02d:%02d:%02d.%06d.(Thread:0x%x)",
				current.Year(), current.Month(), current.Day(), current.Hour(), current.Minute(), current.Second(), current.Microsecond(),
				DKThread::CurrentThreadId());
			switch (err)
			{
			case GL_INVALID_ENUM:
				DKLog("[%ls] glGetError returns GL_INVALID_ENUM\n", (const wchar_t*)time);
				break;
			case GL_INVALID_VALUE:
				DKLog("[%ls] glGetError returns GL_INVALID_VALUE\n", (const wchar_t*)time);
				break;
			case GL_INVALID_OPERATION:
				DKLog("[%ls] glGetError returns GL_INVALID_OPERATION\n", (const wchar_t*)time);
				break;
#ifdef GL_STACK_OVERFLOW
			case GL_STACK_OVERFLOW:
				DKLog("[%ls] glGetError returns GL_STACK_OVERFLOW\n", (const wchar_t*)time);
				break;
#endif
#ifdef GL_STACK_UNDERFLOW
			case GL_STACK_UNDERFLOW:
				DKLog("[%ls] glGetError returns GL_STACK_UNDERFLOW\n", (const wchar_t*)time);
				break;
#endif
			case GL_OUT_OF_MEMORY:
				DKLog("[%ls] glGetError returns GL_OUT_OF_MEMORY\n", (const wchar_t*)time);
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				DKLog("[%ls] glGetError returns GL_INVALID_FRAMEBUFFER_OPERATION\n", (const wchar_t*)time);
				break;
			default:
				DKLog("[%ls] glGetError returns error code: %x\n", (const wchar_t*)time, err);
				break;
			}
		}
#endif
	}
}

void DKScreen::Suspend(void)
{
	suspended = true;
}

bool DKScreen::IsSuspended(void) const
{
	return suspended;
}

void DKScreen::Resume(void)
{
	suspended = false;
}

const DKSize& DKScreen::ScreenResolution(void) const
{
	return screenResolution;
}

void DKScreen::OnInitialize(void)
{
	alContext = DKOpenALContext::SharedInstance();	
	glContext = DKOpenGLContext::SharedInstance();
	
	alContext->Bind();
	glContext->Bind(window);

	DKSize contentResolution = window->ContentSize();
	this->screenResolution = contentResolution;

	auto fboProxy = [this]() -> DKRenderTarget::TargetQuery
	{
		DKRenderTarget::TargetQuery tq;
		tq.width = floor(this->screenResolution.width + 0.5f);
		tq.height = floor(this->screenResolution.height + 0.5f);
		tq.depth = this->rootFrame->DepthFormat();
		tq.fbo = this->glContext->FramebufferId();
		return tq;
	};
	DKObject<DKRenderTarget> rootRenderTarget = DKRenderTarget::Create(DKFunction(fboProxy)->Invocation());
	DKASSERT_DEBUG( rootRenderTarget );
	this->renderer = DKObject<DKRenderer>::New(rootRenderTarget);
	
	rootFrame->transform = DKMatrix3::identity;
	rootFrame->transformInverse = DKMatrix3::identity;

	tickDelta = 0;
	tickCount = 0;
	tickDate = DKDateTime::Now();
	timer.Reset();

	rootFrame->Load(this, contentResolution);
	rootFrame->SetRedraw();

	window->AddObserver(this, DKFunction(this, &DKScreen::OnWindowEvent), DKFunction(this, &DKScreen::OnKeyboardEvent), DKFunction(this, &DKScreen::OnMouseEvent), this->RunLoop());

	double elapsed = timer.Elapsed();
	DKLog("DKScreen::OnInitialize %f seconds.\n", elapsed);
	timer.Reset();  // set first frame's delta nearest to 0
}

void DKScreen::OnTerminate(void)
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);

	window->RemoveObserver(this);

	// unload main frame.
	rootFrame->Unload();

	// unload all frames (which set to automatic unload)
	DKArray<DKFrame*> framesToUnload;
	framesToUnload.Reserve(autoUnloadFrames.Count());
	autoUnloadFrames.EnumerateForward([&framesToUnload](DKFrame* f) {framesToUnload.Add(f);});
	for (DKFrame* frame : framesToUnload)
	{
		frame->Unload();
	}
	autoUnloadFrames.Clear();
	

	renderer = NULL;

	alContext->Unbind();
	glContext->Unbind();
	
	alContext = NULL;
	glContext = NULL;

	this->window = NULL;
	this->rootFrame = NULL;
}

void DKScreen::OnIdle(void)
{
	if (visible && !suspended)
	{
		double elapsed = timer.Elapsed();
		double delay = 0;
		if (activated)
			delay = activeFrameLatency - elapsed;
		else
			delay = inactiveFrameLatency - elapsed;

		if (delay < 0)
		{
			RenderScreen(false);
		}
		else
		{
			DKRunLoop::WaitNextLoopTimeout(delay);
		}
	}
	else
	{
		DKRunLoop::OnIdle();
	}
}

void DKScreen::PerformOperation(const DKFoundation::DKOperation* operation)
{
	operation->Perform();
}

void DKScreen::OnWindowEvent(DKWindow::EventWindow type, DKSize contentSize, DKPoint windowOrigin)
{
	switch (type)
	{
	case DKWindow::EventWindowCreated:
		break;
	case DKWindow::EventWindowMoved:
		break;			
	case DKWindow::EventWindowClosed:    // window closed.
		Terminate(false);
		break;
	case DKWindow::EventWindowUpdate:    // refresh screen.
		RenderScreen(true);
		break;
	case DKWindow::EventWindowResized:   // window resize
		glContext->Update();
		screenResolution = contentSize;
		rootFrame->UpdateContentResolution();
		break;
	case DKWindow::EventWindowShown:
		visible = true;
		break;
	case DKWindow::EventWindowActivated:
		activated = true;
		visible = true;
		break;
	case DKWindow::EventWindowInactivated:
		activated = false;
		this->RemoveAllKeyFramesForAnyDevices(true);
		this->RemoveAllFocusFramesForAnyDevices(true);
		break;
	case DKWindow::EventWindowHidden:
	case DKWindow::EventWindowMinimized:
		visible = false;
		this->RemoveAllKeyFramesForAnyDevices(true);
		this->RemoveAllFocusFramesForAnyDevices(true);
		glFinish();
		break;
	}
	
#ifdef DKGL_DEBUG_ENABLED	
	switch (type)
	{
		case DKWindow::EventWindowCreated:
			DKLog("DKScreen::OnWindowEvent: WindowCreated (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowMoved:
		//	DKLog("DKScreen::OnWindowEvent: WindowMoved (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowClosed:
			DKLog("DKScreen::OnWindowEvent: WindowClosed (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowUpdate:
			DKLog("DKScreen::OnWindowEvent: WindowUpdate (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowResized:
			DKLog("DKScreen::OnWindowEvent: WindowResized (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowShown:
			DKLog("DKScreen::OnWindowEvent: WindowShown (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowActivated:
			DKLog("DKScreen::OnWindowEvent: WindowActivated (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowInactivated:
			DKLog("DKScreen::OnWindowEvent: WindowInactivated (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowHidden:
			DKLog("DKScreen::OnWindowEvent: WindowHidden (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		case DKWindow::EventWindowMinimized:
			DKLog("DKScreen::OnWindowEvent: WindowMinimized (%d x %d)\n", (int)screenResolution.width, (int)screenResolution.height);
			break;
		default:
			DKLog("DKScreen::OnWindowEvent: UNKNOWN EVENT!!\n");
			break;			
	}
#endif
}

void DKScreen::OnMouseEvent(DKWindow::EventMouse type, int deviceId, int buttonId, DKPoint pos, DKVector2 delta)
{
	pos = WindowToScreen(pos);
	delta = WindowToScreen(DKPoint(delta)).Vector();

	if (type == DKWindow::EventMouseMove)
	{
		RootFrame()->ProcessMouseInOut(deviceId, pos, true);
	}

	DKFrame* focusFrame = FocusFrame(deviceId);
	if (focusFrame)
	{
		// mouse events are available to visible, captured frame only.
		if (focusFrame->CanHandleMouse() && focusFrame->IsDescendantOf(rootFrame))
		{
		}
		else
		{
			// reset. (target is invalid)
			DKLog("DKScreen's Focus-Frame:%x for device:%d reset to NULL.\n", focusFrame, deviceId);
			SetFocusFrame(deviceId, NULL);
			focusFrame = NULL;
		}
	}
	if (focusFrame)
	{
		if (focusFrame != rootFrame)
		{
			DKFrame* superFrame = focusFrame->Superframe();
			DKASSERT_DEBUG(superFrame);

			// convert coordinates to root-frame space
			const DKSize& scale = rootFrame->ContentScale();
			DKMatrix3 tm = DKMatrix3::identity;
			tm.Multiply(DKAffineTransform2(DKLinearTransform2(scale.width, scale.height)).Matrix3());
			tm.Multiply(rootFrame->ContentTransformInverse());

			// convert coordinates to target's parent space
			tm.Multiply(superFrame->LocalFromRootTransform());
			// normalize to local space
			tm.Multiply(focusFrame->TransformInverse());

			// calculate delta
			DKVector2 posInFrame = pos.Vector().Transform(tm);
			DKVector2 oldPosInFrame = DKVector2(pos.Vector() - delta).Transform(tm);

			pos = posInFrame;
			delta = posInFrame - oldPosInFrame;
		}
		focusFrame->ProcessMouseEvent(type, deviceId, buttonId, pos, delta, false); // no propagation
	}
	else
	{
		if (DKRect(0,0,1,1).IsPointInside(pos))
			rootFrame->ProcessMouseEvent(type, deviceId, buttonId, pos, delta, true); // propagate event from root
	}
}

void DKScreen::OnKeyboardEvent(DKWindow::EventKeyboard type, int deviceId, DKVirtualKey key, DKString text)
{
	DKFrame* keyFrame = KeyFrame(deviceId);
	if (keyFrame)
	{
		// keyboard events are available for every frames.
		if (keyFrame->CanHandleKeyboard() && keyFrame->Screen() == this)
		{
			keyFrame->ProcessKeyboardEvent(type, deviceId, key, text);
		}
		else
		{
			// reset. (target is invalid)
			DKLog("DKScreen's Key-Frame:%x for device:%d reset to NULL.\n", keyFrame, deviceId);
			SetKeyFrame(deviceId, NULL);
		}
	}
	//DKLOG_DEBUG("%s\n", DKGL_FUNCTION_NAME);
}

DKPoint DKScreen::WindowToScreen(const DKPoint& pt) const
{
	return DKPoint(pt.x / screenResolution.width, pt.y / screenResolution.height);
}

DKPoint DKScreen::ScreenToWindow(const DKPoint& pt) const
{
	return DKPoint(pt.x * screenResolution.width, pt.y * screenResolution.height);
}

DKSize DKScreen::WindowToScreen(const DKSize& size) const
{
	return DKSize(size.width / screenResolution.width, size.height / screenResolution.height);
}

DKSize DKScreen::ScreenToWindow(const DKSize& size) const
{
	return DKSize(size.width * screenResolution.width, size.height * screenResolution.height);
}

DKRect DKScreen::WindowToScreen(const DKRect& rect) const
{
	return DKRect(WindowToScreen(rect.origin), WindowToScreen(rect.size));
}

DKRect DKScreen::ScreenToWindow(const DKRect& rect) const
{
	return DKRect(ScreenToWindow(rect.origin), ScreenToWindow(rect.size));
}

void DKScreen::RegisterAutoUnloadFrame(DKFrame* frame)
{
	if (frame && frame != rootFrame)
	{
		autoUnloadFrames.Insert(frame);
	}
}

void DKScreen::UnregisterAutoUnloadFrame(DKFrame* frame)
{
	if (frame && frame != rootFrame)
	{
		autoUnloadFrames.Remove(frame);
	}
}
