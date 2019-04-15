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
#import <UIKit/UIKit.h>

#include "../../Interface/DKApplicationInterface.h"

namespace DKFramework::Private::iOS
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
        NSTimer* timer;

        DKSpinLock lock;	// lock for runLoop pointer.
        CFRunLoopRef runLoop;
    };
}
#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
