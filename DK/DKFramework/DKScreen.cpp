//
//  File: DKScreen.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKScreen.h"
#include "DKAffineTransform2.h"

using namespace DKFramework;

DKScreen::DKScreen(DKCommandQueue* cq, DKOperationQueue* oq)
    : window(nullptr)
    , rootFrame(nullptr)
    , screenResolution(DKSize(0, 0))
{
	struct LocalEventLoop : public DKEventLoop
	{
		DKScreen* screen;
	};
	DKObject<LocalEventLoop> localEventLoop = DKOBJECT_NEW LocalEventLoop();
	localEventLoop->screen = this;

	eventLoop = localEventLoop.SafeCast<DKEventLoop>();
	audioDevice = DKAudioDevice::SharedInstance();

    commandQueue = cq;
    if (commandQueue == nullptr)
    {
        DKObject<DKGraphicsDevice> graphicsDevice = DKGraphicsDevice::SharedInstance();
        commandQueue = graphicsDevice->CreateCommandQueue(0);
    }
    operationQueue = oq;
    if (operationQueue == nullptr)
    {
        operationQueue = DKOBJECT_NEW DKOperationQueue();
        operationQueue->SetMaxConcurrentOperations(1);
    }
}

DKScreen::~DKScreen()
{
    operationQueue->WaitForCompletion();
    eventLoop->Stop();
}

void DKScreen::Start()
{

}

void DKScreen::Pause()
{
}

void DKScreen::Resume()
{
}

void DKScreen::Stop()
{
}

void DKScreen::SetWindow(DKWindow* window)
{
    if (this->window != window)
    {
        this->window = window;
    }
}

void DKScreen::SetRootFrame(DKFrame* frame)
{
    if (rootFrame != frame)
    {
        bool loaded = false;
        if (rootFrame)
            loaded = rootFrame->IsLoaded();

        DKObject<DKFrame> oldFrame = rootFrame;
        rootFrame = frame;

        if (loaded)
        {
            frame->Load(this, this->Resolution());
            oldFrame->Unload();
        }
    }
}

DKObject<DKCanvas> DKScreen::CreateCanvas() const
{
	return nullptr;
}

DKSize DKScreen::Resolution() const
{
	return {};
}

bool DKScreen::SetKeyFrame(int deviceId, DKFrame* frame)
{
    DKFrame* prevCaptor = nullptr;
    auto p = this->keyFrames.Find(deviceId);
    if (p)
        prevCaptor = p->value; // prevCaptor can be null

    if (prevCaptor == frame)
        return true;

    if (frame)
    {
        if (frame->CanHandleKeyboard() && frame->IsDescendantOf(rootFrame))
        {
            this->keyFrames.Update(deviceId, frame);
            if (prevCaptor)
                prevCaptor->OnKeyboardLost(deviceId);
            return true;
        }
        return false;
    }
    if (p)
    {
        this->keyFrames.Remove(deviceId);
        if (prevCaptor)
            prevCaptor->OnKeyboardLost(deviceId);
    }
    return true;
}

bool DKScreen::SetFocusFrame(int deviceId, DKFrame* frame)
{
    DKFrame* prevCaptor = NULL;
    auto p = this->focusFrames.Find(deviceId);
    if (p)
        prevCaptor = p->value;

    if (prevCaptor == frame)
        return true;

    if (frame)
    {
        if (frame->CanHandleMouse() && frame->IsDescendantOf(rootFrame))
        {
            this->focusFrames.Update(deviceId, frame);
            if (prevCaptor)
                prevCaptor->OnMouseLost(deviceId);
            return true;
        }
        return false;
    }
    if (p)
    {
        this->focusFrames.Remove(deviceId);
        if (prevCaptor)
            prevCaptor->OnMouseLost(deviceId);
    }
    return true;
}

bool DKScreen::RemoveKeyFrame(int deviceId, DKFrame* frame)
{
    auto p = this->keyFrames.Find(deviceId);
    if (p && p->value == frame)
    {
        this->keyFrames.Remove(deviceId);
        return true;
    }
    return false;
}

bool DKScreen::RemoveFocusFrame(int deviceId, DKFrame* frame)
{
    if (auto p = this->focusFrames.Find(deviceId); p && p->value == frame)
    {
        this->focusFrames.Remove(deviceId);
        return true;
    }
    return false;
}

DKFrame* DKScreen::KeyFrame(int deviceId)
{
    if (auto p = keyFrames.Find(deviceId); p)
        return p->value;
    return nullptr;
}

const DKFrame* DKScreen::KeyFrame(int deviceId) const
{
    if (auto p = keyFrames.Find(deviceId); p)
        return p->value;
    return nullptr;
}

DKFrame* DKScreen::FocusFrame(int deviceId)
{
    if (auto p = focusFrames.Find(deviceId); p)
        return p->value;
    return nullptr;
}

const DKFrame* DKScreen::FocusFrame(int deviceId) const
{
    if (auto p = focusFrames.Find(deviceId); p)
        return p->value;
    return nullptr;
}

void DKScreen::RemoveKeyFrameForAnyDevices(DKFrame* frame, bool notify)
{
    DKArray<int> devIds;
    devIds.Reserve(this->keyFrames.Count());
    this->keyFrames.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
    {
        if (pair.value == frame)
            devIds.Add(pair.key);
    });
    for (int devId : devIds)
        this->keyFrames.Remove(devId);
    if (frame && notify)
    {
        for (int devId : devIds)
            frame->OnKeyboardLost(devId);
    }
}

void DKScreen::RemoveFocusFrameForAnyDevices(DKFrame* frame, bool notify)
{
    DKArray<int> devIds;
    devIds.Reserve(this->focusFrames.Count());
    this->focusFrames.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
    {
        if (pair.value == frame)
            devIds.Add(pair.key);
    });
    for (int devId : devIds)
        this->focusFrames.Remove(devId);
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
        pairs.Reserve(this->keyFrames.Count());
        this->keyFrames.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
        {
            pairs.Add(pair);
        });
        this->keyFrames.Clear();
        for (DKMap<int, DKFrame*>::Pair& pair : pairs)
        {
            if (pair.value)
                pair.value->OnKeyboardLost(pair.key);
        }
    }
    else
    {
        this->keyFrames.Clear();
    }
}

void DKScreen::RemoveAllFocusFramesForAnyDevices(bool notify)
{
    if (notify)
    {
        DKArray<DKMap<int, DKFrame*>::Pair> pairs;
        pairs.Reserve(this->focusFrames.Count());
        this->focusFrames.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
        {
            pairs.Add(pair);
        });
        this->focusFrames.Clear();
        for (DKMap<int, DKFrame*>::Pair& pair : pairs)
        {
            if (pair.value)
                pair.value->OnMouseLost(pair.key);
        }
    }
    else
    {
        this->focusFrames.Clear();
    }
}

const DKFrame* DKScreen::HoverFrame(int deviceId) const
{
    if (auto p = hoverFrames.Find(deviceId); p)
        return p->value;
    return nullptr;
}

void DKScreen::LeaveHoverFrame(DKFrame* frame)
{
    DKArray<int> devIds;
    devIds.Reserve(this->hoverFrames.Count());
    this->hoverFrames.EnumerateForward([&](DKMap<int, DKFrame*>::Pair& pair)
    {
        if (pair.value == frame)
            devIds.Add(pair.key);
    });
    for (int devId : devIds)
    {
        this->hoverFrames.Remove(devId);
        DKASSERT_DEBUG(frame->Screen() == this);
        frame->OnMouseLeave(devId);
    }
}

DKPoint DKScreen::WindowToScreen(const DKPoint& pt)  const
{
    DKASSERT_DEBUG(screenResolution.width > 0.0f && screenResolution.height > 0.0f);
    return DKPoint(pt.x / screenResolution.width, pt.y / screenResolution.height);
}

DKPoint DKScreen::ScreenToWindow(const DKPoint& pt)  const
{
    DKASSERT_DEBUG(screenResolution.width > 0.0f && screenResolution.height > 0.0f);
    return DKPoint(pt.x * screenResolution.width, pt.y * screenResolution.height);
}

DKSize DKScreen::WindowToScreen(const DKSize& size) const
{
    DKASSERT_DEBUG(screenResolution.width > 0.0f && screenResolution.height > 0.0f);
    return DKSize(size.width / screenResolution.width, size.height / screenResolution.height);
}

DKSize DKScreen::ScreenToWindow(const DKSize& size) const
{
    DKASSERT_DEBUG(screenResolution.width > 0.0f && screenResolution.height > 0.0f);
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

void DKScreen::Draw() const
{
}

void DKScreen::EventLoopIdle(DKScreen*, DKEventLoop*)
{
}

bool DKScreen::ProcessKeyboardEvent(const DKWindow::KeyboardEvent& event)
{
    DKFrame* keyFrame = KeyFrame(event.deviceId);
    if (keyFrame)
    {
        // keyboard events are available for every frames.
        if (keyFrame->CanHandleKeyboard() && keyFrame->Screen() == this)
        {
            return keyFrame->ProcessKeyboardEvent(event);
        }
        else
        {
            // reset. (target is invalid)
            DKLogW("DKScreen's Key-Frame:%x for device:%d reset to null.",
                   keyFrame, event.deviceId);
            SetKeyFrame(event.deviceId, nullptr);
        }
    }
    return false;
}

bool DKScreen::ProcessMouseEvent(const DKWindow::MouseEvent& event)
{
    if (rootFrame)
    {
        const DKSize res = Resolution();
        DKASSERT_DEBUG(res.width > 0.0f && res.height > 0.0f);

        // normalize vector.
        DKPoint pos = event.location.Vector() / res.Vector();
        DKVector2 delta = event.delta / res.Vector();

        if (event.type == DKWindow::MouseEvent::Move)
        {
            DKFrame* hover = rootFrame->FindHoverFrame(pos);
            DKFrame* leave = nullptr;

            if (auto p = this->hoverFrames.Find(event.deviceId); p)
                leave = p->value;

            if (hover != leave)
            {
                if (hover)
                    this->hoverFrames.Update(event.deviceId, hover);
                else
                    this->hoverFrames.Remove(event.deviceId);

                if (leave)
                {
                    DKASSERT_DEBUG(leave->Screen() == this);
                    leave->OnMouseLeave(event.deviceId);
                }
                if (hover)
                {
                    DKASSERT_DEBUG(hover->Screen() == this);
                    hover->OnMouseHover(event.deviceId);
                }
            }
        }

        DKFrame* focus = FocusFrame(event.deviceId);
        if (focus)
        {
            if (focus->CanHandleMouse() && focus->IsDescendantOf(rootFrame))
            {
            }
            else
            {
                // reset. (target is invalid)
                DKLogW("DKScreen's focus-frame:%x for device:%d reset to null.",
                       focus, event.deviceId);
                SetFocusFrame(event.deviceId, nullptr);
                focus = nullptr;
            }
        }
        if (focus)
        {
            if (focus != rootFrame)
            {
                const DKFrame* const super = focus->Superframe();
                DKASSERT_DEBUG(super);

                // convert coordinates to root-frame space
                const DKSize scale = rootFrame->ContentScale();
                DKMatrix3 tm = DKMatrix3::identity;
                tm.Multiply(DKAffineTransform2(DKLinearTransform2(scale.width, scale.height)).Matrix3());
                tm.Multiply(rootFrame->ContentTransformInverse());

                // convert coordinates to target's parent space
                tm.Multiply(super->LocalFromRootTransform());
                // normalize to local space
                tm.Multiply(focus->TransformInverse());

                // calculate delta
                const DKVector2 posInFrame = pos.Vector().Transform(tm);
                const DKVector2 oldPosInFrame = DKVector2(pos.Vector() - delta).Transform(tm);

                pos = posInFrame;
                delta = posInFrame - oldPosInFrame;
            }
            return focus->ProcessMouseEvent(event, pos, delta, false /* propagate */);
        }
        else
        {
            if (DKRect(0, 0, 1, 1).IsPointInside(pos))
                return rootFrame->ProcessMouseEvent(event, pos, delta, true);
        }
    }
    return false;
}

bool DKScreen::ProcessWindowEvent(const DKWindow::WindowEvent&)
{
    return false;
}
