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

namespace DKFramework
{
	namespace Private
	{
		namespace iOS
		{
			class Window : public DKWindowInterface
			{
			public:
				Window(DKWindow*);
				~Window(void);

				DKRect ContentRect(void);
				DKRect WindowRect(void);
				
				// DKWindowInterface override
				bool Create(const DKString& title, uint32_t style);
				bool CreateProxy(void* systemHandle);
				bool IsProxy(void) const;
				void UpdateProxy(void);
				void Destroy(void);
				void* PlatformHandle(void) const;
				bool IsValid(void) const;

				void ShowMouse(int deviceId, bool show);
				bool IsMouseVisible(int deviceId) const;
				void HoldMouse(int deviceId, bool hold);
				bool IsMouseHeld(int deviceId) const;

				void Show(void);
				void Hide(void);
				void Activate(void);
				void Minimize(void);

				void SetOrigin(DKPoint);
				void Resize(DKSize, const DKPoint* optionalOrigin);
				double ContentScaleFactor(void) const;  // logical coords by pixel ratio.

				void SetTitle(const DKString& title);
				DKString Title(void) const;

				void SetMousePosition(int deviceId, DKPoint pt);
				DKPoint MousePosition(int deviceId) const;

				void EnableTextInput(int deviceId, bool enable);
				bool IsTextInputEnabled(int deviceId);

			private:
				DKWindow* instance;
				UIWindow* window;	// view holder. use view.window instead.
				UIView* view;
			};
		}
	}
}
#endif //if TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
