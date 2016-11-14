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
{
}

AppEventLoop::~AppEventLoop(void)
{
}

bool AppEventLoop::Run(void)
{
	return DKEventLoop::Run();
}

void AppEventLoop::Stop(void)
{
	return DKEventLoop::Stop();
}

DKObject<DKEventLoop::PendingState> AppEventLoop::Post(const DKOperation* operation, double delay)
{
	return DKEventLoop::Post(operation, delay);
}

DKObject<DKEventLoop::PendingState> AppEventLoop::Post(const DKOperation* operation, const DKDateTime& runAfter)
{
	return DKEventLoop::Post(operation, runAfter);
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
