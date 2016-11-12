//
//  File: Application.cpp
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
#include <windows.h>
#include "Application.h"

using namespace DKFramework;
using namespace DKFramework::Private::Win32;

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
	static AppLogger logger;
	return &logger;
}

DKString Application::DefaultPath(SystemPath)
{
	return "";
}

DKString Application::EnvironmentString(EnvironmentVariable)
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
#endif // _WIN32
