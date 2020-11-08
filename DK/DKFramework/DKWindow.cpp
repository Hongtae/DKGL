//
//  File: DKWindow.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include "DKWindow.h"
#include "Interface/DKWindowInterface.h"

namespace DKFramework::Private
{
    static inline bool _GetUCArrayBit(const unsigned char* bytes, int index)
    {
        unsigned char v = 1 << (index % 8);
        unsigned char c = bytes[index / 8];
        return (c & v) != 0;
    }
    static inline void _SetUCArrayBit(unsigned char* bytes, int index, bool value)
    {
        unsigned char v = 1 << (index % 8);
        unsigned char& c = bytes[index / 8];
        if (value)
        c |= v;
        else
        c &= ~v;
    }
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKWindow::DKWindow()
	: activated(false)
	, visible(false)
	, dispatchQueue(nullptr)
	, impl(nullptr)
{
}

DKWindow::~DKWindow()
{
	if (impl)
	{
		impl->Destroy();
		delete impl;
	}
}

DKObject<DKWindow> DKWindow::Create(const DKString& name,
									uint32_t style,
									DKDispatchQueue* dispatchQueue,
									const WindowCallback& cb)
{
	DKObject<DKWindow> window = DKObject<DKWindow>::New();
	window->impl = DKWindowInterface::CreateInterface(window);
	window->dispatchQueue = dispatchQueue;
	window->callback = cb;
	if (window->impl->Create(name, style))
	{
		return window;
	}
	return NULL;
}

DKObject<DKWindow> DKWindow::CreateProxy(void* systemHandle)
{
	DKObject<DKWindow> window = DKObject<DKWindow>::New();
	window->impl = DKWindowInterface::CreateInterface(window);
	if (window->impl->CreateProxy(systemHandle))
	{
		return window;
	}
	return NULL;
}

void DKWindow::UpdateProxy()
{
	if (impl)
		impl->UpdateProxy();
}

bool DKWindow::IsProxy() const
{
	if (impl)
		return impl->IsProxy();
	return false;
}

void DKWindow::SetCallback(const WindowCallback& cb)
{
	this->callback = cb;
}

const DKWindow::WindowCallback& DKWindow::Callback() const
{
	return this->callback;
}

void DKWindow::Close()
{
	if (impl)
	{
		impl->Destroy();
		delete impl;
	}
	impl = NULL;
	this->callback = Callback();
}

bool DKWindow::IsTextInputEnabled(int deviceId) const
{
	if (impl)
		return impl->IsTextInputEnabled(deviceId);
	return false;
}

void DKWindow::SetTextInputEnabled(int deviceId, bool enabled)
{
	if (impl)
		impl->EnableTextInput(deviceId, enabled);
}

bool DKWindow::KeyState(int deviceId, DKVirtualKey k) const
{
	DKASSERT_DEBUG(static_cast<int>(k) >= 0 && static_cast<int>(k) <= static_cast<int>(DKVirtualKey::MaxValue));
	DKCriticalSection<DKSpinLock> guard(this->stateLock);
	if (auto p = keyboardStateMap.Find(deviceId); p)
	{
		return _GetUCArrayBit(p->value.keyStateBits, static_cast<int>(k));
	}
	return false;
}

void DKWindow::SetKeyState(int deviceId, DKVirtualKey k, bool down)
{
	DKASSERT_DEBUG(static_cast<int>(k) >= 0 && static_cast<int>(k) <= static_cast<int>(DKVirtualKey::MaxValue));
	DKCriticalSection<DKSpinLock> guard(this->stateLock);
	KeyboardState& state = KeyboardStateForDevice(deviceId);

	_SetUCArrayBit(state.keyStateBits, static_cast<int>(k), down);
}

void DKWindow::ResetKeyState(int deviceId)
{
	DKCriticalSection<DKSpinLock> guard(this->stateLock);
	if (auto p = keyboardStateMap.Find(deviceId); p)
	{
		KeyboardState& state = p->value;
		memset(state.keyStateBits, 0, sizeof(state.keyStateBits));
	}
}

void DKWindow::ResetKeyStateForAllDevices()
{
	DKCriticalSection<DKSpinLock> guard(this->stateLock);
	this->keyboardStateMap.Clear();
}

DKWindow::KeyboardState& DKWindow::KeyboardStateForDevice(int deviceId) const
{
	if (auto p = keyboardStateMap.Find(deviceId); p)
		return p->value;

	KeyboardState& newState = keyboardStateMap.Value(deviceId);
	memset(&newState, 0, sizeof(KeyboardState));
	return newState;
}

void DKWindow::PostMouseEvent(const MouseEvent& event)
{
	if (this->dispatchQueue)
	{
		MouseEvent* eventCopy = new MouseEvent(event);	

		DKCriticalSection<DKSpinLock> guard(handlerLock);
		pendingEvents.Reserve(pendingEvents.Count() + mouseEventHandlers.Count());
		mouseEventHandlers.EnumerateForward([&](decltype(mouseEventHandlers)::Pair& pair) {
			if (pair.value)
			{
				DKObject<DKOperation> op = pair.value->Invocation(*eventCopy).SafeCast<DKOperation>();
				PendingEvent pe = { pair.key, nullptr };
				pe.state = this->dispatchQueue->Submit(op);	// enqueue the event.
				if (pe.state)
					pendingEvents.Add(pe);
			}
		});

		// delete eventCopy async.
		DKObject<DKWindow> self = this;
		this->dispatchQueue->Submit(DKFunction([eventCopy, self]() mutable
		{
			delete eventCopy;
			self->ClearCompletedEvents();
		})->Invocation());
	}
	else
	{
		handlerLock.Lock();
		DKArray<DKObject<MouseEventHandler>> callbacks;
		callbacks.Reserve(mouseEventHandlers.Count());
		mouseEventHandlers.EnumerateForward([&callbacks](decltype(mouseEventHandlers)::Pair& pair) {
			if (pair.value)
				callbacks.Add(pair.value);
		});
		handlerLock.Unlock();

		for (MouseEventHandler* handler : callbacks)
			handler->Invoke(event);
	}
}

void DKWindow::PostKeyboardEvent(const KeyboardEvent& event)
{
	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(this->stateLock);
		KeyboardState& keyboard = KeyboardStateForDevice(event.deviceId);

		if (event.type == KeyboardEvent::KeyDown)
		{
			if (_GetUCArrayBit(keyboard.keyStateBits, static_cast<int>(event.key)))
				return;
			else
				_SetUCArrayBit(keyboard.keyStateBits, static_cast<int>(event.key), true);
		}
		if (event.type == KeyboardEvent::KeyUp)
		{
			if (_GetUCArrayBit(keyboard.keyStateBits, static_cast<int>(event.key)))
				_SetUCArrayBit(keyboard.keyStateBits, static_cast<int>(event.key), false);
			else
				return;
		}
	}

	if (this->dispatchQueue)
	{
		KeyboardEvent* eventCopy = new KeyboardEvent(event);

		DKCriticalSection<DKSpinLock> guard(handlerLock);
		pendingEvents.Reserve(pendingEvents.Count() + keyboardEventHandlers.Count());
		keyboardEventHandlers.EnumerateForward([&](decltype(keyboardEventHandlers)::Pair& pair) {
			if (pair.value)
			{
				DKObject<DKOperation> op = pair.value->Invocation(*eventCopy).SafeCast<DKOperation>();
				PendingEvent pe = { pair.key, nullptr };
				pe.state = this->dispatchQueue->Submit(op);	// enqueue the event.
				if (pe.state)
					pendingEvents.Add(pe);
			}
		});

		// delete eventCopy async.
		DKObject<DKWindow> self = this;
		this->dispatchQueue->Submit(DKFunction([eventCopy, self]() mutable
		{
			delete eventCopy;
			self->ClearCompletedEvents();
		})->Invocation());
	}
	else
	{
		handlerLock.Lock();
		DKArray<DKObject<KeyboardEventHandler>> callbacks;
		callbacks.Reserve(keyboardEventHandlers.Count());
		keyboardEventHandlers.EnumerateForward([&callbacks](decltype(keyboardEventHandlers)::Pair& pair) {
			if (pair.value)
				callbacks.Add(pair.value);
		});
		handlerLock.Unlock();

		for (KeyboardEventHandler* handler : callbacks)
			handler->Invoke(event);
	}
}

void DKWindow::PostWindowEvent(const WindowEvent& event)
{
	if (true)
	{
		DKCriticalSection<DKSpinLock> guard(this->stateLock);
		switch (event.type)
		{
		case WindowEvent::WindowCreated:
			this->windowRect = event.windowRect;
			this->contentRect = event.contentRect;
			this->activated = false;
			this->visible = false;
			DKLogD("EventWindowCreated (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowClosed:
			this->activated = false;
			this->visible = false;
			this->keyboardStateMap.Clear();
			DKLogD("EventWindowDestroy (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowHidden:
			this->activated = false;
			this->visible = false;
			this->keyboardStateMap.Clear();
			DKLogD("EventWindowHidden (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowShown:
			this->visible = true;
			DKLogD("EventWindowShown (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowActivated:
			this->activated = true;
			this->visible = true;
			DKLogD("EventWindowActivated (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowInactivated:
			this->activated = false;
			this->keyboardStateMap.Clear();
			DKLogD("EventWindowInactivated (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowMinimized:
			this->visible = false;
			this->keyboardStateMap.Clear();
			DKLogD("EventWindowMinimized (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowMoved:
			//	DKLogD("EventWindowMoved (%.0f x %.0f), scale:%.1f", contentSize.width, contentSize.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowResized:
			this->windowRect = event.windowRect;
			this->contentRect = event.contentRect;
			DKLogD("EventWindowResized (%.0f x %.0f), scale:%.1f", contentRect.size.width, contentRect.size.height, event.contentScaleFactor);
			break;
		case WindowEvent::WindowUpdate:
			//	DKLogD("EventWindowUpdate (%.0f x %.0f), scale:%.1f", contentSize.width, contentSize.height, event.contentScaleFactor);
			break;
		}
	}

	if (this->dispatchQueue)
	{
        WindowEvent* eventCopy = new WindowEvent(event);

        DKCriticalSection<DKSpinLock> guard(handlerLock);
        pendingEvents.Reserve(pendingEvents.Count() + windowEventHandlers.Count());
        windowEventHandlers.EnumerateForward([&](decltype(windowEventHandlers)::Pair& pair)
        {
            if (pair.value)
            {
                DKObject<DKOperation> op = pair.value->Invocation(*eventCopy).SafeCast<DKOperation>();
                PendingEvent pe = { pair.key, nullptr };
                pe.state = this->dispatchQueue->Submit(op);	// enqueue the event.
                if (pe.state)
                    pendingEvents.Add(pe);
            }
        });

        // delete eventCopy async
        DKObject<DKWindow> self = this;
        this->dispatchQueue->Submit(DKFunction([eventCopy, self]() mutable
        {
            delete eventCopy;
            self->ClearCompletedEvents();
        })->Invocation());
	}
	else
	{
		handlerLock.Lock();
		DKArray<DKObject<WindowEventHandler>> callbacks;
		callbacks.Reserve(windowEventHandlers.Count());
		windowEventHandlers.EnumerateForward([&callbacks](decltype(windowEventHandlers)::Pair& pair)
        {
			if (pair.value)
				callbacks.Add(pair.value);
		});
		handlerLock.Unlock();

		for (WindowEventHandler* handler : callbacks)
			handler->Invoke(event);
	}
}

void DKWindow::ClearCompletedEvents()
{
	DKCriticalSection<DKSpinLock> guard(handlerLock);
	DKArray<PendingEvent> activeEvents;
	activeEvents.Reserve(pendingEvents.Count());

	for (PendingEvent& pe : pendingEvents)
	{
		if (pe.state->IsPending())
			activeEvents.Add(pe);
	}

	pendingEvents.Clear();
	pendingEvents = std::move(activeEvents);
}

void DKWindow::ShowMouse(int deviceId, bool bShow)
{
	if (impl)
		impl->ShowMouse(deviceId, bShow);
}

void DKWindow::HoldMouse(int deviceId, bool bHold)
{
	if (impl)
		impl->HoldMouse(deviceId, bHold);
}

bool DKWindow::IsMouseVisible(int deviceId) const
{
	if (impl)
		return impl->IsMouseVisible(deviceId);
	return false;
}

bool DKWindow::IsMouseHeld(int deviceId) const
{
	if (impl)
		return impl->IsMouseHeld(deviceId);
	return false;
}

DKPoint DKWindow::MousePosition(int deviceId) const
{
	if (impl)
		return impl->MousePosition(deviceId);
	return DKPoint(0,0);
}

void DKWindow::SetMousePosition(int deviceId, const DKPoint& pt)
{
	if (impl)
		impl->SetMousePosition(deviceId, pt);
}

void DKWindow::Show()
{
	if (impl)
		impl->Show();
}

void DKWindow::Hide()
{
	if (impl)
		impl->Hide();
}

void DKWindow::Activate()
{
	if (impl)
		impl->Activate();
}

void DKWindow::Minimize()
{
	if (impl)
		impl->Minimize();
}

void DKWindow::Resize(const DKSize& size)
{
	if (impl)
		impl->Resize(size, NULL);
}

void DKWindow::Resize(const DKSize& size, const DKPoint& pt)
{
	if (impl)
		impl->Resize(size, &pt);
}

void DKWindow::SetOrigin(const DKPoint& pt)
{
	if (impl)
		impl->SetOrigin(pt);
}

DKRect DKWindow::WindowRect() const
{
	return windowRect;
}

DKRect DKWindow::ContentRect() const
{
	return contentRect;
}

float DKWindow::ContentScaleFactor() const
{
	if (impl)
		return impl->ContentScaleFactor();
	return 1.0;
}

void DKWindow::SetTitle(const DKString& title)
{
	if (impl)
		impl->SetTitle(title);
}

DKString DKWindow::Title() const
{
	if (impl)
		return impl->Title();
	return L"";
}

void DKWindow::AddEventHandler(EventHandlerContext context,
							   WindowEventHandler* windowProc,
							   KeyboardEventHandler* keyboardProc,
							   MouseEventHandler* mouseProc)
{
	if (context)
	{
		DKCriticalSection<DKSpinLock> guard(handlerLock);
		windowEventHandlers.Update(context, windowProc);
		keyboardEventHandlers.Update(context, keyboardProc);
		mouseEventHandlers.Update(context, mouseProc);
	}
}
	
void DKWindow::RemoveEventHandler(EventHandlerContext context)
{
	if (context)
	{
		DKCriticalSection<DKSpinLock> guard(handlerLock);
		windowEventHandlers.Remove(context);
		keyboardEventHandlers.Remove(context);
		mouseEventHandlers.Remove(context);

		if (this->dispatchQueue)
		{
            for (PendingEvent& pe : pendingEvents)
            {
                if (pe.context == context)
                    pe.state->Revoke();
            }
		}
	}
}

bool DKWindow::IsValid() const
{
	if (impl)
		return impl->IsValid();
	return false;
}

void* DKWindow::PlatformHandle() const
{
	if (impl)
		return impl->PlatformHandle();
	return NULL;
}

DKString DKWindow::GetVKName(DKVirtualKey lKey)
{
    switch (lKey)
    {
    case DKVirtualKey::None:				return L"None";

    case DKVirtualKey::Escape:				return L"Escape";

    case DKVirtualKey::F1:					return L"F1";
    case DKVirtualKey::F2:					return L"F2";
    case DKVirtualKey::F3:					return L"F3";
    case DKVirtualKey::F4:					return L"F4";
    case DKVirtualKey::F5:					return L"F5";
    case DKVirtualKey::F6:					return L"F6";
    case DKVirtualKey::F7:					return L"F7";
    case DKVirtualKey::F8:					return L"F8";
    case DKVirtualKey::F9:					return L"F9";
    case DKVirtualKey::F10:					return L"F10";
    case DKVirtualKey::F11:					return L"F11";
    case DKVirtualKey::F12:					return L"F12";
    case DKVirtualKey::F13:					return L"F13";
    case DKVirtualKey::F14:					return L"F14";
    case DKVirtualKey::F15:					return L"F15";
    case DKVirtualKey::F16:					return L"F16";
    case DKVirtualKey::F17:					return L"F17";
    case DKVirtualKey::F18:					return L"F18";
    case DKVirtualKey::F19:					return L"F19";
    case DKVirtualKey::F20:					return L"F20";

    case DKVirtualKey::Num0:				return L"Num0";
    case DKVirtualKey::Num1:				return L"Num1";
    case DKVirtualKey::Num2:				return L"Num2";
    case DKVirtualKey::Num3:				return L"Num3";
    case DKVirtualKey::Num4:				return L"Num4";
    case DKVirtualKey::Num5:				return L"Num5";
    case DKVirtualKey::Num6:				return L"Num6";
    case DKVirtualKey::Num7:				return L"Num7";
    case DKVirtualKey::Num8:				return L"Num8";
    case DKVirtualKey::Num9:				return L"Num9";

    case DKVirtualKey::A:					return L"A";
    case DKVirtualKey::B:					return L"B";
    case DKVirtualKey::C:					return L"C";
    case DKVirtualKey::D:					return L"D";
    case DKVirtualKey::E:					return L"E";
    case DKVirtualKey::F:					return L"F";
    case DKVirtualKey::G:					return L"G";
    case DKVirtualKey::H:					return L"H";
    case DKVirtualKey::I:					return L"I";
    case DKVirtualKey::J:					return L"J";
    case DKVirtualKey::K:					return L"K";
    case DKVirtualKey::L:					return L"L";
    case DKVirtualKey::M:					return L"M";
    case DKVirtualKey::N:					return L"N";
    case DKVirtualKey::O:					return L"O";
    case DKVirtualKey::P:					return L"P";
    case DKVirtualKey::Q:					return L"Q";
    case DKVirtualKey::R:					return L"R";
    case DKVirtualKey::S:					return L"S";
    case DKVirtualKey::T:					return L"T";
    case DKVirtualKey::U:					return L"U";
    case DKVirtualKey::V:					return L"V";
    case DKVirtualKey::W:					return L"W";
    case DKVirtualKey::X:					return L"X";
    case DKVirtualKey::Y:					return L"Y";
    case DKVirtualKey::Z:					return L"Z";

    case DKVirtualKey::Period:				return L"Period";
    case DKVirtualKey::Comma:				return L"Comma";
    case DKVirtualKey::Slash:				return L"Slash";
    case DKVirtualKey::Tab:					return L"Tab";
    case DKVirtualKey::AccentTilde:			return L"AccentTilde";
    case DKVirtualKey::Backspace:			return L"Backspace";
    case DKVirtualKey::Semicolon:			return L"Semicolon";
    case DKVirtualKey::Quote:				return L"Quote";
    case DKVirtualKey::Backslash:			return L"Backslash";
    case DKVirtualKey::Equal:				return L"Equal";
    case DKVirtualKey::Hyphen:				return L"Hyphen";
    case DKVirtualKey::Space:				return L"Space";
    case DKVirtualKey::OpenBracket:			return L"OpenBracket";
    case DKVirtualKey::CloseBracket:		return L"CloseBracket";
    case DKVirtualKey::Capslock:			return L"Capslock";

    case DKVirtualKey::Return:				return L"Return";

    case DKVirtualKey::Fn:					return L"Fn";
    case DKVirtualKey::Insert:				return L"Insert";
    case DKVirtualKey::Home:				return L"Home";
    case DKVirtualKey::PageUp:				return L"PageUp";
    case DKVirtualKey::PageDown:			return L"PageDown";
    case DKVirtualKey::End:					return L"End";
    case DKVirtualKey::Delete:				return L"Delete";

    case DKVirtualKey::Left:				return L"Left";
    case DKVirtualKey::Right:				return L"Right";
    case DKVirtualKey::Up:					return L"Up";
    case DKVirtualKey::Down:				return L"Down";

    case DKVirtualKey::LeftShift:			return L"LeftShift";
    case DKVirtualKey::RightShift:			return L"RightShift";
    case DKVirtualKey::LeftOption:			return L"LeftOption";
    case DKVirtualKey::RightOption:			return L"RightOption";
    case DKVirtualKey::LeftControl:			return L"LeftControl";
    case DKVirtualKey::RightControl:		return L"RightControl";
    case DKVirtualKey::LeftCommand:			return L"LeftCommand";
    case DKVirtualKey::RightCommand:		return L"RightCommand";

    case DKVirtualKey::Pad0:				return L"Pad0";
    case DKVirtualKey::Pad1:				return L"Pad1";
    case DKVirtualKey::Pad2:				return L"Pad2";
    case DKVirtualKey::Pad3:				return L"Pad3";
    case DKVirtualKey::Pad4:				return L"Pad4";
    case DKVirtualKey::Pad5:				return L"Pad5";
    case DKVirtualKey::Pad6:				return L"Pad6";
    case DKVirtualKey::Pad7:				return L"Pad7";
    case DKVirtualKey::Pad8:				return L"Pad8";
    case DKVirtualKey::Pad9:				return L"Pad9";
    case DKVirtualKey::Enter:				return L"Enter";
    case DKVirtualKey::Numlock:				return L"Numlock";
    case DKVirtualKey::PadSlash:			return L"PadSlash";
    case DKVirtualKey::PadAsterisk:			return L"PadAsterisk";
    case DKVirtualKey::PadPlus:				return L"PadPlus";
    case DKVirtualKey::PadMinus:			return L"PadMinus";
    case DKVirtualKey::PadEqual:			return L"PadEqual";
    case DKVirtualKey::PadPeriod:			return L"PadPeriod";

    case DKVirtualKey::MaxValue:			return L"MaxValue";
    default:								return L"None";
    }

    return L"Error";
}
