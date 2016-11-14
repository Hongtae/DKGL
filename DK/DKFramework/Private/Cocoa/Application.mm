//
//  File: Application.mm
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import <AppKit/AppKit.h>
#include "Application.h"

using namespace DKFramework;
using namespace DKFramework::Private::macOS;

DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app)
{
	return new Application(app);
}

Application::Application(DKApplication* app)
: mainLoop(app)
{
}

Application::~Application(void)
{
}

DKEventLoop* Application::EventLoop(void)
{
	return &mainLoop;
}

DKLogger* Application::DefaultLogger(void)
{
	struct Logger : public DKLogger
	{
		void Log(const DKString& msg) override
		{
			NSLog(@"%@", [NSString stringWithUTF8String:(const char*)DKStringU8(msg)]);
		}
	};
	static Logger logger;
	return &logger;
}

DKString Application::DefaultPath(SystemPath)
{
	return "";
}

DKString Application::ProcessInfoString(ProcessInfo)
{
	return "";
}

DKObject<DKData> Application::LoadResource(const DKString& res, DKAllocator& alloc)
{
	return NULL;
}

DKObject<DKData> Application::LoadStaticResource(const DKString& res)
{
	return NULL;
}

DKRect Application::DisplayBounds(int displayId) const
{
	return DKRect();
}

DKRect Application::ScreenContentBounds(int displayId) const
{
	return DKRect();
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
