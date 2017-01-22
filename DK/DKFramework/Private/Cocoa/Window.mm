//
//  File: Window.mm
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import <AppKit/AppKit.h>
#import "View.h"
#include "Window.h"

using namespace DKFramework;
using namespace DKFramework::Private::macOS;

#pragma mark - _DKWindow - a dummy window class
@interface _DKWindow : NSWindow
@end
@implementation _DKWindow
- (BOOL)canBecomeKeyWindow		{ return YES; }
- (BOOL)canBecomeMainWindow		{ return YES; }
@end

#pragma mark - dispatch_sync (without deadlock)
template <typename Fn, typename Ret> struct DispatchSyncAndFetchResult
{
	static Ret Call(Fn&& fn)
	{
		if ([NSThread isMainThread] == NO)
		{
			__block Ret ret;
			dispatch_sync(dispatch_get_main_queue(), ^() {
				ret = fn();
			});
			return ret;
		}
		return fn();
	}
};
template <typename Fn> struct DispatchSyncAndFetchResult<Fn, void>
{
	static void Call(Fn&& fn)
	{
		if ([NSThread isMainThread])
			fn();
		else
			dispatch_sync(dispatch_get_main_queue(), ^() {
				fn();
			});
	}
};

template <typename Fn>
auto DispatchSyncOnMain(Fn&& fn) -> typename DKFunctionType<Fn>::ReturnType
{
	using Ret = typename DKFunctionType<Fn>::ReturnType;
	return DispatchSyncAndFetchResult<Fn, Ret>::Call(std::forward<Fn>(fn));
}


#pragma mark - DKWindowInterface
DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
{
	return DKRawPtrNew<Window>(win);
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

DKRect Window::ContentRect(void)
{
	NSRect rect = view.bounds;
	if (view.window)
		rect = [view convertRect:rect toView:nil];
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

DKRect Window::WindowRect(void)
{
	NSRect rect = view.frame;
	if (view.window)
		rect = view.window.frame;
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

#pragma mark - DKWindowInterface overrides
bool Window::Create(const DKString& title, uint32_t style)
{
	return DispatchSyncOnMain([&]() {
		NSWindowStyleMask styleMask = 0;
		NSBackingStoreType bufferingType = NSBackingStoreBuffered;
		NSRect contentRect = NSMakeRect(0, 0, 640, 480);

		if (style & DKWindow::StyleTitle)
			styleMask |= NSWindowStyleMaskTitled;
		if (style & DKWindow::StyleCloseButton)
			styleMask |= NSWindowStyleMaskClosable;
		if (style & DKWindow::StyleMinimizeButton)
			styleMask |= NSWindowStyleMaskMiniaturizable;
		if (style & DKWindow::StyleMaximizeButton)
			(void)0;
		if (style & DKWindow::StyleResizableBorder)
			styleMask |= NSWindowStyleMaskResizable;

		window = [[_DKWindow alloc] initWithContentRect:contentRect
											  styleMask:styleMask
												backing:bufferingType
												  defer:YES];

		view = [[_DKView alloc] initWithFrame:contentRect];
		((_DKView*)view).userInstance = instance;

		window.contentView = view;
		window.delegate = (id<NSWindowDelegate>)view;
		window.releasedWhenClosed = NO;
		window.acceptsMouseMovedEvents = YES;
		window.title = [NSString stringWithUTF8String:(const char*)DKStringU8(title)];

		instance->PostWindowEvent({
			WindowEvent::WindowCreated,
			WindowRect(),
			ContentRect(),
			window.backingScaleFactor
		});
		return true;
	});
}

bool Window::CreateProxy(void* systemHandle)
{
	DKASSERT_DEBUG(window == nil);
	DKASSERT_DEBUG(view == nil);

	NSView* view = (id)systemHandle;
	if ([view isKindOfClass:[NSView class]])
	{
		this->view = [view retain];
		return true;
	}
	return false;
}

bool Window::IsProxy(void) const
{
	DKASSERT_DEBUG(view != nil);
	return window == nil;
}

void Window::UpdateProxy(void)
{
	if (IsProxy())
	{
		dispatch_async(dispatch_get_main_queue(), ^(){
			CGFloat backingScaleFactor = 1.0;
			if (view.window)
				backingScaleFactor = view.window.backingScaleFactor;

			instance->PostWindowEvent({
				WindowEvent::WindowResized,
				WindowRect(),
				ContentRect(),
				backingScaleFactor
			});
		});
	}
}

void Window::Destroy(void)
{
	DispatchSyncOnMain([&](){
		if (window)
		{
			((_DKView*)view).userInstance = NULL;
			[window close];
		}

		[window release];
		[view release];
		window = nil;
		view = nil;
	});
}

void* Window::PlatformHandle(void) const
{
	return view;
}

bool Window::IsValid(void) const
{
	return view.window != nil;
}

void Window::ShowMouse(int deviceId, bool show)
{
	if (deviceId == 0)
	{
		if (show)
		{
			CGDisplayShowCursor(kCGDirectMainDisplay);
			//[NSCursor unhide];
		}
		else
		{
			CGDisplayHideCursor(kCGDirectMainDisplay);
			//[NSCursor hide];
		}
	}
}

bool Window::IsMouseVisible(int deviceId) const
{
	if (deviceId == 0)
		return CGCursorIsVisible();

	return false;
}

void Window::HoldMouse(int deviceId, bool hold)
{
	if (deviceId == 0)
	{
		if ([view isKindOfClass:[_DKView class]])
		{
			((_DKView*)view).holdMouse = hold;
		}
	}
}

bool Window::IsMouseHeld(int deviceId) const
{
	if (deviceId == 0)
	{
		if ([view isKindOfClass:[_DKView class]])
		{
			return ((_DKView*)view).holdMouse;
		}
	}
	return false;
}

void Window::Show(void)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window)
		{
			[view.window orderFront: nil];

			instance->PostWindowEvent({
				WindowEvent::WindowShown,
				WindowRect(),
				ContentRect(),
				view.window.backingScaleFactor
			});
		}
	});
}

void Window::Hide(void)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window)
		{
			[view.window resignKeyWindow];
			[view.window orderOut:nil];

			instance->PostWindowEvent({
				WindowEvent::WindowHidden,
				WindowRect(),
				ContentRect(),
				view.window.backingScaleFactor
			});
		}
	});
}

void Window::Activate(void)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window)
		{
			[view.window makeKeyAndOrderFront:nil];

			instance->PostWindowEvent({
				WindowEvent::WindowShown,
				WindowRect(),
				ContentRect(),
				view.window.backingScaleFactor
			});
			instance->PostWindowEvent({
				WindowEvent::WindowActivated,
				WindowRect(),
				ContentRect(),
				view.window.backingScaleFactor
			});
		}
	});
}

void Window::Minimize(void)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window)
		{
			[view.window miniaturize: nil];
		}
	});
}

void Window::SetOrigin(const DKPoint& pt)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window && view.window.contentView == view)
			[view.window setFrameOrigin:NSMakePoint(pt.x, pt.y)];
		else
		{
			NSRect frame = view.frame;
			frame.origin = NSMakePoint(pt.x, pt.y);
			view.frame = frame;
		}
	});
}

void Window::Resize(const DKSize& s, const DKPoint* pt)
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		if (view.window && view.window.contentView == view)
		{
			view.window.contentSize = NSMakeSize(s.width, s.height);
			if (pt)
				[view.window setFrameOrigin:NSMakePoint(pt->x, pt->y)];
			[view.window displayIfNeeded];
		}
		else
		{
			NSRect frame = view.frame;
			frame.size = NSMakeSize(s.width, s.height);
			if (pt)
				frame.origin = NSMakePoint(pt->x, pt->y);
			view.frame = frame;
			if (view.window)
				[view.window layoutIfNeeded];
		}
	});
}

double Window::ContentScaleFactor(void) const
{
	return DispatchSyncOnMain([&]()->CGFloat {
		if (view.window)
			return view.window.backingScaleFactor;
		return 1.0;
	});
}

void Window::SetTitle(const DKString& title)
{
	DispatchSyncOnMain([&](){
		if (view.window)
			view.window.title = [NSString stringWithUTF8String:(const char*)DKStringU8(title)];
	});
}

DKString Window::Title(void) const
{
	return DispatchSyncOnMain([&]() -> DKString {
		if (view.window)
			return view.window.title.UTF8String;
		return "";
	});
}

void Window::SetMousePosition(int deviceId, const DKPoint& pt)
{
	if (deviceId == 0)
	{
		DispatchSyncOnMain([&](){
			if ([view isKindOfClass:[_DKView class]])
				((_DKView*)view).mousePosition = pt;
		});
	}
}

DKPoint Window::MousePosition(int deviceId) const
{
	if (deviceId == 0)
	{
		return DispatchSyncOnMain([&](){
			if ([view isKindOfClass:[_DKView class]])
				return ((_DKView*)view).mousePosition;
			return DKPoint(-1,-1);
		});
	}
	return DKPoint(-1,-1);
}

void Window::EnableTextInput(int deviceId, bool enable)
{
	if (deviceId == 0)
	{
		DispatchSyncOnMain([&](){
			if ([view isKindOfClass:[_DKView class]])
				((_DKView*)view).textInput = enable;
		});
	}
}

bool Window::IsTextInputEnabled(int deviceId)
{
	if (deviceId == 0)
	{
		return DispatchSyncOnMain([&]()->bool {
			if ([view isKindOfClass:[_DKView class]])
				return ((_DKView*)view).textInput;
			return false;
		});
	}
	return false;
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
