//
//  File: DKApplicationImpl.h
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE

#ifdef __OBJC__
#import <UIKit/UIKit.h> 
#import <AVFoundation/AVFoundation.h>

@interface DKApplicationDelegate : NSObject<UIApplicationDelegate, AVAudioSessionDelegate>
{
	UIWindow*					window;
	UIViewController*			viewController;
	BOOL						initialized;
}

@property (readonly) UIViewController* viewController;
@end
#else

#endif	//ifdef __OBJC__

#include "../../Interface/DKApplicationInterface.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		class DKApplicationImpl : public DKApplicationInterface
		{
		public:
			DKApplicationImpl(DKApplication* app);
			~DKApplicationImpl(void);
			
			int Run(DKArray<char*>& args);
			void Terminate(int exitCode);

			void PerformOperationOnMainThread(DKOperation* op, bool waitUntilDone);

			DKLogger& DefaultLogger(void);
			DKString EnvironmentPath(SystemPath);
			DKString ModulePath(void);

			DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc);
			DKObject<DKData> LoadStaticResource(const DKString& res);

			DKRect DisplayBounds(int displayId) const;
			DKRect ScreenContentBounds(int displayId) const;

			DKString HostName(void) const;
			DKString OSName(void) const;
			DKString UserName(void) const;

			void AppInitialize(void);
			void AppFinalize(void);

			DKApplication* mainApp;
			bool terminateRequested;
		};
	}
}

#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)

