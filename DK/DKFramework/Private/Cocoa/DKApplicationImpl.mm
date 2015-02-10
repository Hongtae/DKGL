//
//  File: DKApplicationImpl.mm
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2014 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#warning Compiling DKApplication for Mac OS X

#import <AppKit/AppKit.h>
#import "DKApplicationImpl.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
#if defined(__BIG_ENDIAN__)
#define NSSTRING_ENCODING_WCHAR		NSUTF32BigEndianStringEncoding
#elif defined(__LITTLE_ENDIAN__)
#define NSSTRING_ENCODING_WCHAR		NSUTF32LittleEndianStringEncoding
#else
#error System endianness not defined.
#endif
		static_assert(sizeof(wchar_t) == 4, "wchar_t must be UTF-32 in OS X");

		DKString NSStringToIGString(NSString *str)
		{
			return (const wchar_t*)[str cStringUsingEncoding:NSSTRING_ENCODING_WCHAR];
		}
		NSString* DKStringToNSString(const DKFoundation::DKString& str)
		{
			size_t len = str.Bytes();
			if (len > 0)
				return [[[NSString alloc] initWithBytes:(const wchar_t*)str length:len encoding:NSSTRING_ENCODING_WCHAR] autorelease];
			return @"";
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;


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
	DKApplicationImpl* mainAppImpl;
}
@property (readwrite, nonatomic) DKApplicationImpl* mainAppImpl;
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
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
- (void)applicationDidUpdate:(NSNotification *)aNotification
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
*/
- (BOOL)applicationShouldHandleReopen:(NSApplication *)theApplication hasVisibleWindows:(BOOL)flag
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return nil;
}
- (NSError *)application:(NSApplication *)application willPresentError:(NSError *)error
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return error;
}
- (void)applicationDidChangeScreenParameters:(NSNotification *)aNotification
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(id)sender openFileWithoutUI:(NSString *)filename
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
}
- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NO;
}
- (NSApplicationPrintReply)application:(NSApplication *)application printFiles:(NSArray *)fileNames withSettings:(NSDictionary *)printSettings showPrintPanels:(BOOL)showPrintPanels
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	return NSPrintingFailure;
}
@end


////////////////////////////////////////////////////////////////////////////////
// DKApplicationImpl implementation
#pragma mark - DKApplicationImpl implementation
DKApplicationInterface* DKApplicationInterface::CreateInterface(DKApplication* app)
{
	return new DKApplicationImpl(app);
}

DKApplicationImpl::DKApplicationImpl(DKApplication* app)
: mainApp(app)
, retCode(0)
, terminate(false)
{
}

DKApplicationImpl::~DKApplicationImpl(void)
{
}

int DKApplicationImpl::Run(DKArray<char*>& args)
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

void DKApplicationImpl::Terminate(int exitCode)
{
	if (terminate)
		return;
	terminate = true;
	retCode = exitCode;
	NSApplication* app = [NSApplication sharedApplication];
	[app performSelectorOnMainThread:@selector(stop:) withObject:nil waitUntilDone:NO];
//	[app performSelectorOnMainThread:@selector(terminate:) withObject:nil waitUntilDone:NO];
}

void DKApplicationImpl::PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone)
{
	if (op)
	{
		id obj = [[DKAppOperationWrapper alloc] initWithOperation:op];
		[obj performSelectorOnMainThread:@selector(invoke) withObject:nil waitUntilDone:waitUntilDone];
		[obj autorelease];
	}
}

DKLogger& DKApplicationImpl::DefaultLogger(void)
{
	struct Logger : public DKLogger
	{
		void Log(const DKString& msg)
		{
			NSLog(@"%@", DKStringToNSString(msg));
		}
	};
	static Logger logger;
	return logger;
}

DKString DKApplicationImpl::EnvironmentPath(SystemPath s)
{
	auto SearchPath = [](NSSearchPathDirectory path) -> DKString
	{
		NSFileManager* fileManager = [[[NSFileManager alloc] init] autorelease];
		NSURL* url = [fileManager URLForDirectory:path inDomain:NSUserDomainMask appropriateForURL:nil create:YES error:nil];
		return NSStringToIGString(url.path);
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
		return NSStringToIGString(path);
	};

	DKString path;
	switch (s)
	{
		case SystemPath::SystemPathSystemRoot:
			path = L"/";
			break;
		case SystemPath::SystemPathAppRoot:
			path = NSStringToIGString( [[NSBundle mainBundle] bundlePath] );
			break;
		case SystemPath::SystemPathAppResource:
			path = NSStringToIGString( [[NSBundle mainBundle] resourcePath] );
			break;
		case SystemPath::SystemPathAppExecutable:
			path = NSStringToIGString( [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent] );
			break;
		case SystemPath::SystemPathAppData:
			path = SearchPath(NSApplicationSupportDirectory);
			break;
		case SystemPath::SystemPathUserHome:
			path = NSStringToIGString(NSHomeDirectory());
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
			path = NSStringToIGString(NSTemporaryDirectory());
			break;
	}
	return path;
}

DKString DKApplicationImpl::ModulePath(void)
{
	return NSStringToIGString( [[NSBundle mainBundle] executablePath] );
}

DKObject<DKData> DKApplicationImpl::LoadResource(const DKString& res, DKAllocator& alloc)
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

DKObject<DKData> DKApplicationImpl::LoadStaticResource(const DKString& res)
{
	DKObject<DKData> ret = NULL;
	DKObject<DKDirectory> dir = DKDirectory::OpenDir(EnvironmentPath(SystemPath::SystemPathAppResource));
	if (dir)
	{
		ret = dir->MapFile(res, 0, false);
	}
	return ret;
}

DKRect DKApplicationImpl::DisplayBounds(int displayId) const
{
	if (displayId <= 0)
	{
		NSRect screenBounds = [[NSScreen mainScreen] frame];
		return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
	}
	return DKRect(0,0,0,0);
}

DKRect DKApplicationImpl::ScreenContentBounds(int displayId) const
{
	if (displayId <= 0)
	{
		NSRect screenBounds = [[NSScreen mainScreen] visibleFrame];
		return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
	}
	return DKRect(0,0,0,0);
}

DKString DKApplicationImpl::HostName(void) const
{
	return NSStringToIGString([[NSProcessInfo processInfo] hostName]);		
}

DKString DKApplicationImpl::OSName(void) const
{
	return NSStringToIGString([[NSProcessInfo processInfo] operatingSystemName]) + L" (Mac OS X) " + NSStringToIGString([[NSProcessInfo processInfo] operatingSystemVersionString]);
}

DKString DKApplicationImpl::UserName(void) const
{
	return NSStringToIGString(NSUserName());
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
