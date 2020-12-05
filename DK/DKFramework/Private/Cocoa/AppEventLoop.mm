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

#define PSKEY_APP_DELEGATE				"NSApplicationDelegate"

AppEventLoop::AppEventLoop(DKApplication* app)
: appInstance(app)
, threadId(DKThread::invalidId)
, running(false)
, runLoop(nil)
, timer(nil)
{
}

AppEventLoop::~AppEventLoop()
{
}

bool AppEventLoop::Run()
{
	if (!running && threadId == DKThread::invalidId)
	{
		running = true;
        threadId = DKThread::CurrentThreadId();

		@autoreleasepool {
			// initialize multi-threading mode
			if ([NSThread isMultiThreaded] == NO)
			{
				NSThread *thread = [[[NSThread alloc] init] autorelease];
				[thread start];
			}

			lock.Lock();
			runLoop = CFRunLoopGetCurrent();
			CFRetain(runLoop);
			lock.Unlock();

			NSApplication* app = [NSApplication sharedApplication];

			id appDelegate = nil;

			@autoreleasepool {
				DKPropertySet& config = DKPropertySet::SystemConfig();
				Class appDelegateClass = nil;
				for (const char* key : {PSKEY_APP_DELEGATE, "AppDelegate"})
				{
					if (config.HasValue(key))
					{
						const DKVariant& var = config.Value(key);
						if (var.ValueType() == DKVariant::TypeString)
						{
							DKStringU8 className = DKStringU8(var.String());
							appDelegateClass = NSClassFromString([NSString stringWithUTF8String:(const char*)className]);
							if (appDelegateClass)
								break;
						}
					}
				}
				if (appDelegateClass)
					appDelegate = [[appDelegateClass alloc] init];
			}

			if (appDelegate)
				app.delegate = appDelegate;

			NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

			// install notification observers.
			NSArray* observers =
			@[
			  [center addObserverForName:NSSystemClockDidChangeNotification
								  object:nil
								   queue:nil
							  usingBlock:^(NSNotification *note) {
								  this->DispatchAndInstallTimer();
							  }]
			  ];

			// initialize DKApplication instance.
			@autoreleasepool {
				DKApplicationInterface::AppInitialize(appInstance);
			}
			if (running)
			{
				CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^() {
					this->DispatchAndInstallTimer();
				});
				[app run];
			}
			// finalize DKApplication instance.
			@autoreleasepool {
				DKApplicationInterface::AppFinalize(appInstance);
			}

			// uninstall observer
			for (id ob in observers)
				[center removeObserver:ob];

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
			
			[appDelegate release];
		}

        threadId = DKThread::invalidId;
		running = false;
		return true;
	}
	return false;
}

void AppEventLoop::Stop()
{
	lock.Lock();
	if (runLoop)
	{
		CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^() {
			this->running = false;
			NSApplication* app = [NSApplication sharedApplication];
			if (app.running)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:NSApplicationWillTerminateNotification
																	object:app];

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

void AppEventLoop::DispatchAndInstallTimer()
{
	if (timer)
	{
		[timer invalidate];
		[timer release];
		timer = nil;
	}
	int count = 0;
	while (this->running && Execute()) { count++; }
	//NSLog(@" < Dispatched: %d / Running: %d >", count, this->running);
	if (this->running)
	{
		double intv = NextDispatchInterval();
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

DKObject<DKDispatchQueue::ExecutionState> AppEventLoop::Submit(DKOperation* operation, double delay)
{
	DKObject<DKDispatchQueue::ExecutionState> es = DKDispatchQueue::Submit(operation, delay);

	lock.Lock();
	if (runLoop)
	{
		CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^() {
			this->DispatchAndInstallTimer();
		});
		CFRunLoopWakeUp(runLoop);
	}
	lock.Unlock();
	return es;
}

bool AppEventLoop::IsRunning() const
{
    return this->running;
}

bool AppEventLoop::IsDispatchThread() const
{
    return DKThread::CurrentThreadId() == threadId;
}
#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
