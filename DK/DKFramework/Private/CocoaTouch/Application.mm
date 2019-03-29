//
//  File: Application.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#include "Application.h"

using namespace DKFramework;
using namespace DKFramework::Private::iOS;

DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app, int argc, char* argv[])
{
	return new Application(app);
}

Application::Application(DKApplication* app)
: mainLoop(app)
{
}

Application::~Application()
{
}

DKEventLoop* Application::EventLoop()
{
	return &mainLoop;
}

DKLogger* Application::DefaultLogger()
{
	struct Logger : public DKLogger
	{
		void Log(Category c, const DKString& msg) override
		{
			switch (c)
			{
				case Category::Verbose:
				case Category::Info:
				case Category::Debug:
				case Category::Warning:
					NSLog(@"[%c] %@", c, [NSString stringWithUTF8String:(const char*)DKStringU8(msg)]);
					break;
				case Category::Error:
					NSLog(@"<ERROR> %@", [NSString stringWithUTF8String:(const char*)DKStringU8(msg)]);
					break;
				default:
					NSLog(@"[0x%X] %@", c, [NSString stringWithUTF8String:(const char*)DKStringU8(msg)]);
					break;
			}
		}
	};
	static Logger logger;
	return &logger;
}

DKString Application::DefaultPath(SystemPath sp)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

	auto searchPath = [](NSSearchPathDirectory path) -> DKString
	{
		NSFileManager* fileManager = [[[NSFileManager alloc] init] autorelease];
		NSURL* url = [fileManager URLForDirectory:path inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
		return [url.path UTF8String];
	};

	DKString path;
	switch (sp)
	{
		case SystemPath::SystemRoot:
			path = L"/";
			break;
		case SystemPath::AppRoot:
			path = [[[NSBundle mainBundle] bundlePath] UTF8String];
			break;
		case SystemPath::AppResource:
			path = [[[NSBundle mainBundle] resourcePath] UTF8String];
			break;
		case SystemPath::AppExecutable:
			path = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] UTF8String];
			break;
		case SystemPath::AppData:
			path = searchPath(NSApplicationSupportDirectory);
			break;
		case SystemPath::UserHome:
			path = [NSHomeDirectory() UTF8String];
			break;
		case SystemPath::UserDocuments:
			path = searchPath(NSDocumentDirectory);
			break;
		case SystemPath::UserPreferences:
			path = []()-> DKString {
				NSFileManager* fileManager = [[[NSFileManager alloc] init] autorelease];
				NSURL* url = [fileManager URLForDirectory:NSLibraryDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
				NSString* path = [url URLByAppendingPathComponent:@"Preferences"].path;
				BOOL isDir = NO;
				if ([fileManager fileExistsAtPath:path isDirectory:&isDir] == NO) // file not exists
				{
					if ([fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil] == NO)
					{
						NSString* errDesc = [NSString stringWithFormat:@"Cannot create directory:%@", path];
						DKERROR_THROW((const char*)[errDesc UTF8String]);
						NSLog(@"%@\n", errDesc);
					}
				}
				else if (!isDir)	// file exists but not dir.
				{
					NSString* errDesc = [NSString stringWithFormat:@"Destination path:%@ is not a directory!", path];
					DKERROR_THROW((const char*)[errDesc UTF8String]);
					NSLog(@"%@\n", errDesc);
				}
				return [path UTF8String];
			}();
			break;
		case SystemPath::UserCache:
			path = searchPath(NSCachesDirectory);
			break;
		case SystemPath::UserTemp:
			path = [NSTemporaryDirectory() UTF8String];
			break;
	}
	[pool release];
	return path;
}

DKString Application::ProcessInfoString(ProcessInfo pi)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	DKString value;
	switch (pi)
	{
		case ProcessInfo::HostName:
			value = [[[NSProcessInfo processInfo] hostName] UTF8String];
			break;
		case ProcessInfo::OsName:
			value = [[NSString stringWithFormat:@"Apple macOS %@",
					  [[NSProcessInfo processInfo] operatingSystemVersionString]] UTF8String];
			break;
		case ProcessInfo::UserName:
			value = [NSUserName() UTF8String];
			break;
		case ProcessInfo::ModulePath:
			value = [[[NSBundle mainBundle] executablePath] UTF8String];
			break;
	}
	[pool release];
	return value;
}

DKObject<DKData> Application::LoadResource(const DKString& res, DKAllocator& alloc)
{
	DKObject<DKData> ret = NULL;
	DKObject<DKDirectory> dir = DKDirectory::OpenDir(DefaultPath(SystemPath::AppResource));
	if (dir)
	{
		DKObject<DKFile> file = dir->OpenFile(res, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll);
		if (file)
		{
			DKFile::FileInfo info;
			if (file->GetInfo(info))
				ret = file->Read(info.size, alloc);
		}
	}
	return ret;
}

DKObject<DKData> Application::LoadStaticResource(const DKString& res)
{
	DKObject<DKData> ret = NULL;
	DKObject<DKDirectory> dir = DKDirectory::OpenDir(DefaultPath(SystemPath::AppResource));
	if (dir)
	{
		ret = dir->MapFile(res, 0, false);
	}
	return ret;
}

DKRect Application::DisplayBounds(int displayId) const
{
	return ScreenContentBounds(displayId);
}

DKRect Application::ScreenContentBounds(int displayId) const
{
	DKRect bounds(0,0,0,0);
	if (displayId >= 0)
	{
		@autoreleasepool {
			NSArray<UIScreen*>* screens = UIScreen.screens;
			if (displayId < screens.count)
			{
				CGRect b = screens[displayId].bounds;	// display bounds, measured in points
				bounds = DKRect(b.origin.x, b.origin.y, b.size.width, b.size.height);
			}
		}
	}
	return bounds;
}

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
