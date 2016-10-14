//
//  File: DKApplication_OSX.mm
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#warning Compiling DKApplication for Mac OS X

#import <AppKit/AppKit.h>
#import "DKApplication_OSX.h"

using namespace DKGL;
using namespace DKGL::Private;


#pragma mark - DKAppOperationWrapper
// DKAppOperationWrapper: DKOperation Wrapper Class
@interface DKAppOperationWrapper : NSObject
{
	DKObject<DKOperation> op;
}
- (void)invoke;
@end
@implementation DKAppOperationWrapper
-(instancetype)initWithOperation:(DKOperation*)operation
{
	self = [super init];
	if (self)
	{
		self->op = operation;
	}
	return self;
}
-(void)dealloc
{
	self->op = NULL;
	[super dealloc];
}
-(void)invoke
{
	if (self->op)
	{
		self->op->Perform();
	}
}
@end


////////////////////////////////////////////////////////////////////////////////
// NSApplicationDelegator
#pragma mark - NSApplicationDelegator
@interface DKApplicationDelegate : NSObject<NSApplicationDelegate>
{
@private
	DKApplication_OSX* mainAppImpl;
}
@property (readwrite, nonatomic) DKApplication_OSX* mainAppImpl;
@end

@implementation DKApplicationDelegate
@synthesize mainAppImpl;

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	return NSTerminateNow;
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return NO;
}
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
}
- (void)applicationWillBecomeActive:(NSNotification *)aNotification
{
}
- (void)applicationDidBecomeActive:(NSNotification *)aNotification
{
}
- (void)applicationWillResignActive:(NSNotification *)aNotification
{
}
- (void)applicationDidResignActive:(NSNotification *)aNotification
{
}
- (void)applicationWillHide:(NSNotification *)aNotification
{
}
- (void)applicationDidHide:(NSNotification *)aNotification
{
}
- (void)applicationWillUnhide:(NSNotification *)aNotification
{
}
- (void)applicationDidUnhide:(NSNotification *)aNotification
{
}
/*
- (void)applicationWillUpdate:(NSNotification *)aNotification
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}
- (void)applicationDidUpdate:(NSNotification *)aNotification
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}
*/
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return nil;
}
- (NSError *)application:(NSApplication *)application willPresentError:(NSError *)error
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return error;
}
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(id)sender openFileWithoutUI:(NSString *)filename
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
}
- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NO;
}
- (NSApplicationPrintReply)application:(NSApplication *)application printFiles:(NSArray *)fileNames withSettings:(NSDictionary *)printSettings showPrintPanels:(BOOL)showPrintPanels
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	return NSPrintingFailure;
}
@end


////////////////////////////////////////////////////////////////////////////////
// DKApplication_OSX implementation
#pragma mark - DKApplication_OSX implementation
DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app)
{
	return new DKApplication_OSX(app);
}

DKApplication_OSX::DKApplication_OSX(DKApplication* app)
: mainApp(app)
, retCode(0)
, terminate(false)
{
}

DKApplication_OSX::~DKApplication_OSX(void)
{
}

int DKApplication_OSX::Run(DKArray<char*>& args)
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	NSApplication* app = [NSApplication sharedApplication];
	DKApplicationDelegate* appDelegator = [[DKApplicationDelegate alloc] init];
	appDelegator.mainAppImpl = this;
	
	[app setDelegate: appDelegator];

	terminate = false;

	AppInitialize(mainApp);

	if (!terminate)
		[app run];

	AppFinalize(mainApp);

	[appDelegator release];
	[pool release];
	
	return retCode;
}

void DKApplication_OSX::Terminate(int exitCode)
{
	if (terminate)
		return;
	terminate = true;
	retCode = exitCode;
	NSApplication* app = [NSApplication sharedApplication];
	[app performSelectorOnMainThread:@selector(stop:) withObject:nil waitUntilDone:NO];
//	[app performSelectorOnMainThread:@selector(terminate:) withObject:nil waitUntilDone:NO];
}

void DKApplication_OSX::PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone)
{
	if (op)
	{
		id obj = [[DKAppOperationWrapper alloc] initWithOperation:op];
		[obj performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:waitUntilDone];
		[obj autorelease];
	}
}

DKLogger& DKApplication_OSX::DefaultLogger(void)
{
	struct Logger : public DKLogger
	{
		void Log(const DKString& msg)
		{
			NSLog(@"%@", [NSString stringWithUTF8String:(const char*)DKStringU8(msg)]);
		}
	};
	static Logger logger;
	return logger;
}

DKString DKApplication_OSX::EnvironmentPath(SystemPath s)
{
	auto SearchPath = [](NSSearchPathDirectory path) -> DKString
	{
		NSFileManager* fileManager = [[[NSFileManager alloc] init] autorelease];
		NSURL* url = [fileManager URLForDirectory:path inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
		return [url.path UTF8String];
	};
	auto PreferencesPath = []()-> DKString
	{
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
	};

	DKString path;
	switch (s)
	{
		case SystemPath::SystemPathSystemRoot:
			path = L"/";
			break;
		case SystemPath::SystemPathAppRoot:
			path = [[[NSBundle mainBundle] bundlePath] UTF8String];
			break;
		case SystemPath::SystemPathAppResource:
			path = [[[NSBundle mainBundle] resourcePath] UTF8String];
			break;
		case SystemPath::SystemPathAppExecutable:
			path = [[[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] UTF8String];
			break;
		case SystemPath::SystemPathAppData:
			path = SearchPath(NSApplicationSupportDirectory);
			break;
		case SystemPath::SystemPathUserHome:
			path = [NSHomeDirectory() UTF8String];
			break;
		case SystemPath::SystemPathUserDocuments:
			path = SearchPath(NSDocumentDirectory);
			break;
		case SystemPath::SystemPathUserPreferences:
			path = PreferencesPath();
			break;
		case SystemPath::SystemPathUserCache:
			path = SearchPath(NSCachesDirectory);
			break;
		case SystemPath::SystemPathUserTemp:
			path = [NSTemporaryDirectory() UTF8String];
			break;
	}
	return path;
}

DKString DKApplication_OSX::ModulePath(void)
{
	return [[[NSBundle mainBundle] executablePath] UTF8String];
}

DKObject<DKData> DKApplication_OSX::LoadResource(const DKString& res, DKAllocator& alloc)
{
	DKObject<DKData> ret = NULL;
	DKObject<DKDirectory> dir = DKDirectory::OpenDir(EnvironmentPath(SystemPath::SystemPathAppResource));
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

DKObject<DKData> DKApplication_OSX::LoadStaticResource(const DKString& res)
{
	DKObject<DKData> ret = NULL;
	DKObject<DKDirectory> dir = DKDirectory::OpenDir(EnvironmentPath(SystemPath::SystemPathAppResource));
	if (dir)
	{
		ret = dir->MapFile(res, 0, false);
	}
	return ret;
}

DKRect DKApplication_OSX::DisplayBounds(int displayId) const
{
	if (displayId <= 0)
	{
		NSRect screenBounds = [[NSScreen mainScreen] frame];
		return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
	}
	return DKRect(0,0,0,0);
}

DKRect DKApplication_OSX::ScreenContentBounds(int displayId) const
{
	if (displayId <= 0)
	{
		NSRect screenBounds = [[NSScreen mainScreen] visibleFrame];
		return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
	}
	return DKRect(0,0,0,0);
}

DKString DKApplication_OSX::HostName(void) const
{
	return [[[NSProcessInfo processInfo] hostName] UTF8String];
}

DKString DKApplication_OSX::OSName(void) const
{
	NSString* osName = [[NSProcessInfo processInfo] operatingSystemName];
	NSString* osVer = [[NSProcessInfo processInfo] operatingSystemVersionString];

	return [[NSString stringWithFormat:@"@ (Mac OS X) @", osName, osVer] UTF8String];
}

DKString DKApplication_OSX::UserName(void) const
{
	return [NSUserName() UTF8String];
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
