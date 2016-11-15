//
//  File: AppEventLoop.mm
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import <AppKit/AppKit.h>
#include "AppEventLoop.h"

using namespace DKFramework;
using namespace DKFramework::Private::macOS;

AppEventLoop::AppEventLoop(DKApplication* app)
: appInstance(app)
, running(false)
, runLoop(nil)
, timer(nil)
{
}

AppEventLoop::~AppEventLoop(void)
{
}

bool AppEventLoop::Run(void)
{
	if (BindThread())
	{
		running = true;

		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

		lock.Lock();
		runLoop = CFRunLoopGetCurrent();
		CFRetain(runLoop);
		lock.Unlock();

		NSApplication* app = [NSApplication sharedApplication];

		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
		// install observer for system-time change notification
		id<NSObject> timeChangeObserver = [center addObserverForName:NSSystemClockDidChangeNotification
															  object:nil
															   queue:nil
														  usingBlock:^(NSNotification *note) {
															  this->DispatchAndInstallTimer();
														  }];


		DKApplicationInterface::AppInitialize(appInstance);

		if (running)
		{
			CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
				this->DispatchAndInstallTimer();
			});
			[app run];
		}

		DKApplicationInterface::AppFinalize(appInstance);

		// uninstall observer
		[center removeObserver:timeChangeObserver];

		lock.Lock();
		CFRelease(runLoop);
		runLoop = nil;
		lock.Unlock();

		if (timer)
		{
			[timer invalidate];
			[timer release];
			timer = nil;
		}

		[pool release];

		running = false;
		UnbindThread();
		return true;
	}
	return false;
}

void AppEventLoop::Stop(void)
{
	lock.Lock();
	if (runLoop)
	{
		CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
			this->running = false;
			NSApplication* app = [NSApplication sharedApplication];
			if (app.running)
			{
				// To stop run-loop immediately, call 'stop:' during process some NSEvent object.
				// Generate dummy NSEvent object.
				NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
													location:NSZeroPoint
											   modifierFlags:0
												   timestamp:[NSProcessInfo processInfo].systemUptime
												windowNumber:0
													 context:nil
													 subtype:0
													   data1:0
													   data2:0];

				[app postEvent:event atStart:YES];
				[app stop:nil];
			}
		});
		CFRunLoopWakeUp(runLoop);
	}
	else
		this->running = false;
	lock.Unlock();
	CFRunLoopStop(runLoop);
}

void AppEventLoop::DispatchAndInstallTimer(void)
{
	if (timer)
	{
		[timer invalidate];
		[timer release];
		timer = nil;
	}
	int count = 0;
	while (this->running && Dispatch()) { count++; }
	//NSLog(@" < Dispatched: %d / Running: %d >", count, this->running);
	if (this->running)
	{
		double intv = PendingEventInterval();
		if (intv >= 0.0)
		{
			//NSLog(@"Install timer with interval: %f", intv);
			// install timer for next pending event.
			timer = [NSTimer scheduledTimerWithTimeInterval:intv
													repeats:NO
													  block:^(NSTimer* t) {
														  this->DispatchAndInstallTimer();
													  }];
			[timer retain];
		}
	}
}

DKObject<DKEventLoop::PendingState> AppEventLoop::Post(const DKOperation* operation, double delay)
{
	DKObject<PendingState> ps = DKEventLoop::Post(operation, delay);

	lock.Lock();
	if (runLoop)
	{
		CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
			this->DispatchAndInstallTimer();
		});
		CFRunLoopWakeUp(runLoop);
	}
	lock.Unlock();
	return ps;
}

DKObject<DKEventLoop::PendingState> AppEventLoop::Post(const DKOperation* operation, const DKDateTime& runAfter)
{
	DKObject<PendingState> ps = DKEventLoop::Post(operation, runAfter);

	lock.Lock();
	if (runLoop)
	{
		CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
			this->DispatchAndInstallTimer();
		});
		CFRunLoopWakeUp(runLoop);
	}
	lock.Unlock();
	return ps;
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
