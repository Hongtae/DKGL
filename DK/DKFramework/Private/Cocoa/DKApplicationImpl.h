//
//  File: DKApplicationImpl.h
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2014 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE

#ifdef __OBJC__
#import <AppKit/AppKit.h> 
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

			DKApplication* mainApp;
			int retCode;
			bool terminate;
		};
	}
}

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)

