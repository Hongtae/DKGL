//
//  File: Window.h
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

#include "../../Interface/DKWindowInterface.h"

namespace DKFramework::Private::iOS
{
    class Window : public DKWindowInterface
    {
    public:
        Window(DKWindow*);
        ~Window();

        DKRect ContentRect();
        DKRect WindowRect();

        // DKWindowInterface override
        bool Create(const DKString& title, uint32_t style) override;
        bool CreateProxy(void* systemHandle) override;
        bool IsProxy() const override;
        void UpdateProxy() override;
        void Destroy() override;
        void* PlatformHandle() const override;
        bool IsValid() const override;

        void ShowMouse(int deviceId, bool show) override;
        bool IsMouseVisible(int deviceId) const override;
        void HoldMouse(int deviceId, bool hold) override;
        bool IsMouseHeld(int deviceId) const override;

        void Show() override;
        void Hide() override;
        void Activate() override;
        void Minimize() override;

        void SetOrigin(DKPoint) override;
        void Resize(DKSize, const DKPoint* optionalOrigin) override;
        double ContentScaleFactor() const override;  // logical coords by pixel ratio.

        void SetTitle(const DKString& title) override;
        DKString Title() const override;

        void SetMousePosition(int deviceId, DKPoint pt) override;
        DKPoint MousePosition(int deviceId) const override;

        void EnableTextInput(int deviceId, bool enable) override;
        bool IsTextInputEnabled(int deviceId) const override;

    private:
        DKWindow* instance;
        UIWindow* window;	// view holder. use view.window instead.
        UIView* view;
    };
}
#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
