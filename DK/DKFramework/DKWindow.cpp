//
//  File: DKWindow.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include "DKWindow.h"
#include "Interface/DKWindowInterface.h"

namespace DKFramework
{
	namespace Private
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
}

using namespace DKFramework;
using namespace DKFramework::Private;

DKWindow::DKWindow(void)
	: activated(false)
	, visible(false)
	, eventLoop(NULL)
	, impl(NULL)
{
}

DKWindow::~DKWindow(void)
{
	if (impl)
	{
		impl->Destroy();
		delete impl;
	}
}

DKObject<DKWindow> DKWindow::Create(const DKString& name,
									int style,
									DKEventLoop* eventLoop,
									const WindowCallback& cb)
{
	DKObject<DKWindow> window = DKObject<DKWindow>::New();
	window->impl = DKWindowInterface::CreateInterface(window);
	window->eventLoop = eventLoop;
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

void DKWindow::UpdateProxy(void)
{
	if (impl)
		impl->UpdateProxy();
}

bool DKWindow::IsProxy(void) const
{
	if (impl)
		return impl->IsProxy();
	return false;
}

void DKWindow::SetCallback(const WindowCallback& cb)
{
	this->callback = cb;
}

const DKWindow::WindowCallback& DKWindow::Callback(void) const
{
	return this->callback;
}

void DKWindow::Close(void)
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

bool DKWindow::KeyState(int deviceId, const DKVirtualKey& k) const
{
	DKASSERT_DEBUG(k >= 0 && k <= DKVK_MAXVALUE);
	DKCriticalSection<DKSpinLock> guard(this->keyboardLock);
	auto p = keyboardStateMap.Find(deviceId);
	if (p)
	{
		return _GetUCArrayBit(p->value.keyStateBits, k);
	}
	return false;
}

void DKWindow::SetKeyState(int deviceId, const DKVirtualKey& k, bool down)
{
	DKASSERT_DEBUG(k >= 0 && k <= DKVK_MAXVALUE);
	DKCriticalSection<DKSpinLock> guard(this->keyboardLock);
	KeyboardState& state = GetKeyboardState(deviceId);

	_SetUCArrayBit(state.keyStateBits, k, down);
}

void DKWindow::ResetKeyState(int deviceId)
{
	DKCriticalSection<DKSpinLock> guard(this->keyboardLock);
	auto p = keyboardStateMap.Find(deviceId);
	if (p)
	{
		KeyboardState& state = p->value;
		memset(state.keyStateBits, 0, sizeof(state.keyStateBits));
	}
}

void DKWindow::ResetKeyStateForAllDevices(void)
{
	DKCriticalSection<DKSpinLock> guard(this->keyboardLock);
	this->keyboardStateMap.Clear();
}

DKWindow::KeyboardState& DKWindow::GetKeyboardState(int deviceId) const
{
	auto p = keyboardStateMap.Find(deviceId);
	if (p)
		return p->value;

	KeyboardState& newState = keyboardStateMap.Value(deviceId);
	memset(&newState, 0, sizeof(KeyboardState));
	return newState;
}

void DKWindow::PostMouseEvent(const MouseEvent& event)
{
	if (this->eventLoop)
	{
		MouseEvent* eventCopy = ::new(DKMemoryDefaultAllocator::Alloc(sizeof(MouseEvent))) MouseEvent(event);	

		DKArray<DKObject<DKOperation>> operations;
		handlerLock.Lock();
		operations.Reserve(mouseEventHandlers.Count());
		mouseEventHandlers.EnumerateForward([&](decltype(mouseEventHandlers)::Pair& pair) {
			if (pair.value)
				operations.Add(pair.value->Invocation(*eventCopy).SafeCast<DKOperation>());
		});
		handlerLock.Unlock();

		for (DKOperation* op : operations)
			this->eventLoop->Post(op);
		// delete eventCopy
		this->eventLoop->Post(DKFunction([eventCopy]()
		{
			eventCopy->~MouseEvent();
			DKMemoryDefaultAllocator::Free(eventCopy);
		})->Invocation());
	}
	else
	{
		DKArray<DKObject<MouseEventHandler>> callbacks;
		handlerLock.Lock();
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
		DKCriticalSection<DKSpinLock> guard(this->keyboardLock);
		KeyboardState& keyboard = GetKeyboardState(event.deviceId);

		if (event.type == KeyboardEvent::KeyDown)
		{
			if (_GetUCArrayBit(keyboard.keyStateBits, event.key))
				return;
			else
				_SetUCArrayBit(keyboard.keyStateBits, event.key, true);
		}
		if (event.type == KeyboardEvent::KeyUp)
		{
			if (_GetUCArrayBit(keyboard.keyStateBits, event.key))
				_SetUCArrayBit(keyboard.keyStateBits, event.key, false);
			else
				return;
		}
	}

	if (this->eventLoop)
	{
		KeyboardEvent* eventCopy = ::new(DKMemoryDefaultAllocator::Alloc(sizeof(KeyboardEvent))) KeyboardEvent(event);

		DKArray<DKObject<DKOperation>> operations;
		handlerLock.Lock();
		operations.Reserve(keyboardEventHandlers.Count());
		keyboardEventHandlers.EnumerateForward([&](decltype(keyboardEventHandlers)::Pair& pair) {
			if (pair.value)
				operations.Add(pair.value->Invocation(*eventCopy).SafeCast<DKOperation>());
		});
		handlerLock.Unlock();

		for (DKOperation* op : operations)
			this->eventLoop->Post(op);
		// delete eventCopy
		this->eventLoop->Post(DKFunction([eventCopy]()
		{
			eventCopy->~KeyboardEvent();
			DKMemoryDefaultAllocator::Free(eventCopy);
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
	switch (event.type)
	{
	case WindowEvent::WindowCreated:
		this->windowRect = event.windowRect;
		this->contentRect = event.contentRect;
		this->activated = false;
		this->visible = false;
		DKLog("EventWindowCreated (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowClosed:
		this->activated = false;
		this->visible = false;
		this->ResetKeyStateForAllDevices();
		DKLog("EventWindowDestroy (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowHidden:
		this->activated = false;
		this->visible = false;
		this->ResetKeyStateForAllDevices();
		DKLog("EventWindowHidden (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowShown:
		this->visible = true;
		DKLog("EventWindowShown (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowActivated:
		this->activated = true;
		this->visible = true;
		DKLog("EventWindowActivated (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowInactivated:
		this->activated = false;
		this->ResetKeyStateForAllDevices();
		DKLog("EventWindowInactivated (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowMinimized:
		this->visible = false;
		this->ResetKeyStateForAllDevices();
		DKLog("EventWindowMinimized (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowMoved:
	//	DKLog("EventWindowMoved (%.0f x %.0f)\n", contentSize.width, contentSize.height);
		break;
	case WindowEvent::WindowResized:
		this->windowRect = event.windowRect;
		this->contentRect = event.contentRect;
		DKLog("EventWindowResized (%.0f x %.0f)\n", contentRect.size.width, contentRect.size.height);
		break;
	case WindowEvent::WindowUpdate:
	//	DKLog("EventWindowUpdate (%.0f x %.0f)\n", contentSize.width, contentSize.height);
		break;
	}

	if (this->eventLoop)
	{
		WindowEvent* eventCopy = ::new(DKMemoryDefaultAllocator::Alloc(sizeof(WindowEvent))) WindowEvent(event);

		DKArray<DKObject<DKOperation>> operations;
		handlerLock.Lock();
		operations.Reserve(windowEventHandlers.Count());
		windowEventHandlers.EnumerateForward([&](decltype(windowEventHandlers)::Pair& pair) {
			if (pair.value)
				operations.Add(pair.value->Invocation(*eventCopy).SafeCast<DKOperation>());
		});
		handlerLock.Unlock();

		for (DKOperation* op : operations)
			this->eventLoop->Post(op);
		// delete eventCopy
		this->eventLoop->Post(DKFunction([eventCopy]()
		{
			eventCopy->~WindowEvent();
			DKMemoryDefaultAllocator::Free(eventCopy);
		})->Invocation());
	}
	else
	{
		handlerLock.Lock();
		DKArray<DKObject<WindowEventHandler>> callbacks;
		callbacks.Reserve(windowEventHandlers.Count());
		windowEventHandlers.EnumerateForward([&callbacks](decltype(windowEventHandlers)::Pair& pair) {
			if (pair.value)
				callbacks.Add(pair.value);
		});
		handlerLock.Unlock();

		for (WindowEventHandler* handler : callbacks)
			handler->Invoke(event);
	}
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

void DKWindow::Show(void)
{
	if (impl)
		impl->Show();
}

void DKWindow::Hide(void)
{
	if (impl)
		impl->Hide();
}

void DKWindow::Activate(void)
{
	if (impl)
		impl->Activate();
}

void DKWindow::Minimize(void)
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

DKRect DKWindow::WindowRect(void) const
{
	return windowRect;
}

DKRect DKWindow::ContentRect(void) const
{
	return contentRect;
}

double DKWindow::ContentScaleFactor(void) const
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

DKString DKWindow::Title(void) const
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
	}
}

bool DKWindow::IsValid(void) const
{
	if (impl)
		return impl->IsValid();
	return false;
}

void* DKWindow::PlatformHandle(void) const
{
	if (impl)
		return impl->PlatformHandle();
	return NULL;
}

DKString DKWindow::GetVKName(DKVirtualKey lKey)
{
	switch (lKey)
	{
	case DKVK_NONE:					return L"DKVK_NONE";

	case DKVK_ESCAPE:				return L"DKVK_ESCAPE";

	case DKVK_F1:					return L"DKVK_F1"; 
	case DKVK_F2:					return L"DKVK_F2"; 
	case DKVK_F3:					return L"DKVK_F3"; 
	case DKVK_F4:					return L"DKVK_F4"; 
	case DKVK_F5:					return L"DKVK_F5"; 
	case DKVK_F6:					return L"DKVK_F6"; 
	case DKVK_F7:					return L"DKVK_F7"; 
	case DKVK_F8:					return L"DKVK_F8"; 
	case DKVK_F9:					return L"DKVK_F9"; 
	case DKVK_F10:					return L"DKVK_F10"; 
	case DKVK_F11:					return L"DKVK_F11"; 
	case DKVK_F12:					return L"DKVK_F12"; 
	case DKVK_F13:					return L"DKVK_F13"; 
	case DKVK_F14:					return L"DKVK_F14"; 
	case DKVK_F15:					return L"DKVK_F15"; 
	case DKVK_F16:					return L"DKVK_F16"; 
	case DKVK_F17:					return L"DKVK_F17"; 
	case DKVK_F18:					return L"DKVK_F18"; 
	case DKVK_F19:					return L"DKVK_F19"; 
	case DKVK_F20:					return L"DKVK_F20"; 

	case DKVK_0:					return L"DKVK_0"; 
	case DKVK_1:					return L"DKVK_1"; 
	case DKVK_2:					return L"DKVK_2"; 
	case DKVK_3:					return L"DKVK_3"; 
	case DKVK_4:					return L"DKVK_4"; 
	case DKVK_5:					return L"DKVK_5"; 
	case DKVK_6:					return L"DKVK_6"; 
	case DKVK_7:					return L"DKVK_7"; 
	case DKVK_8:					return L"DKVK_8"; 
	case DKVK_9:					return L"DKVK_9"; 

	case DKVK_A:					return L"DKVK_A"; 
	case DKVK_B:					return L"DKVK_B"; 
	case DKVK_C:					return L"DKVK_C"; 
	case DKVK_D:					return L"DKVK_D"; 
	case DKVK_E:					return L"DKVK_E"; 
	case DKVK_F:					return L"DKVK_F"; 
	case DKVK_G:					return L"DKVK_G"; 
	case DKVK_H:					return L"DKVK_H"; 
	case DKVK_I:					return L"DKVK_I"; 
	case DKVK_J:					return L"DKVK_J"; 
	case DKVK_K:					return L"DKVK_K"; 
	case DKVK_L:					return L"DKVK_L"; 
	case DKVK_M:					return L"DKVK_M"; 
	case DKVK_N:					return L"DKVK_N"; 
	case DKVK_O:					return L"DKVK_O"; 
	case DKVK_P:					return L"DKVK_P"; 
	case DKVK_Q:					return L"DKVK_Q"; 
	case DKVK_R:					return L"DKVK_R"; 
	case DKVK_S:					return L"DKVK_S"; 
	case DKVK_T:					return L"DKVK_T"; 
	case DKVK_U:					return L"DKVK_U"; 
	case DKVK_V:					return L"DKVK_V"; 
	case DKVK_W:					return L"DKVK_W"; 
	case DKVK_X:					return L"DKVK_X"; 
	case DKVK_Y:					return L"DKVK_Y"; 
	case DKVK_Z:					return L"DKVK_Z"; 

	case DKVK_PERIOD:				return L"DKVK_PERIOD"; 
	case DKVK_COMMA:				return L"DKVK_COMMA"; 
	case DKVK_SLASH:				return L"DKVK_SLASH"; 
	case DKVK_TAB:					return L"DKVK_TAB"; 
	case DKVK_ACCENT_TILDE:			return L"DKVK_ACCENT_TILDE"; 
	case DKVK_BACKSPACE:			return L"DKVK_BACKSPACE"; 
	case DKVK_SEMICOLON:			return L"DKVK_SEMICOLON"; 
	case DKVK_QUOTE:				return L"DKVK_QUOTE"; 
	case DKVK_BACKSLASH:			return L"DKVK_BACKSLASH"; 
	case DKVK_EQUAL:				return L"DKVK_EQUAL"; 
	case DKVK_HYPHEN:				return L"DKVK_HYPHEN"; 
	case DKVK_SPACE:				return L"DKVK_SPACE"; 
	case DKVK_OPEN_BRACKET:			return L"DKVK_OPEN_BRACKET"; 
	case DKVK_CLOSE_BRACKET:		return L"DKVK_CLOSE_BRACKET"; 
	case DKVK_CAPSLOCK:				return L"DKVK_CAPSLOCK"; 

	case DKVK_RETURN:				return L"DKVK_RETURN"; 

	case DKVK_FN:					return L"DKVK_FN";
	case DKVK_INSERT:				return L"DKVK_INSERT"; 
	case DKVK_HOME:					return L"DKVK_HOME"; 
	case DKVK_PAGE_UP:				return L"DKVK_PAGE_UP"; 
	case DKVK_PAGE_DOWN:			return L"DKVK_PAGE_DOWN"; 
	case DKVK_END:					return L"DKVK_END"; 
	case DKVK_DELETE:				return L"DKVK_DELETE"; 

	case DKVK_LEFT:					return L"DKVK_LEFT"; 
	case DKVK_RIGHT:				return L"DKVK_RIGHT"; 
	case DKVK_UP:					return L"DKVK_UP"; 
	case DKVK_DOWN:					return L"DKVK_DOWN"; 

	case DKVK_LEFT_SHIFT:			return L"DKVK_LSHIFT"; 
	case DKVK_RIGHT_SHIFT:			return L"DKVK_RSHIFT"; 
	case DKVK_LEFT_OPTION:			return L"DKVK_LOPTION"; 
	case DKVK_RIGHT_OPTION:			return L"DKVK_ROPTION"; 
	case DKVK_LEFT_CONTROL:			return L"DKVK_LCONTROL"; 
	case DKVK_RIGHT_CONTROL:		return L"DKVK_RCONTROL"; 
	case DKVK_LEFT_COMMAND:			return L"DKVK_LCOMMAND"; 
	case DKVK_RIGHT_COMMAND:		return L"DKVK_RCOMMAND"; 

	case DKVK_PAD_0:				return L"DKVK_PAD_0"; 
	case DKVK_PAD_1:				return L"DKVK_PAD_1"; 
	case DKVK_PAD_2:				return L"DKVK_PAD_2"; 
	case DKVK_PAD_3:				return L"DKVK_PAD_3"; 
	case DKVK_PAD_4:				return L"DKVK_PAD_4"; 
	case DKVK_PAD_5:				return L"DKVK_PAD_5"; 
	case DKVK_PAD_6:				return L"DKVK_PAD_6"; 
	case DKVK_PAD_7:				return L"DKVK_PAD_7"; 
	case DKVK_PAD_8:				return L"DKVK_PAD_8"; 
	case DKVK_PAD_9:				return L"DKVK_PAD_9"; 
	case DKVK_ENTER:				return L"DKVK_ENTER"; 
	case DKVK_NUMLOCK:				return L"DKVK_NUMLOCK"; 
	case DKVK_PAD_SLASH:			return L"DKVK_PAD_SLASH"; 
	case DKVK_PAD_ASTERISK:			return L"DKVK_PAD_ASTERISK"; 
	case DKVK_PAD_PLUS:				return L"DKVK_PAD_PLUS"; 
	case DKVK_PAD_MINUS:			return L"DKVK_PAD_MINUS"; 
	case DKVK_PAD_EQUAL:			return L"DKVK_PAD_EQUL"; 
	case DKVK_PAD_PERIOD:			return L"DKVK_PAD_PERIOD"; 

	case DKVK_MAXVALUE:				return L"DKVK_MAXVALUE";
	default:						return L"DKVK_NONE";
	}

	return L"Error";
}
