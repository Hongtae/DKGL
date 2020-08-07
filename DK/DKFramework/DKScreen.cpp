//
//  File: DKScreen.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKScreen.h"

using namespace DKFramework;
using namespace DKFramework::Private;

DKScreen::DKScreen(DKWindow*, DKFrame*)
    : window(nullptr)
    , rootFrame(nullptr)
    , screenResolution({0, 0})
{
	struct LocalEventLoop : public DKEventLoop
	{
		DKScreen* screen;
	};
	DKObject<LocalEventLoop> localEventLoop = DKOBJECT_NEW LocalEventLoop();
	localEventLoop->screen = this;

	eventLoop = localEventLoop.SafeCast<DKEventLoop>();
	graphicsDevice = DKGraphicsDevice::SharedInstance();
	audioDevice = DKAudioDevice::SharedInstance();
}

DKScreen::~DKScreen()
{
	if (thread && thread->IsAlive())
	{
		eventLoop->Stop();
		thread->WaitTerminate();
	}
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
    DKFrame* prevHolder = nullptr;
    auto p = this->keyboardHolders.Find(deviceId);
    if (p)
        prevHolder = p->value; // prevHolder can be null

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
    auto p = keyboardHolders.Find(deviceId);
    if (p)
        return p->value;
    return nullptr;
}

const DKFrame* DKScreen::KeyFrame(int deviceId) const
{
    auto p = keyboardHolders.Find(deviceId);
    if (p)
        return p->value;
    return nullptr;
}

DKFrame* DKScreen::FocusFrame(int deviceId)
{
    auto p = mouseHolders.Find(deviceId);
    if (p)
        return p->value;
    return nullptr;
}

const DKFrame* DKScreen::FocusFrame(int deviceId) const
{
    auto p = mouseHolders.Find(deviceId);
    if (p)
        return p->value;
    return nullptr;
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
