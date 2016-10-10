//
//  File: DKApplicationImpl.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKApplication for iOS

#import <UIKit/UIKit.h>
#import "../../DKApplication.h"
#import "../../DKLinearTransform3.h"
#import "../../DKOpenALContext.h"
#import "DKApplicationImpl.h"

using namespace DKGL;
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


#pragma mark - DKApplicationViewController
@interface DKApplicationViewController : UIViewController
{
	UIView* targetView; // view instance, to recover display view which removed by low-memory situation.
}
@end

@implementation DKApplicationViewController
- (id)init
{
	self = [super init];
	if (self)
	{
		targetView = [[UIView alloc] initWithFrame:[[UIScreen mainScreen] applicationFrame]];		
		targetView.autoresizesSubviews = YES;
		targetView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		targetView.contentMode = UIViewContentModeScaleToFill;
		targetView.backgroundColor = [UIColor clearColor];
	}
	return self;
}

- (void)dealloc
{
	self.view = nil;
	[targetView release];
	[super dealloc];
}

- (void)loadView
{
	self.view = targetView;
}

- (BOOL)shouldAutorotate
{
	return YES;
}

- (NSUInteger)supportedInterfaceOrientations
{
	return UIInterfaceOrientationMaskAll;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	NSDictionary* dic = [[NSBundle mainBundle] infoDictionary];
	
	NSArray* supportedInterfaceOrientations = [dic objectForKey:@"UISupportedInterfaceOrientations"];
	if (supportedInterfaceOrientations)
	{
		switch (interfaceOrientation)
		{
			case UIInterfaceOrientationPortrait:
				return [supportedInterfaceOrientations containsObject:@"UIInterfaceOrientationPortrait"];
			case UIInterfaceOrientationPortraitUpsideDown:
				return [supportedInterfaceOrientations containsObject:@"UIInterfaceOrientationPortraitUpsideDown"];
			case UIInterfaceOrientationLandscapeLeft:
				return [supportedInterfaceOrientations containsObject:@"UIInterfaceOrientationLandscapeLeft"];
			case UIInterfaceOrientationLandscapeRight:
				return [supportedInterfaceOrientations containsObject:@"UIInterfaceOrientationLandscapeRight"];
		}		
	}
	return NO;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	[self.view layoutIfNeeded];
}

@end


////////////////////////////////////////////////////////////////////////////////
// DKApplicationDelegate
#pragma mark - DKApplicationDelegate implementation
@implementation DKApplicationDelegate
@synthesize viewController;

- (void)initialize
{
	[[AVAudioSession sharedInstance] setDelegate:self];
	[[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategorySoloAmbient error:nil];
		
    window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];  
    window.backgroundColor = [UIColor blackColor];
	
	viewController = [[DKApplicationViewController alloc] init];
	
	if ([window respondsToSelector:@selector(rootViewController)])
		window.rootViewController = viewController;
	else
		[window addSubview:viewController.view];	
	
	initialized = YES;

	((DKApplicationImpl*)DKApplicationInterface::SharedInstance())->AppInitialize();
	
	if (initialized)
		[window makeKeyAndVisible];  	
}

- (void)finalize
{
	if (initialized == NO)
		return;

	((DKApplicationImpl*)DKApplicationInterface::SharedInstance())->AppFinalize();

	[window release];
	window = nil;
	[viewController release];
	viewController = nil;
	
	initialized = NO;
}

- (void)terminate:(NSNumber*)exitCode
{
	[self finalize];
	int code = [exitCode intValue];
	DKLog("Application terminated.(%x)\n", code);
	exit(code);
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	[self initialize];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	[self finalize];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	DKLog("Application activated.\n");
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	DKLog("Application become hidden.\n");	
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
	NSTimeInterval remains = [application backgroundTimeRemaining];
	DKLog("Application did enter background. (will terminated after %e seconds)\n", remains);
	
	// delays method returns, to earn time for other threads can stop working.
	// especially OpenGL ES drawing. If app has gone to background, accessing
	// OpenGL ES will terminate app immediately.
	DKThread::Sleep(1.0);
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
	DKLog("Application will enter foreground.\n");
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	NSLog(@"\n"
		  @"################################################################################\n"
		  @"#####                         Memory Warning!!                              ####\n"
		  @"################################################################################\n");
}

#pragma mark AVAudioSession delegate
- (void)beginInterruption
{
	DKOpenALContext::Deactivate();
	[[AVAudioSession sharedInstance] setActive:NO error:nil];
	DKLog("Audio Session begin interruption.\n");
}

- (void)endInterruption
{
	DKTimer timer;
	timer.Reset();

	while ([[AVAudioSession sharedInstance] setActive:YES error:nil] == NO)
	{
		if (timer.Elapsed() > 3.0)
		{
			DKLog("FATAL ERROR: Failed to active audio session!!\n");
			break;
		}
	}
	
	DKLog("Audio Session end interruption.\n");		
	DKOpenALContext::Activate();
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
, terminateRequested(false)
{
}

DKApplicationImpl::~DKApplicationImpl(void)
{
}

int DKApplicationImpl::Run(DKArray<char*>& args)
{
	this->terminateRequested = false;
	return UIApplicationMain((int)args.Count(), (char**)args, @"UIApplication", @"DKApplicationDelegate");
}

void DKApplicationImpl::Terminate(int exitCode)
{
	if (this->terminateRequested)
		return;

	this->terminateRequested = true;
	DKLog("DKApplication Terminate(%x) requested.\n", exitCode);

	UIApplication* app = [UIApplication sharedApplication];

	if ([app.delegate isKindOfClass:[DKApplicationDelegate class]])
	{
		DKApplicationDelegate* appDelegate = (DKApplicationDelegate*)[UIApplication sharedApplication].delegate;
		[appDelegate performSelectorOnMainThread:@selector(terminate:) withObject:[NSNumber numberWithInt:exitCode] waitUntilDone:NO];
	}
	else
	{
		exit(exitCode);
	}
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

void DKApplicationImpl::AppInitialize(void)
{
	DKApplicationInterface::AppInitialize(mainApp);
}

void DKApplicationImpl::AppFinalize(void)
{
	DKApplicationInterface::AppFinalize(mainApp);
}

DKLogger& DKApplicationImpl::DefaultLogger(void)
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

DKString DKApplicationImpl::EnvironmentPath(SystemPath s)
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
		if ([fileManager fileExistsAtPath:path isDirectory:&isDir] == NO)	// file not exists
		{
			if ([fileManager createDirectoryAtPath:path withIntermediateDirectories:YES attributes:nil error:nil] == NO)
			{
				NSString* errDesc = [NSString stringWithFormat:@"Cannot create directory:%@", path];
				DKERROR_THROW((const char*)[errDesc UTF8String]);
				NSLog(@"%@\n", errDesc);
			}
		}
		else if (!isDir)	// file is exists, but not dir
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

DKString DKApplicationImpl::ModulePath(void)
{
	return [[[NSBundle mainBundle] executablePath] UTF8String];
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
		CGRect screenBounds = [[UIScreen mainScreen] bounds];
		return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
	}
	return DKRect(0,0,0,0);
}

DKRect DKApplicationImpl::ScreenContentBounds(int displayId) const
{
	if (displayId <= 0)
	{
		UIApplication* app = [UIApplication sharedApplication];
		if ([app.delegate isKindOfClass:[DKApplicationDelegate class]])
		{
			CGRect bounds = ((DKApplicationDelegate*)app.delegate).viewController.view.bounds;
			return DKRect(bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
		}
		else
		{
			CGRect screenBounds = [[UIScreen mainScreen] applicationFrame];
			return DKRect(screenBounds.origin.x, screenBounds.origin.y, screenBounds.size.width, screenBounds.size.height);
		}
	}
	return DKRect(0,0,0,0);
}

DKString DKApplicationImpl::HostName(void) const
{
	return [[[UIDevice currentDevice] name] UTF8String];
}

DKString DKApplicationImpl::OSName(void) const
{
	NSString* systemName = [[UIDevice currentDevice] systemName];
	NSString* systemVersion = [[UIDevice currentDevice] systemVersion];

	return [[NSString stringWithFormat:@"%@ (Version:%@)", systemName, systemVersion] UTF8String];
}

DKString DKApplicationImpl::UserName(void) const
{
	return [NSUserName() UTF8String];
}

#endif // if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
