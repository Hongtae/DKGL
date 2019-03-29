//
//  File: Application.h
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import <AppKit/AppKit.h>

#include "../../Interface/DKApplicationInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace macOS
		{
			class AppEventLoop : public DKEventLoop
			{
			public:
				AppEventLoop(DKApplication* app);
				~AppEventLoop();

				bool Run() override;
				void Stop() override;

				DKObject<PendingState> Post(const DKOperation* operation, double delay) override;
				DKObject<PendingState> Post(const DKOperation* operation, const DKDateTime& runAfter) override;

			private:
				void DispatchAndInstallTimer();
				
				DKApplication* appInstance;
				bool running;
				NSTimer* timer;

				DKSpinLock lock;	// lock for runLoop pointer.
				CFRunLoopRef runLoop;
			};
		}
	}
}
#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
