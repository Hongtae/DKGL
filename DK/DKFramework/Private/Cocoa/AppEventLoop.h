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

namespace DKFramework::Private::macOS
{
    class AppEventLoop : public DKApplication::EventLoop
    {
    public:
        AppEventLoop(DKApplication* app);
        ~AppEventLoop();

        bool Run() override;
        void Stop() override;

        DKObject<DKDispatchQueue::ExecutionState> Submit(DKOperation* operation, double delay) override;

        bool IsRunning() const override;
        bool IsDispatchThread() const override;

    private:
        void DispatchAndInstallTimer();

        DKApplication* appInstance;
        DKThread::ThreadId threadId;
        bool running;
        NSTimer* timer;

        DKSpinLock lock;	// lock for runLoop pointer.
        CFRunLoopRef runLoop;
    };
}
#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
