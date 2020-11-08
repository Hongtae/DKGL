//
//  File: Window.h
//  Platform: Win32
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#ifdef _WIN32
#include <Windows.h>
#include "../../Interface/DKWindowInterface.h"

namespace DKFramework::Private::Win32
{
    class Window : public DKWindowInterface
    {
    public:
        Window(DKWindow*);
        ~Window();

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
        float ContentScaleFactor() const override;  // logical coords by pixel ratio.

        void SetTitle(const DKString& title) override;
        DKString Title() const override;

        void SetMousePosition(int deviceId, DKPoint pt) override;
        DKPoint MousePosition(int deviceId) const override;

        void EnableTextInput(int deviceId, bool enable) override;
        bool IsTextInputEnabled(int deviceId) const override;

    private:
        static LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        static DKVirtualKey ConvertVKey(int key);
        void UpdateKeyboard();
        void UpdateMouse();
        void ResetKeyboard();
        void ResetMouse();

        DKWindow* instance;
        IDropTarget* dropTarget;
        HWND hWnd;

        BYTE keyboardStates[256];
        DKRect windowRect;
        DKRect contentRect;
        float contentScaleFactor;
        
        DKPoint mousePosition;
        DKPoint holdingMousePosition;

        bool textCompositionMode;
        bool proxyWindow;
        bool activated;
        bool visible;
        bool minimized;
        bool holdMouse;
        bool resizing;
        bool autoResize;

        union {
            uint8_t buttons;
            struct {
                uint8_t button1 : 1;
                uint8_t button2 : 1;
                uint8_t button3 : 1;
                uint8_t button4 : 1;
                uint8_t button5 : 1;
                uint8_t button6 : 1;
                uint8_t button7 : 1;
                uint8_t button8 : 1;
            };
        } mouseButtonDown;
    };
}
#endif // _WIN32
