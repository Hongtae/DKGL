//
//  File: DKWindowImpl.h
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
#import "DKWindowView.h"
#else
class NSWindow;
class DKWindowView;
#endif	//ifdef __OBJC__

#include "../../Interface/DKWindowInterface.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		class DKWindowImpl : public DKWindowInterface
		{
		public:
			DKWindowImpl(DKWindow *window);
			~DKWindowImpl(void);
			
			bool CreateProxy(void* systemHandle);
			void UpdateProxy(void);
			bool IsProxy(void) const;
			
			bool Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style);
			void Destroy(void);

			void Show(void);
			void Hide(void);
			void Activate(void);
			void Minimize(void);
			void Resize(const DKSize&, const DKPoint*);
			void SetOrigin(const DKPoint&);

			DKSize ContentSize(void) const;
			DKPoint Origin(void) const;
			double ContentScaleFactor(void) const;

			void SetTitle(const DKString& title);
			DKString Title(void) const;

			bool IsVisible(void) const;
			
			void* PlatformHandle(void) const;
			
			void ShowMouse(int deviceId, bool bShow);
			bool IsMouseVisible(int deviceId) const;
			void HoldMouse(int deviceId, bool bHold);
			bool IsMouseHeld(int deviceId) const;
			void SetMousePosition(int deviceId, const DKPoint& pt);
			DKPoint MousePosition(int deviceId) const;
			
			void EnableTextInput(int deviceId, bool bTextInput);
			
			bool IsValid(void) const;
			static DKVirtualKey ConvertVKey(unsigned long key);

		private:
			NSWindow* window;
			NSView* view;
			DKWindow* ownerWindow;
			bool proxyWindow;
		};
	}
}

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
