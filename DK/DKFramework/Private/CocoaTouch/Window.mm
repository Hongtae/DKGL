//
//  File: Window.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#include "Window.h"

using namespace DKFramework;
using namespace DKFramework::Private::iOS;

#pragma mark - DKWindowInterface
DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
{
	return new Window(win);
}

#pragma mark - Window implementation
Window::Window(DKWindow* w)
: instance(w)
, window(nil)
, view(nil)
{
}

Window::~Window(void)
{
	DKASSERT_DEBUG(window == nil);
	DKASSERT_DEBUG(view == nil);
}

#pragma mark - DKWindowInterface override
bool Window::Create(const DKString& title, uint32_t style)
{
	return false;
}

bool Window::CreateProxy(void* systemHandle)
{
	return false;
}

bool Window::IsProxy(void) const
{
	return false;
}

void Window::UpdateProxy(void)
{
}

void Window::Destroy(void)
{
}

void* Window::PlatformHandle(void) const
{
	return NULL;
}

bool Window::IsValid(void) const
{
	return false;
}

void Window::ShowMouse(int deviceId, bool show)
{
}

bool Window::IsMouseVisible(int deviceId) const
{
	return false;
}

void Window::HoldMouse(int deviceId, bool hold)
{
}

bool Window::IsMouseHeld(int deviceId) const
{
	return false;
}

void Window::Show(void)
{
}

void Window::Hide(void)
{
}

void Window::Activate(void)
{
}

void Window::Minimize(void)
{
}


void Window::SetOrigin(const DKPoint&)
{
}

void Window::Resize(const DKSize&, const DKPoint* optionalOrigin)
{
}

double Window::ContentScaleFactor(void) const
{
	return 1.0;
}

void Window::SetTitle(const DKString& title)
{
}

DKString Window::Title(void) const
{
	return "";
}

void Window::SetMousePosition(int deviceId, const DKPoint& pt)
{
}

DKPoint Window::MousePosition(int deviceId) const
{
	return DKPoint(-1,-1);
}

void Window::EnableTextInput(int deviceId, bool enable)
{
}

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
