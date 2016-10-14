//
//  File: DKWindow_iOS.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKWindow for iOS

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include "DKWindow_iOS.h"
#include "DKApplication_iOS.h"

using namespace DKGL;
using namespace DKGL::Private;


////////////////////////////////////////////////////////////////////////////////
// DKWindow_iOS implementation
#pragma mark - DKWindow_iOS implementation
DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
{
	return new DKWindow_iOS(win);
}

DKWindow_iOS::DKWindow_iOS(DKWindow *window)
: ownerWindow(window)
, view(nil)
{
}

DKWindow_iOS::~DKWindow_iOS(void)
{
	DKASSERT_DEBUG(view == nil);
}

bool DKWindow_iOS::CreateProxy(void* systemHandle)
{
	DKLog("A Proxy Window is not supported in DK version of iOS.\n");
	return false;
}

void DKWindow_iOS::UpdateProxy(void)
{
}

bool DKWindow_iOS::IsProxy(void) const
{
	return false;
}

bool DKWindow_iOS::Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style)
{
	DKASSERT_DEBUG(this->view == nil);

	CGRect rect = CGRectMake(origin.x, origin.y, size.width, size.height);
	if (rect.size.width < 1 || rect.size.height < 1)
	{
		CGRect screenBounds = [[UIScreen mainScreen] bounds];

		UIApplication* app = [UIApplication sharedApplication];
		if ([app.delegate isKindOfClass:[DKApplicationDelegate class]])
			screenBounds = ((DKApplicationDelegate*)app.delegate).viewController.view.bounds;
		else
			screenBounds = [[UIScreen mainScreen] applicationFrame];

		rect.size = screenBounds.size;
	}
	if (origin.x + rect.size.width < 0 || origin.y + rect.size.height < 0)
	{
		rect.origin = CGPointMake(0, 0);
	}


	DKWindowView_iOS* targetView = [[DKWindowView_iOS alloc] initWithFrame:rect handler:ownerWindow];

	// Attach view to key-window's subview if running application instance
	// is DKApplication. Otherwise, You need to handle this view by calling
	// PlatformHandle() outside.
	if (DKApplication::Instance() &&
		[[UIApplication sharedApplication].delegate isKindOfClass:[DKApplicationDelegate class]])
	{
		DKApplicationDelegate* appDelegate = (DKApplicationDelegate*)[UIApplication sharedApplication].delegate;
		[appDelegate.viewController.view addSubview:targetView];
		[appDelegate.viewController.view bringSubviewToFront:targetView];
		targetView.frame = rect;
		targetView.hidden = YES;
	}
	
	if (style & DKWindow::StyleResizableBorder)
	{
		targetView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	}
	else
	{
		targetView.autoresizingMask = UIViewAutoresizingNone;
	}

	// window created.
	ownerWindow->PostWindowEvent(DKWindow::EventWindowCreated, targetView.contentSize, targetView.origin, false);

	view = targetView;
	
	return true;
}

void DKWindow_iOS::Destroy(void)
{
	if (view)
	{
		ownerWindow->PostWindowEvent(DKWindow::EventWindowClosed, view.contentSize, view.origin, false);
		
		[view removeFromSuperview];
		[view release];
	}
	view = nil;
}

void DKWindow_iOS::Show(void)
{
	view.hidden = NO;
}

void DKWindow_iOS::Hide(void)
{
	view.hidden = YES;
}

void DKWindow_iOS::Activate(void)
{
	view.hidden = NO;
	[view becomeFirstResponder];
}

void DKWindow_iOS::Minimize(void)
{
}

void DKWindow_iOS::Resize(const DKSize& s, const DKPoint* pt)
{
	CGRect frame = view.frame;
	frame.size = CGSizeMake(s.width, s.height);
	if (pt)
		frame.origin = CGPointMake(pt->x, pt->y);

	view.frame = frame;
}

void DKWindow_iOS::SetOrigin(const DKPoint& pt)
{
	CGRect frame = view.frame;
	frame.origin = CGPointMake(pt.x, pt.y);
	view.frame = frame;
}

DKSize DKWindow_iOS::ContentSize(void) const
{
	return view.contentSize;
}

DKPoint DKWindow_iOS::Origin(void) const
{
	return view.origin;
}

double DKWindow_iOS::ContentScaleFactor(void) const
{
	return view.contentScaleFactor;
}

void DKWindow_iOS::SetTitle(const DKString& title)
{
}

DKString DKWindow_iOS::Title(void) const
{
	return L"";
}

bool DKWindow_iOS::IsVisible(void) const
{
	return [view isHidden];
}

void* DKWindow_iOS::PlatformHandle(void) const
{
	return view;
}

void DKWindow_iOS::ShowMouse(int deviceId, bool bShow)
{
}

bool DKWindow_iOS::IsMouseVisible(int deviceId) const
{
	return false;
}

void DKWindow_iOS::HoldMouse(int deviceId, bool bHold)
{
}

bool DKWindow_iOS::IsMouseHeld(int deviceId) const
{
	return false;
}

void DKWindow_iOS::SetMousePosition(int deviceId, const DKPoint& pt)
{
	[view setTouchPosition:pt atIndex:deviceId];
}

DKPoint DKWindow_iOS::MousePosition(int deviceId) const
{
	return [view touchPositionAtIndex:deviceId];
}

void DKWindow_iOS::EnableTextInput(int deviceId, bool enable)
{
	if (deviceId == 0)
		[view enableTextInput:enable];
}

bool DKWindow_iOS::IsValid(void) const
{
	if (view)
		return true;
	
	return false;			
}

DKVirtualKey DKWindow_iOS::ConvertVKey(unsigned long key)
{
	switch (key)
	{
		case 0x00:	return DKVK_A;				// a
		case 0x01:	return DKVK_S;				// s
		case 0x02:	return DKVK_D;				// d
		case 0x03:	return DKVK_F;				// f
		case 0x04:	return DKVK_H;				// h
		case 0x05:	return DKVK_G;				// g
		case 0x06:	return DKVK_Z;				// z
		case 0x07:	return DKVK_X;				// x
		case 0x08:	return DKVK_C;				// c
		case 0x09:	return DKVK_V;				// v
		case 0x0A:	return DKVK_NONE;
		case 0x0B:	return DKVK_B;				// b
		case 0x0C:	return DKVK_Q;				// q
		case 0x0D:	return DKVK_W;				// w
		case 0x0E:	return DKVK_E;				// e
		case 0x0F:	return DKVK_R;				// r
		case 0x10:	return DKVK_Y;				// y
		case 0x11:	return DKVK_T;				// t
		case 0x12:	return DKVK_1;				// 1
		case 0x13:	return DKVK_2;				// 2
		case 0x14:	return DKVK_3;				// 3
		case 0x15:	return DKVK_4;				// 4
		case 0x16:	return DKVK_6;				// 6
		case 0x17:	return DKVK_5;				// 5
		case 0x18:	return DKVK_EQUAL;			// =
		case 0x19:	return DKVK_9;				// 9
		case 0x1A:	return DKVK_7;				// 7
		case 0x1B:	return DKVK_HYPHEN;			// -
		case 0x1C:	return DKVK_8;				// 8
		case 0x1D:	return DKVK_0;				// 0
		case 0x1E:	return DKVK_CLOSE_BRACKET;	// ]
		case 0x1F:	return DKVK_O;				// o
		case 0x20:	return DKVK_U;				// u
		case 0x21:	return DKVK_OPEN_BRACKET;	// [
		case 0x22:	return DKVK_I;				// i
		case 0x23:	return DKVK_P;				// p
		case 0x24:	return DKVK_RETURN;			// return
		case 0x25:	return DKVK_L;				// l
		case 0x26:	return DKVK_J;				// j
		case 0x27:	return DKVK_QUOTE;			// '
		case 0x28:	return DKVK_K;				// k
		case 0x29:	return DKVK_SEMICOLON;		// ;
		case 0x2A:	return DKVK_BACKSLASH;		// backslash
		case 0x2B:	return DKVK_COMMA;			// ,
		case 0x2C:	return DKVK_SLASH;			// /
		case 0x2D:	return DKVK_N;				// n
		case 0x2E:	return DKVK_M;				// m
		case 0x2F:	return DKVK_PERIOD;			// .
		case 0x30:	return DKVK_TAB;			// tab
		case 0x31:	return DKVK_SPACE;			// space
		case 0x32:	return DKVK_ACCENT_TILDE;	// ` (~)
		case 0x33:	return DKVK_BACKSPACE;		// delete (backspace)
		case 0x34:	return DKVK_NONE;			//
		case 0x35:	return DKVK_ESCAPE;			// esc
		case 0x36:	return DKVK_NONE;			
		case 0x37:	return DKVK_NONE;			
			//case 0x38:	return DKVK_SHIFT;			// l-shift	
			//case 0x39:	return DKVK_NONE;			
			//case 0x3A:	return DKVK_OPTION;			// l-alt			
			//case 0x3B:	return DKVK_CONTROL;		// l_ctrl			
			//case 0x3C:	return DKVK_R_SHIFT;		// r-shift	
			//case 0x3D:	return DKVK_R_ALT;			// r-alt
			//case 0x3E:	return DKVK_R_CTRL;			// r-ctrl
		case 0x3F:	return DKVK_NONE;			
		case 0x40:	return DKVK_F17;			// f17
		case 0x41:	return DKVK_PAD_PERIOD;		// . (keypad)
		case 0x42:	return DKVK_NONE;			
		case 0x43:	return DKVK_PAD_ASTERISK;	// * (keypad)
		case 0x44:	return DKVK_NONE;			
		case 0x45:	return DKVK_PAD_PLUS;		// + (keypad)
		case 0x46:	return DKVK_NONE;			
		case 0x47:	return DKVK_NUMLOCK;		// clear (keypad)
		case 0x48:	return DKVK_NONE;			
		case 0x49:	return DKVK_NONE;			
		case 0x4A:	return DKVK_NONE;			
		case 0x4B:	return DKVK_PAD_SLASH;		// / (keypad)
		case 0x4C:	return DKVK_ENTER;			// enter (keypad)
		case 0x4D:	return DKVK_NONE;			
		case 0x4E:	return DKVK_PAD_MINUS;		// - (keypad)
		case 0x4F:	return DKVK_F18;			// f18
		case 0x50:	return DKVK_F19;			// f19
		case 0x51:	return DKVK_PAD_EQUAL;		// = (keypad)
		case 0x52:	return DKVK_PAD_0;			// 0 (keypad)
		case 0x53:	return DKVK_PAD_1;			// 1 (keypad)
		case 0x54:	return DKVK_PAD_2;			// 2 (keypad)
		case 0x55:	return DKVK_PAD_3;			// 3 (keypad)
		case 0x56:	return DKVK_PAD_4;			// 4 (keypad)
		case 0x57:	return DKVK_PAD_5;			// 5 (keypad)
		case 0x58:	return DKVK_PAD_6;			// 6 (keypad)
		case 0x59:	return DKVK_PAD_7;			// 7 (keypad)
		case 0x5A:	return DKVK_NONE;			
		case 0x5B:	return DKVK_PAD_8;			// 8 (keypad)
		case 0x5C:	return DKVK_PAD_9;			// 9 (keypad)
		case 0x5D:	return DKVK_NONE;			
		case 0x5E:	return DKVK_NONE;			
		case 0x5F:	return DKVK_NONE;			
		case 0x60:	return DKVK_F5;				// f5
		case 0x61:	return DKVK_F6;				// f6
		case 0x62:	return DKVK_F7;				// f7
		case 0x63:	return DKVK_F3;				// f3
		case 0x64:	return DKVK_F8;				// f8
		case 0x65:	return DKVK_NONE;			
		case 0x66:	return DKVK_NONE;			
		case 0x67:	return DKVK_NONE;			
		case 0x68:	return DKVK_NONE;			
		case 0x69:	return DKVK_F13;			// f13
		case 0x6A:	return DKVK_F16;			// f16
		case 0x6B:	return DKVK_F14;			// f14
		case 0x6C:	return DKVK_NONE;			
		case 0x6D:	return DKVK_NONE;			
		case 0x6E:	return DKVK_NONE;			
		case 0x6F:	return DKVK_NONE;			
		case 0x70:	return DKVK_NONE;			
		case 0x71:	return DKVK_F15;			// f15
		case 0x72:	return DKVK_NONE;			
		case 0x73:	return DKVK_HOME;			// home
		case 0x74:	return DKVK_PAGE_UP;		// page up
		case 0x75:	return DKVK_DELETE;			// delete (below insert key)
		case 0x76:	return DKVK_F4;				// f4
		case 0x77:	return DKVK_END;			// end
		case 0x78:	return DKVK_F2;				// f2
		case 0x79:	return DKVK_PAGE_DOWN;		// page down
		case 0x7A:	return DKVK_F1;				// f1
		case 0x7B:	return DKVK_LEFT;			// left
		case 0x7C:	return DKVK_RIGHT;			// right
		case 0x7D:	return DKVK_DOWN;			// down
		case 0x7E:	return DKVK_UP;				// up
		case 0x7F:	return DKVK_NONE;
		default:	return DKVK_NONE;
	}
	
	return DKVK_NONE;
}

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)

