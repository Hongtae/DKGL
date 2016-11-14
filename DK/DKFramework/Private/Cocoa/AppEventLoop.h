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
#ifdef __OBJC__
#import <AppKit/AppKit.h>
#endif	//ifdef __OBJC__

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
				~AppEventLoop(void);

				bool Run(void) override;
				void Stop(void) override;

				DKObject<PendingState> Post(const DKOperation* operation, double delay) override;
				DKObject<PendingState> Post(const DKOperation* operation, const DKDateTime& runAfter) override;

			private:
				DKApplication* appInstance;
			};
		}
	}
}
#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
