//
//  File: DKApplication_iOS.h
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE

#ifdef __OBJC__
#import <UIKit/UIKit.h> 

@interface DKApplicationDelegate : NSObject<UIApplicationDelegate>
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
		class DKApplication_iOS : public DKApplicationInterface
		{
		public:
			DKApplication_iOS(DKApplication* app);
			~DKApplication_iOS(void);
			
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

