//
//  File: DKWindow_OSX.mm
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE
#warning Compiling DKWindow for Mac OS X

#import <AppKit/AppKit.h> 
#import "DKWindow_OSX.h"

using namespace DKFoundation;
using namespace DKFramework;
using namespace DKFramework::Private;


#pragma mark - DKNSWindow
@interface DKNSWindow : NSWindow
@end
@implementation DKNSWindow
- (BOOL)canBecomeKeyWindow
{
	return YES;
}
- (BOOL)canBecomeMainWindow
{
	return YES;
}
@end

#pragma mark - DKWindow_OSX implementation
////////////////////////////////////////////////////////////////////////////////
// DKWindow_OSX implementation
DKWindowInterface* DKWindowInterface::CreateInterface(DKWindow* win)
{
	return new DKWindow_OSX(win);
}

DKWindow_OSX::DKWindow_OSX(DKWindow *window)
: ownerWindow(window)
, window(nil)
, view(nil)
, proxyWindow(false)
{
	bool bMultiThread = [NSThread isMultiThreaded];
	bool bMainThread = [NSThread isMainThread];
	DKLog("NSThread MultiThread result: %d\n", bMultiThread);
	DKLog("NSThread MainThread result: %d\n", bMainThread);
}

DKWindow_OSX::~DKWindow_OSX(void)
{
	DKASSERT_DEBUG(this->window == nil);
	DKASSERT_DEBUG(this->view == nil);
}

bool DKWindow_OSX::CreateProxy(void* systemHandle)
{
	if (systemHandle == nil)
		return false;

	DKASSERT_DEBUG(this->window == nil);
	DKASSERT_DEBUG(this->view == nil);

	id obj = (id)systemHandle;
	if ([obj isKindOfClass: [NSWindow class]])
	{
		this->window = obj;
		this->view = [this->window contentView];
	}
	else if ([obj isKindOfClass: [NSView class]])
	{
		this->view = obj;
		this->window = [this->view window];
	}
	else
	{
		NSLog(@"Unknown value!\n");
		return false;
	}			
	if (this->view && this->window)
	{
		[this->window retain];
		[this->view retain];

		proxyWindow = true;
		
		NSSize contentSize = view.bounds.size;
		NSPoint windowOrigin = window.frame.origin;

		ownerWindow->PostWindowEvent(DKWindow::EventWindowCreated, DKSize(contentSize.width, contentSize.height), DKPoint(windowOrigin.x, windowOrigin.y), false);
		return true;
	}
	return false;
}

void DKWindow_OSX::UpdateProxy(void)
{
	if (proxyWindow)
	{
		DKASSERT_DEBUG(view != nil);
		DKASSERT_DEBUG(window != nil);

		NSRect bounds = [view bounds];
		NSRect frame = [window frame];

		ownerWindow->PostWindowEvent(DKWindow::EventWindowResized, DKSize(bounds.size.width, bounds.size.height), DKPoint(frame.origin.x, frame.origin.y), false);
	}
}

bool DKWindow_OSX::IsProxy(void) const
{
	return proxyWindow;
}

bool DKWindow_OSX::Create(const DKString& title, const DKSize& size, const DKPoint& origin, int style)
{
	NSRect screenBounds = [[NSScreen mainScreen] visibleFrame];

	NSRect rect = NSMakeRect(0, 0, size.width, size.height);

	if (rect.size.width < 1 || rect.size.height < 1)
	{
		rect.size.width = screenBounds.size.width;
		rect.size.height = screenBounds.size.height;
	}

	NSUInteger windowStyle = 0;	/* NSBorderlessWindowMask */
	if (style & DKWindow::StyleTitle)
		windowStyle |= NSTitledWindowMask;
	if (style & DKWindow::StyleCloseButton)
		windowStyle |= NSClosableWindowMask;
	if (style & DKWindow::StyleMinimizeButton)
		windowStyle |= NSMiniaturizableWindowMask;
	if (style & DKWindow::StyleMaximizeButton)
		(void)0;
	if (style & DKWindow::StyleResizableBorder)
		windowStyle |= NSResizableWindowMask;

	DKASSERT_DEBUG(this->window == nil);
	DKASSERT_DEBUG(this->view == nil);

	// create window
	this->window = [[DKNSWindow alloc] initWithContentRect:rect
												 styleMask:windowStyle
												   backing:NSBackingStoreBuffered
													 defer:NO];

	// create view
	this->view = [[DKWindowView_OSX alloc] initWithFrame:rect handler:ownerWindow];
	[this->window setContentView:view];
	[window setTitle:[NSString stringWithUTF8String:(const char*)DKStringU8(title)]];

	if (origin.x + rect.size.width > 0 && origin.y + rect.size.height > 0)
		[this->window setFrameOrigin:NSMakePoint(origin.x, origin.y)];
	else
	{
		float w = (screenBounds.size.width - rect.size.width) * 0.5f;
		float h = (screenBounds.size.height - rect.size.height) * 0.5f;
		NSPoint center = NSMakePoint(screenBounds.origin.x + w, screenBounds.origin.y + h);
		[this->window setFrameOrigin:center];
	}

	proxyWindow = false;

	// set delegate to view
	[window setDelegate:(id<NSWindowDelegate>)view];
	[window makeFirstResponder:view];
	
	// enable mouse event
	[window setAcceptsMouseMovedEvents:YES];
	// do not release when closed.
	[window setReleasedWhenClosed:NO];


	if (style & DKWindow::StyleAcceptFileDrop)
	{
		[this->view registerForDraggedTypes:@[NSFilenamesPboardType]];
	}

	// window created.
	ownerWindow->PostWindowEvent(DKWindow::EventWindowCreated, this->ContentSize(), this->Origin(), false);

	[window resignKeyWindow];
	[window orderOut:nil];		// hide window

	return true;
}

void DKWindow_OSX::Destroy(void)
{
	if (window)
	{
		// a proxy window does not close window.
		// but the screen may be dirty, update display.
		if (proxyWindow)
		{
			// 2009-08-14 : display will invoked if necessary.
			// [window display];

		}
		else
		{
			[window close];
		}
		[view release];
		[window release];
	}
	window = nil;
	view = nil;
}

void DKWindow_OSX::Show(void)
{
	DKASSERT_DEBUG(window != nil);

	[window orderFront: nil];

	ownerWindow->PostWindowEvent(DKWindow::EventWindowShown, this->ContentSize(), this->Origin(), false);
}

void DKWindow_OSX::Hide(void)
{
	DKASSERT_DEBUG(window != nil);

	[window resignKeyWindow];
	[window orderOut: nil];
	
	ownerWindow->PostWindowEvent(DKWindow::EventWindowHidden, this->ContentSize(), this->Origin(), false);
}

void DKWindow_OSX::Activate(void)
{
	DKASSERT_DEBUG(window != nil);

	[window makeKeyAndOrderFront: nil];

	DKSize contentSize = this->ContentSize();
	DKPoint origin = this->Origin();

	ownerWindow->PostWindowEvent(DKWindow::EventWindowShown, contentSize, origin, false);
	ownerWindow->PostWindowEvent(DKWindow::EventWindowActivated, contentSize, origin, false);
}

void DKWindow_OSX::Minimize(void)
{
	DKASSERT_DEBUG(window != nil);

	[window miniaturize: nil];
}

void DKWindow_OSX::Resize(const DKSize& s, const DKPoint* pt)
{
	DKASSERT_DEBUG(window != nil);

	if (pt)
	{
		NSRect rect = [window frameRectForContentRect:NSMakeRect(pt->x, pt->y, s.width, s.height)];
		[window setFrame:rect display:YES];
	}
	else
	{
		NSSize size = {s.width, s.height};
		[window setContentSize:size];
	}
}

void DKWindow_OSX::SetOrigin(const DKPoint& origin)
{
	[this->window setFrameOrigin:NSMakePoint(origin.x, origin.y)];
}

DKSize DKWindow_OSX::ContentSize(void) const
{
	DKASSERT_DEBUG(view != nil);
	DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]]);

	return [(DKWindowView_OSX*)view contentSize];
}

DKPoint DKWindow_OSX::Origin(void) const
{
	DKASSERT_DEBUG(window != nil);

	NSRect rect = window.frame;
	return DKPoint(rect.origin.x, rect.origin.y);
}

double DKWindow_OSX::ContentScaleFactor(void) const
{
	return window.backingScaleFactor;
}

void DKWindow_OSX::SetTitle(const DKString& title)
{
	[window setTitle:[NSString stringWithUTF8String:(const char*)DKStringU8(title)]];
}

DKString DKWindow_OSX::Title(void) const
{
	return [[window title] UTF8String];
}

bool DKWindow_OSX::IsVisible(void) const
{
	return [window isVisible];
}

void* DKWindow_OSX::PlatformHandle(void) const
{
	return view;
}

void DKWindow_OSX::ShowMouse(int deviceId, bool bShow)
{
	if (deviceId != 0)
		return;
	
	if (bShow)
		[NSCursor unhide];
	else
		[NSCursor hide];
}

bool DKWindow_OSX::IsMouseVisible(int deviceId) const
{
	if (deviceId == 0)
	{
		return CGCursorIsVisible();
	}
	return false;
}

void DKWindow_OSX::HoldMouse(int deviceId, bool bHold)
{
	if (deviceId != 0)
		return;

	if (!proxyWindow)
	{
		DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]] == YES);
		[(DKWindowView_OSX*)view holdMouse: (bHold ? YES : NO) ];
	}
}

bool DKWindow_OSX::IsMouseHeld(int deviceId) const
{
	if (!proxyWindow)
	{
		DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]] == YES);
		if (deviceId == 0)
			return [(DKWindowView_OSX*)view isMouseHeld];
	}
	return false;
}

void DKWindow_OSX::SetMousePosition(int deviceId, const DKPoint& pt)
{
	if (!proxyWindow)
	{
		DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]] == YES);
		if (deviceId == 0)
			[(DKWindowView_OSX*)view setMousePosition: pt];
	}
}

DKPoint DKWindow_OSX::MousePosition(int deviceId) const
{
	if (!proxyWindow)
	{
		DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]] == YES);
		if (deviceId == 0)
			return ((DKWindowView_OSX*)view).mousePosition;
	}
	return DKPoint(-1,-1);
}

void DKWindow_OSX::EnableTextInput(int deviceId, bool enable)
{
	if (!proxyWindow)
	{
		DKASSERT_DEBUG([view isKindOfClass:[DKWindowView_OSX class]] == YES);
		if (deviceId == 0)
			((DKWindowView_OSX*)view).textInput = enable;
	}
}

bool DKWindow_OSX::IsValid(void) const
{
	if (window && view)
		return true;
		
	return false;			
}

DKVirtualKey DKWindow_OSX::ConvertVKey(unsigned long key)
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

