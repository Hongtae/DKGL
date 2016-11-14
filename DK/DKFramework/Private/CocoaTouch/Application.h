//
//  File: Application.h
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#endif	//ifdef __OBJC__

#include "AppEventLoop.h"
#include "../../Interface/DKApplicationInterface.h"

namespace DKFramework
{
	namespace Private
	{
		namespace iOS
		{
			class Application : public DKApplicationInterface
			{
			public:
				Application(DKApplication* app);
				~Application(void);

				DKEventLoop* EventLoop(void) override;
				DKLogger* DefaultLogger(void) override;

				DKString DefaultPath(SystemPath) override;
				DKString ProcessInfoString(ProcessInfo) override;

				DKObject<DKData> LoadResource(const DKString& res, DKAllocator& alloc) override;		// read-writable
				DKObject<DKData> LoadStaticResource(const DKString& res) override;	// read-only

				DKRect DisplayBounds(int displayId) const override;
				DKRect ScreenContentBounds(int displayId) const override;

			private:
				AppEventLoop mainLoop;
			};
		}
	}
}
#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
