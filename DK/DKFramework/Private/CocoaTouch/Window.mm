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
#import "View.h"
#import "ViewController.h"
#include "Window.h"

using namespace DKFramework;
using namespace DKFramework::Private::iOS;

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
	return new Window(win);
}

#pragma mark - Window implementation
Window::Window(DKWindow* w)
: instance(w)
, window(nil)
, view(nil)
{
}

Window::~Window()
{
	DKASSERT_DEBUG(window == nil);
	DKASSERT_DEBUG(view == nil);
}

DKRect Window::ContentRect()
{
	CGRect rect = view.bounds;
	if (view.window)
		rect = [view convertRect:rect toView:nil];
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

DKRect Window::WindowRect()
{
	CGRect rect = view.frame;
	if (view.window)
		rect = view.window.frame;
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

#pragma mark - DKWindowInterface overrides
bool Window::Create(const DKString& title, uint32_t style)
{
	return DispatchSyncOnMain([&]() {
		window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
		UIViewController* viewController = [[[_DKViewController alloc] init] autorelease];
		view = viewController.view;

		if (style & DKWindow::StyleAutoResize)
			view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		else
			view.autoresizingMask = UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleBottomMargin;

		((_DKView*)view).userInstance = instance;
		viewController.title = [NSString stringWithUTF8String:(const char*)DKStringU8(title)];

		window.hidden = YES;
		window.rootViewController = viewController;

		instance->PostWindowEvent({
			WindowEvent::WindowCreated,
			WindowRect(),
			ContentRect(),
			(float)view.contentScaleFactor
		});
		return true;
	});
}

bool Window::CreateProxy(void* systemHandle)
{
	DKASSERT_DEBUG(window == nil);
	DKASSERT_DEBUG(view == nil);

	UIView* view = (id)systemHandle;
	if ([view isKindOfClass:[UIView class]])
	{
		if ([[view.class layerClass] isSubclassOfClass:[CAMetalLayer class]])
		{
			this->view = [view retain];
			return true;
		}
		NSLog(@"DKWindow::CreateProxy() failed: [UIView layerClass] should return CAMetalLayer.");
	}
	return false;
}

bool Window::IsProxy() const
{
	DKASSERT_DEBUG(view != nil);
	return window == nil;
}

void Window::UpdateProxy()
{
	if (IsProxy())
	{
		dispatch_async(dispatch_get_main_queue(), ^(){
			CGFloat contentScaleFactor = view.contentScaleFactor;

			instance->PostWindowEvent({
				WindowEvent::WindowResized,
				WindowRect(),
				ContentRect(),
				(float)contentScaleFactor
			});
		});
	}
}

void Window::Destroy()
{
	DispatchSyncOnMain([&](){
		if (window)
		{
			((_DKView*)view).userInstance = NULL;
		}

		[window release];
		[view release];
		window = nil;
		view = nil;
	});
}

void* Window::PlatformHandle() const
{
	return view;
}

bool Window::IsValid() const
{
	return view.window != nil;
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

void Window::Show()
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		view.hidden = NO;
	});
}

void Window::Hide()
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		view.hidden = YES;
	});
}

void Window::Activate()
{
	dispatch_async(dispatch_get_main_queue(), ^(){
		view.hidden = NO;
		[window makeKeyAndVisible];
		[view becomeFirstResponder];
	});
}

void Window::Minimize()
{
}

void Window::SetOrigin(DKPoint pt)
{
    CGPoint origin = CGPointMake(pt.x, pt.y);
	dispatch_async(dispatch_get_main_queue(), ^(){
		CGRect frame = view.frame;
		frame.origin = origin;
		view.frame = frame;
	});
}

void Window::Resize(DKSize s, const DKPoint* pt)
{
    CGSize size = CGSizeMake(s.width, s.height);
    CGPoint origin;
    BOOL updateOrigin = NO;
    if (pt)
    {
        origin = CGPointMake(pt->x, pt->y);
        updateOrigin = YES;
    }
    
	dispatch_async(dispatch_get_main_queue(), ^(){
		CGRect frame = view.frame;
		frame.size = size;
		if (updateOrigin)
			frame.origin = origin;

		view.frame = frame;
	});
}

float Window::ContentScaleFactor() const
{
	return DispatchSyncOnMain([&]()->CGFloat {
		return view.contentScaleFactor;
	});
}

void Window::SetTitle(const DKString& title)
{
	DispatchSyncOnMain([&](){
		if (view.window && view.window.rootViewController)
			view.window.rootViewController.title = [NSString stringWithUTF8String:(const char*)DKStringU8(title)];
	});
}

DKString Window::Title() const
{
	return DispatchSyncOnMain([&]() -> DKString {
		if (view.window && view.window.rootViewController)
			return view.window.rootViewController.title.UTF8String;
		return "";
	});
}

void Window::SetMousePosition(int deviceId, DKPoint pt)
{
}

DKPoint Window::MousePosition(int deviceId) const
{
	return DispatchSyncOnMain([&](){
		if ([view isKindOfClass:[_DKView class]])
			return [(_DKView*)view touchPositionAtIndex:deviceId];
		return DKPoint(-1,-1);
	});
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

bool Window::IsTextInputEnabled(int deviceId) const
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

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
