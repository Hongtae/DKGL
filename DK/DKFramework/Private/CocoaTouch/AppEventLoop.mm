//
//  File: AppEventLoop.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#include "AppEventLoop.h"

using namespace DKFramework;
using namespace DKFramework::Private::iOS;

#define PSKEY_APP_DELEGATE				"UIApplicationDelegate"
#define DKAPP_INITIALIZE_NOTIFICATION	@"DKAPP_INITIALIZE_NOTIFICATION"
#define DKAPP_TERMINATE_NOTIFICATION	@"DKAPP_TERMINATE_NOTIFICATION"

#pragma mark - _DKAppLoader
@interface _DKAppLoader : NSObject<UIApplicationDelegate>
{
	id<UIApplicationDelegate> appDelegate;
}
@end

@implementation _DKAppLoader
- (instancetype)init
{
	self = [super init];
	if (self)
	{
		appDelegate = nil;

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
	return self;
}

- (void)dealloc
{
	[appDelegate release];
	[super dealloc];
}

- (BOOL)application:(UIApplication *)application willFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	[[self retain] autorelease];
	application.delegate = appDelegate;

	[[NSNotificationCenter defaultCenter] postNotificationName:DKAPP_INITIALIZE_NOTIFICATION object:nil];

	if (application.delegate != self)
	{
		if ([application.delegate respondsToSelector:@selector(application:willFinishLaunchingWithOptions:)])
			return [application.delegate application:application willFinishLaunchingWithOptions:launchOptions];
	}
	return YES;
}
@end

#pragma mark - AppEventLoop

AppEventLoop::AppEventLoop(DKApplication* app)
: appInstance(app)
, timer(nil)
, runLoop(nil)
{
}

AppEventLoop::~AppEventLoop(void)
{
}

bool AppEventLoop::Run(void)
{
	if (BindThread())
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

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

		NSNotificationCenter *center = [NSNotificationCenter defaultCenter];

		__block BOOL initialized = NO;
		__block id<UIApplicationDelegate> appDelegate = nil;

		// Install notification observers.
		NSArray* observers =
		@[
		  [center addObserverForName:DKAPP_INITIALIZE_NOTIFICATION
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   [appDelegate autorelease];
			   appDelegate = [[UIApplication sharedApplication].delegate retain];

			   if (!initialized)
			   {
				   DKApplicationInterface::AppInitialize(appInstance);
				   initialized = YES;
			   }
		   }],
		  [center addObserverForName:UIApplicationWillTerminateNotification
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   if (initialized)
			   {
				   DKApplicationInterface::AppFinalize(appInstance);
				   initialized = NO;
			   }
		   }],
		  [center addObserverForName:DKAPP_TERMINATE_NOTIFICATION
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   // terminate app by calling DKEventLoop::Stop().
			   CFRunLoopRef runLoop = CFRunLoopGetCurrent();
			   CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
				   if (initialized)
				   {
					   UIApplication* app = [UIApplication sharedApplication];
					   id delegate = app.delegate;
					   if ([delegate respondsToSelector:@selector(applicationWillTerminate:)])
						   [delegate performSelector:@selector(applicationWillTerminate:) withObject:app];

					   [[NSNotificationCenter defaultCenter] postNotificationName:UIApplicationWillTerminateNotification
																		   object:app];
				   }
			   });
			   CFRunLoopPerformBlock(runLoop, kCFRunLoopCommonModes, ^(void) {
				   exit(0);
			   });
			   CFRunLoopWakeUp(runLoop);
		   }],
		  [center addObserverForName:UIApplicationWillEnterForegroundNotification
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   this->DispatchAndInstallTimer();
		   }],
		  [center addObserverForName:UIApplicationSignificantTimeChangeNotification
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   this->DispatchAndInstallTimer();
		   }],
		  [center addObserverForName:UIApplicationWillResignActiveNotification
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   size_t bytesPurged = DKAllocatorChain::Cleanup();
			   NSLog(@"%zu bytes purged.", bytesPurged);
		   }],
		  [center addObserverForName:UIApplicationDidReceiveMemoryWarningNotification
							  object:nil
							   queue:nil
						  usingBlock:^(NSNotification *note)
		   {
			   NSLog(@"\n"
					 @"################################################################################\n"
					 @"#####                         Memory Warning!!                              ####\n"
					 @"################################################################################\n");
			   size_t bytesPurged = DKAllocatorChain::Cleanup();
			   NSLog(@"%zu bytes purged.", bytesPurged);
		   }],
		  ];
		
		UIApplicationMain(0, 0, nil, NSStringFromClass([_DKAppLoader class]));

		// below codes will never be executed!
		[appDelegate autorelease];

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
		[pool release];

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
			[[NSNotificationCenter defaultCenter] postNotificationName:DKAPP_TERMINATE_NOTIFICATION object:nil];
		});
		CFRunLoopWakeUp(runLoop);
	}
	lock.Unlock();
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
	while (Dispatch()) { count++; }
	//NSLog(@" < Dispatched: %d / Running: %d >", count, this->running);
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

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
