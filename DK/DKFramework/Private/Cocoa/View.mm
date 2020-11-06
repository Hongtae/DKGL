//
//  File: Application.mm
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import "View.h"

@interface _DKView ()
@property (nonatomic, strong) NSString* markedText;
@end

@implementation _DKView
@synthesize markedText;
@synthesize userInstance;
@synthesize textInput;

using WindowEvent = DKWindow::WindowEvent;
using KeyboardEvent = DKWindow::KeyboardEvent;
using MouseEvent = DKWindow::MouseEvent;

- (void)setup
{
	modifierKeyFlags = 0;
	holdMouse = NO;

	self.textInput = 0;
	self.userInstance = NULL;
	self.markedText = nil;

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResizeNotification:) name:NSWindowDidResizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillMiniaturizeNotification:) name:NSWindowWillMiniaturizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidMiniaturizeNotification:) name:NSWindowDidMiniaturizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidDeminiaturizeNotification:) name:NSWindowDidDeminiaturizeNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidBecomeKeyNotification:) name:NSWindowDidBecomeKeyNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResignKeyNotification:) name:NSWindowDidResignKeyNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidMoveNotification:) name:NSWindowDidMoveNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowWillCloseNotification:) name:NSWindowWillCloseNotification object:nil];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidHideNotification:) name:NSApplicationDidHideNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidUnhideNotification:) name:NSApplicationDidUnhideNotification object:nil];
}

- (instancetype)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
		[self setup];
	return self;
}

- (instancetype)initWithCoder:(NSCoder*)coder
{
	self = [super initWithCoder:coder];
	if (self)
		[self setup];
	return self;
}

- (instancetype)init
{
	self = [super init];
	if (self)
		[self setup];
	return self;
}

- (void)dealloc
{
	self.markedText = nil;
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (void)drawRect:(NSRect)rect
{
	[self postWindowEventType:WindowEvent::WindowUpdate];
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return YES;
}

- (BOOL)isFlipped
{
	return YES;	// set upper-left corner is the origin of the view.
}

- (void)setHoldMouse:(BOOL)hold
{
	if (userInstance)
	{
		holdMouse = hold;
		CGAssociateMouseAndMouseCursorPosition(!holdMouse);
	}
	else
		holdMouse = NO;
}

- (BOOL)isMouseHeld
{
	if (userInstance)
		return holdMouse;
	return NO;
}

- (void)setMousePosition:(DKPoint)pt
{
	NSPoint ptInView = NSMakePoint(pt.x, pt.y);
	NSPoint ptInWindow = [self convertPoint:ptInView toView:nil];
	NSRect rcInScreen = [self.window convertRectToScreen:NSMakeRect(ptInWindow.x, ptInWindow.y, 0, 0)];
	NSPoint screenPos = rcInScreen.origin;

	DKLog("setting mouse pos: (%d,%d) (screen-position)\n", (int)screenPos.x, (int)screenPos.y);

	CGPoint toMove;
	toMove.x = screenPos.x;
	toMove.y = CGDisplayPixelsHigh(CGMainDisplayID()) - screenPos.y;
	CGWarpMouseCursorPosition(toMove);
}

- (DKPoint)mousePosition
{
	// get mouse pos with screen-space
	NSPoint ptInScreen = [NSEvent mouseLocation];
	// convert pos to window-space
	NSRect rcInWindow = [self.window convertRectFromScreen:NSMakeRect(ptInScreen.x, ptInScreen.y, 0, 0)];
	// convert pos to view-space
	NSPoint ptInView = [self convertPoint:rcInWindow.origin fromView:nil];
	
	return DKPoint(ptInView.x, ptInView.y);
}

- (void)setTextInput:(BOOL)enabled
{
	if (textInput != enabled)
	{
		self.markedText = nil;
		[self.inputContext discardMarkedText];
	}
	textInput = enabled;
}

- (DKRect)contentRect
{
	NSRect rect = self.bounds;
	if (self.window)
		rect = [self convertRect:rect toView:nil];
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

- (DKRect)windowRect
{
	NSRect rect = self.frame;
	if (self.window)
		rect = self.window.frame;
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

- (CGFloat)contentScaleFactor
{
	if (self.window)
		return self.window.backingScaleFactor;
	return 1.0;
}

- (DKVirtualKey)convertVKey:(unsigned short)key
{
	switch (key)
	{
		case 0x00:	return DKVirtualKey::A;				// a
		case 0x01:	return DKVirtualKey::S;				// s
		case 0x02:	return DKVirtualKey::D;				// d
		case 0x03:	return DKVirtualKey::F;				// f
		case 0x04:	return DKVirtualKey::H;				// h
		case 0x05:	return DKVirtualKey::G;				// g
		case 0x06:	return DKVirtualKey::Z;				// z
		case 0x07:	return DKVirtualKey::X;				// x
		case 0x08:	return DKVirtualKey::C;				// c
		case 0x09:	return DKVirtualKey::V;				// v
		case 0x0A:	return DKVirtualKey::None;
		case 0x0B:	return DKVirtualKey::B;				// b
		case 0x0C:	return DKVirtualKey::Q;				// q
		case 0x0D:	return DKVirtualKey::W;				// w
		case 0x0E:	return DKVirtualKey::E;				// e
		case 0x0F:	return DKVirtualKey::R;				// r
		case 0x10:	return DKVirtualKey::Y;				// y
		case 0x11:	return DKVirtualKey::T;				// t
		case 0x12:	return DKVirtualKey::Num1;				// 1
		case 0x13:	return DKVirtualKey::Num2;				// 2
		case 0x14:	return DKVirtualKey::Num3;				// 3
		case 0x15:	return DKVirtualKey::Num4;				// 4
		case 0x16:	return DKVirtualKey::Num6;				// 6
		case 0x17:	return DKVirtualKey::Num5;				// 5
		case 0x18:	return DKVirtualKey::Equal;			// =
		case 0x19:	return DKVirtualKey::Num9;				// 9
		case 0x1A:	return DKVirtualKey::Num7;				// 7
		case 0x1B:	return DKVirtualKey::Hyphen;			// -
		case 0x1C:	return DKVirtualKey::Num8;				// 8
		case 0x1D:	return DKVirtualKey::Num0;				// 0
		case 0x1E:	return DKVirtualKey::CloseBracket;	// ]
		case 0x1F:	return DKVirtualKey::O;				// o
		case 0x20:	return DKVirtualKey::U;				// u
		case 0x21:	return DKVirtualKey::OpenBracket;	// [
		case 0x22:	return DKVirtualKey::I;				// i
		case 0x23:	return DKVirtualKey::P;				// p
		case 0x24:	return DKVirtualKey::Return;			// return
		case 0x25:	return DKVirtualKey::L;				// l
		case 0x26:	return DKVirtualKey::J;				// j
		case 0x27:	return DKVirtualKey::Quote;			// '
		case 0x28:	return DKVirtualKey::K;				// k
		case 0x29:	return DKVirtualKey::Semicolon;		// ;
		case 0x2A:	return DKVirtualKey::Backslash;		// backslash
		case 0x2B:	return DKVirtualKey::Comma;			// ,
		case 0x2C:	return DKVirtualKey::Slash;			// /
		case 0x2D:	return DKVirtualKey::N;				// n
		case 0x2E:	return DKVirtualKey::M;				// m
		case 0x2F:	return DKVirtualKey::Period;			// .
		case 0x30:	return DKVirtualKey::Tab;			// tab
		case 0x31:	return DKVirtualKey::Space;			// space
		case 0x32:	return DKVirtualKey::AccentTilde;	// ` (~)
		case 0x33:	return DKVirtualKey::Backspace;		// delete (backspace)
		case 0x34:	return DKVirtualKey::None;			//
		case 0x35:	return DKVirtualKey::Escape;			// esc
		case 0x36:	return DKVirtualKey::None;
		case 0x37:	return DKVirtualKey::None;
		//case 0x38:	return DKVirtualKey::LeftShift;		// l-shift
		//case 0x39:	return DKVirtualKey::None;
		//case 0x3A:	return DKVirtualKey::LeftOption;		// l-alt
		//case 0x3B:	return DKVirtualKey::LeftControl;	// l_ctrl
		//case 0x3C:	return DKVirtualKey::RightShift;		// r-shift
		//case 0x3D:	return DKVirtualKey::RightOption;	// r-alt
		//case 0x3E:	return DKVirtualKey::RightControl;	// r-ctrl
		case 0x3F:	return DKVirtualKey::None;
		case 0x40:	return DKVirtualKey::F17;			// f17
		case 0x41:	return DKVirtualKey::PadPeriod;		// . (keypad)
		case 0x42:	return DKVirtualKey::None;
		case 0x43:	return DKVirtualKey::PadAsterisk;	// * (keypad)
		case 0x44:	return DKVirtualKey::None;
		case 0x45:	return DKVirtualKey::PadPlus;		// + (keypad)
		case 0x46:	return DKVirtualKey::None;
		case 0x47:	return DKVirtualKey::Numlock;		// clear (keypad)
		case 0x48:	return DKVirtualKey::None;
		case 0x49:	return DKVirtualKey::None;
		case 0x4A:	return DKVirtualKey::None;
		case 0x4B:	return DKVirtualKey::PadSlash;		// / (keypad)
		case 0x4C:	return DKVirtualKey::Enter;			// enter (keypad)
		case 0x4D:	return DKVirtualKey::None;
		case 0x4E:	return DKVirtualKey::PadMinus;		// - (keypad)
		case 0x4F:	return DKVirtualKey::F18;			// f18
		case 0x50:	return DKVirtualKey::F19;			// f19
		case 0x51:	return DKVirtualKey::PadEqual;		// = (keypad)
		case 0x52:	return DKVirtualKey::Pad0;			// 0 (keypad)
		case 0x53:	return DKVirtualKey::Pad1;			// 1 (keypad)
		case 0x54:	return DKVirtualKey::Pad2;			// 2 (keypad)
		case 0x55:	return DKVirtualKey::Pad3;			// 3 (keypad)
		case 0x56:	return DKVirtualKey::Pad4;			// 4 (keypad)
		case 0x57:	return DKVirtualKey::Pad5;			// 5 (keypad)
		case 0x58:	return DKVirtualKey::Pad6;			// 6 (keypad)
		case 0x59:	return DKVirtualKey::Pad7;			// 7 (keypad)
		case 0x5A:	return DKVirtualKey::None;
		case 0x5B:	return DKVirtualKey::Pad8;			// 8 (keypad)
		case 0x5C:	return DKVirtualKey::Pad9;			// 9 (keypad)
		case 0x5D:	return DKVirtualKey::None;
		case 0x5E:	return DKVirtualKey::None;
		case 0x5F:	return DKVirtualKey::None;
		case 0x60:	return DKVirtualKey::F5;				// f5
		case 0x61:	return DKVirtualKey::F6;				// f6
		case 0x62:	return DKVirtualKey::F7;				// f7
		case 0x63:	return DKVirtualKey::F3;				// f3
		case 0x64:	return DKVirtualKey::F8;				// f8
		case 0x65:	return DKVirtualKey::None;
		case 0x66:	return DKVirtualKey::None;
		case 0x67:	return DKVirtualKey::None;
		case 0x68:	return DKVirtualKey::None;
		case 0x69:	return DKVirtualKey::F13;			// f13
		case 0x6A:	return DKVirtualKey::F16;			// f16
		case 0x6B:	return DKVirtualKey::F14;			// f14
		case 0x6C:	return DKVirtualKey::None;
		case 0x6D:	return DKVirtualKey::None;
		case 0x6E:	return DKVirtualKey::None;
		case 0x6F:	return DKVirtualKey::None;
		case 0x70:	return DKVirtualKey::None;
		case 0x71:	return DKVirtualKey::F15;			// f15
		case 0x72:	return DKVirtualKey::None;
		case 0x73:	return DKVirtualKey::Home;			// home
		case 0x74:	return DKVirtualKey::PageUp;			// page up
		case 0x75:	return DKVirtualKey::Delete;			// delete (below insert key)
		case 0x76:	return DKVirtualKey::F4;				// f4
		case 0x77:	return DKVirtualKey::End;			// end
		case 0x78:	return DKVirtualKey::F2;				// f2
		case 0x79:	return DKVirtualKey::PageDown;		// page down
		case 0x7A:	return DKVirtualKey::F1;				// f1
		case 0x7B:	return DKVirtualKey::Left;			// left
		case 0x7C:	return DKVirtualKey::Right;			// right
		case 0x7D:	return DKVirtualKey::Down;			// down
		case 0x7E:	return DKVirtualKey::Up;				// up
		case 0x7F:	return DKVirtualKey::None;
		default:	return DKVirtualKey::None;
	}
	return DKVirtualKey::None;
}

#pragma mark - Post Event To DKWindow instance
- (void)postWindowEventType:(WindowEvent::Type)type
{
	if (userInstance)
		userInstance->PostWindowEvent({type, self.windowRect, self.contentRect, self.contentScaleFactor});
}

- (void)postKeyboardEventType:(KeyboardEvent::Type)type keyCode:(unsigned short)keyCode
{
	if (userInstance)
		userInstance->PostKeyboardEvent({type, 0, [self convertVKey:keyCode], ""});
}

- (void)postKeyboardEventType:(KeyboardEvent::Type)type mappedVKey:(DKVirtualKey)vKey
{
	if (userInstance)
		userInstance->PostKeyboardEvent({type, 0, vKey, ""});
}

- (void)postTextInputEvent:(NSString*)text
{
	if (userInstance)
		userInstance->PostKeyboardEvent({KeyboardEvent::TextInput, 0, DKVirtualKey::None, text.UTF8String});
}

- (void)postTextCompositingEvent:(NSString*)text
{
	if (userInstance)
		userInstance->PostKeyboardEvent({KeyboardEvent::TextComposition, 0, DKVirtualKey::None, text.UTF8String});
}

- (void)postMouseEvent:(NSEvent*)event
{
	if (userInstance)
	{
		MouseEvent::Type eventType = MouseEvent::Move;
		MouseEvent::Device device = MouseEvent::GenericMouse;
		float pressure = 0.0f;
		NSPoint tilt = {0.0, 0.0};

		switch (event.type)
		{
				// mouse event
			case NSEventTypeLeftMouseDown:
			case NSEventTypeRightMouseDown:
			case NSEventTypeOtherMouseDown:
				eventType = MouseEvent::ButtonDown;
				break;
			case NSEventTypeLeftMouseUp:
			case NSEventTypeRightMouseUp:
			case NSEventTypeOtherMouseUp:
				eventType = MouseEvent::ButtonUp;
				break;
			case NSEventTypeMouseMoved:
			case NSEventTypeLeftMouseDragged:
			case NSEventTypeRightMouseDragged:
			case NSEventTypeOtherMouseDragged:
				eventType = MouseEvent::Move;
				break;
			case NSEventTypeScrollWheel:
				eventType = MouseEvent::Wheel;
				break;
#if 0
				// tablet event (Not tested!)
			case NSEventTypeTabletPoint:
			case NSEventTypeTabletProximity:
				eventType = MouseEvent::Pointing;
				device = MouseEvent::StylusPen;
				pressure = event.pressure;
				tilt = event.tilt;
				break;
#if __LP64__
			case NSEventTypePressure:
				eventType = MouseEvent::Pointing;
				pressure = event.pressure;
				break;
#endif
#endif
			default:	// Unsupported event type for now.
				return;
				break;
		}

		NSPoint location = [self convertPoint:event.locationInWindow fromView:nil];
		DKVector2 delta = {0.0, 0.0};
		if (eventType == MouseEvent::Move)
		{
			delta.x = event.deltaX;	// event.deltaX,Y is screen-coordinates
			delta.y = event.deltaY;
		}
		else if (eventType == MouseEvent::Wheel)
		{
			delta.x = event.scrollingDeltaX;
			delta.y = event.scrollingDeltaY;
		}

		int deviceId = 0;
		int buttonId = (int)event.buttonNumber;

#if 0
		DKLog("EventSubtype: %d", event.subtype);
		switch (event.subtype)
		{
			case NSEventSubtypeMouseEvent:
				break;
			case NSEventSubtypeTabletPoint:
			case NSEventSubtypeTabletProximity:
				pressure = event.pressure;
				tilt = event.tilt;
				break;
			case NSEventSubtypeTouch:
				device = MouseEvent::Touch;
				break;
			default:
				device = MouseEvent::Unknown;
				DKLog("[MouseEvent] Unknown device! (%d)", event.subtype);
				break;
		}
#endif
		userInstance->PostMouseEvent({
			eventType,
			device,
			deviceId,
			buttonId,
			DKPoint(location.x, location.y),
			delta,
			pressure,
			(float)(tilt.x * tilt.y)
		});
	}
}

#pragma mark - Mouse Event
- (void)mouseDownEvent:(NSEvent*)event
{
	if (self.textInput)
		[self unmarkText];

	[self postMouseEvent:event];

//	if ([event.window isKeyWindow] == NO)	// activate window by clicking button which is not left-button.
//	{
//		[NSApp unhide:nil];
//	}
}

- (void)mouseUpEvent:(NSEvent*)event
{
	if (self.textInput)
		[self unmarkText];

	[self postMouseEvent:event];

	// in case of window activated by clicking button (which is not left-button)
	// mouse move events turned off, activate again.
//	[event.window setAcceptsMouseMovedEvents:YES];
}

- (void)mouseMoveEvent:(NSEvent*)event
{
	if (event.deltaX == 0 && event.deltaY == 0)
		return;

	[self postMouseEvent:event];
}

// mouse down
- (void)mouseDown:(NSEvent*)event			{[self mouseDownEvent:event];}
- (void)rightMouseDown:(NSEvent*)event		{[self mouseDownEvent:event];}
- (void)otherMouseDown:(NSEvent*)event		{[self mouseDownEvent:event];}
// mouse move
- (void)mouseMoved:(NSEvent*)event			{[self mouseMoveEvent:event];}
- (void)mouseDragged:(NSEvent*)event		{[self mouseMoveEvent:event];}
- (void)rightMouseDragged:(NSEvent*)event	{[self mouseMoveEvent:event];}
- (void)otherMouseDragged:(NSEvent*)event	{[self mouseMoveEvent:event];}
// mouse up
- (void)mouseUp:(NSEvent*)event				{[self mouseUpEvent:event];}
- (void)rightMouseUp:(NSEvent*)event		{[self mouseUpEvent:event];}
- (void)otherMouseUp:(NSEvent*)event		{[self mouseUpEvent:event];}
// mouse wheel
- (void)scrollWheel:(NSEvent*)event			{[self postMouseEvent:event];}

#pragma mark - Table Event
- (void)pressureChangeWithEvent:(NSEvent *)event	{[self postMouseEvent:event];}
- (void)tabletPoint:(NSEvent *)event				{[self postMouseEvent:event];}
- (void)tabletProximity:(NSEvent *)event			{[self postMouseEvent:event];}

#pragma mark - Keyboard Event
- (void)keyDown:(NSEvent*)event
{
	if (self.textInput)
	{
		[self.inputContext handleEvent:event];
	}

	if (event.ARepeat == NO)
		[self postKeyboardEventType:KeyboardEvent::KeyDown keyCode:event.keyCode];
}

- (void)keyUp:(NSEvent*)event
{
	if (self.textInput)
	{
	}
	[self postKeyboardEventType:KeyboardEvent::KeyUp keyCode:event.keyCode];
}

- (void)flagsChanged:(NSEvent*)event
{
	// flags for shift, control, option.
	// we can determine left or right key with modifierFlags.

	//	NSLog(@"handler::modifierKey (0x%08x)\n", event.modifierFlags);
	[self updateModifier:event.modifierFlags];
}

#define LEFT_SHIFT_BIT			0x20002
#define RIGHT_SHIFT_BIT			0x20004
#define LEFT_CONTROL_BIT		0x40001
#define RIGHT_CONTROL_BIT		0x42000
#define LEFT_ALTERNATE_BIT		0x80020
#define RIGHT_ALTERNATE_BIT		0x80040
#define LEFT_COMMAND_BIT		0x100008
#define RIGHT_COMMAND_BIT		0x100010

- (void)updateModifier:(NSEventModifierFlags)modifier
{
	// CapsLock Key
	if ((modifierKeyFlags & NSEventModifierFlagCapsLock) != (modifier & NSEventModifierFlagCapsLock))
	{
		if (modifier & NSEventModifierFlagCapsLock)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::Capslock];
		}
		else
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::Capslock];
		}
	}
	// Shift Key
	if ((modifierKeyFlags & NSEventModifierFlagShift ) != (modifier & NSEventModifierFlagShift))
	{
		if ((modifier & LEFT_SHIFT_BIT) == LEFT_SHIFT_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::LeftShift];
		}
		else if ((modifierKeyFlags & LEFT_SHIFT_BIT) == LEFT_SHIFT_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::LeftShift];
		}
		if ((modifier & RIGHT_SHIFT_BIT) == RIGHT_SHIFT_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::RightShift];
		}
		else if ((modifierKeyFlags & RIGHT_SHIFT_BIT) == RIGHT_SHIFT_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::RightShift];
		}
	}
	// Control Key
	if ((modifierKeyFlags & NSEventModifierFlagControl) != (modifier & NSEventModifierFlagControl))
	{
		if ((modifier & LEFT_CONTROL_BIT) == LEFT_CONTROL_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::LeftControl];
		}
		else if ((modifierKeyFlags & LEFT_CONTROL_BIT) == LEFT_CONTROL_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::LeftControl];
		}
		if ((modifier & RIGHT_CONTROL_BIT) == RIGHT_CONTROL_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::RightControl];
		}
		else if ((modifierKeyFlags & RIGHT_CONTROL_BIT) == RIGHT_CONTROL_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::RightControl];
		}
	}
	// Option (Alt) Key
	if ((modifierKeyFlags & NSEventModifierFlagOption) != (modifier & NSEventModifierFlagOption))
	{
		if ((modifier & LEFT_ALTERNATE_BIT) == LEFT_ALTERNATE_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::LeftOption];
		}
		else if ((modifierKeyFlags & LEFT_ALTERNATE_BIT) == LEFT_ALTERNATE_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::LeftOption];
		}
		if ((modifier & RIGHT_ALTERNATE_BIT) == RIGHT_ALTERNATE_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::RightOption];
		}
		else if ((modifierKeyFlags & RIGHT_ALTERNATE_BIT) == RIGHT_ALTERNATE_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::RightOption];
		}
	}
	// Command Key
	if ((modifierKeyFlags & NSEventModifierFlagCommand) != (modifier & NSEventModifierFlagCommand))
	{
		if ((modifier & LEFT_COMMAND_BIT) == LEFT_COMMAND_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::LeftCommand];
		}
		else if ((modifierKeyFlags & LEFT_COMMAND_BIT) == LEFT_COMMAND_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::LeftCommand];
		}
		if ((modifier & RIGHT_COMMAND_BIT) == RIGHT_COMMAND_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::RightCommand];
		}
		else if ((modifierKeyFlags & RIGHT_COMMAND_BIT) == RIGHT_COMMAND_BIT)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::RightCommand];
		}
	}
	// Numeric Pad (any key in the numeric keypad)
	if ((modifierKeyFlags & NSEventModifierFlagNumericPad) != (modifier & NSEventModifierFlagNumericPad))
	{
	}
	// Help Key
	if ((modifierKeyFlags & NSEventModifierFlagHelp) != (modifier & NSEventModifierFlagHelp))
	{
	}
	// Fn Key
	if ((modifierKeyFlags & NSEventModifierFlagFunction) != (modifier & NSEventModifierFlagFunction))
	{
		if (modifier & NSEventModifierFlagFunction)
		{
			[self postKeyboardEventType:KeyboardEvent::KeyDown mappedVKey:DKVirtualKey::Fn];
		}
		else
		{
			[self postKeyboardEventType:KeyboardEvent::KeyUp mappedVKey:DKVirtualKey::Fn];

		}
	}
	
	modifierKeyFlags = modifier;
}

#pragma mark - NSTextInputClient protocol
- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
	NSString* str = @"";
	if ([string isKindOfClass:[NSString class]])
		str = string;
	else if ([string isKindOfClass:[NSAttributedString class]])
		str = [string string];

	[self postTextInputEvent:str];

	if (self.markedText.length > 0)
		[self postTextCompositingEvent:@""];

	self.markedText = nil;
	[self.inputContext discardMarkedText];
	[self.inputContext invalidateCharacterCoordinates];
}

- (void)doCommandBySelector:(SEL)selector
{
	struct TextBySelector
	{
		SEL selector;
		NSString* text;
	};
	std::initializer_list<TextBySelector> tsList = {
		{ @selector(insertLineBreak:), @"\r" },
		{ @selector(insertNewline:), @"\n" },
		{ @selector(insertTab:), @"\t" },
		{ @selector(deleteBackward:), @"\b" },
		{ @selector(deleteBackwardByDecomposingPreviousCharacter:), @"\b" },
		{ @selector(cancelOperation:), @"\e" },
	};

	bool processed = false;
	for (auto& ts : tsList)
	{
		if (selector == ts.selector)
		{
			[self insertText:ts.text replacementRange:NSMakeRange(NSNotFound, 0)];
			processed = true;
			break;
		}
	}
	if (!processed)
	{
#ifdef DKGL_DEBUG_ENABLED
		NSEvent *event = [[self window] currentEvent];
		DKVirtualKey key = [self convertVKey:event.keyCode];
		NSLog(@"[NSTextInput] doCommandBySelector:(%@) for key:(%@) not processed.\n",
			  NSStringFromSelector(selector),
			  [NSString stringWithUTF8String:(const char *)DKStringU8(DKWindow::GetVKName(key))]);
#endif
	}

	self.markedText = nil;
	[self.inputContext discardMarkedText];
	[self.inputContext invalidateCharacterCoordinates];
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
	if (self.textInput)
	{
		NSString* str = @"";
		if ([string isKindOfClass:[NSString class]])
			str = string;
		else if ([string isKindOfClass:[NSAttributedString class]])
			str = [string string];

		if (str.length > 0)
		{
			self.markedText = str;
			NSLog(@"self.markedText: %@\n", self.markedText);

			[self postTextCompositingEvent:str];
		}
		else
		{
			[self postTextCompositingEvent:@""];

			self.markedText = nil;
			[self.inputContext discardMarkedText];
		}
		[self.inputContext invalidateCharacterCoordinates]; // recentering
	}
	else
	{
		self.markedText = nil;
		[self.inputContext discardMarkedText];
	}
}

- (void)unmarkText
{
	if (self.textInput && self.markedText.length > 0)
	{
		[self insertText:self.markedText replacementRange:NSMakeRange(NSNotFound, 0)];
	}

	self.markedText = nil;
	[self.inputContext discardMarkedText];
}

- (NSRange)selectedRange
{
	return NSMakeRange(NSNotFound, 0);
}

- (NSRange)markedRange
{
	if (self.markedText.length > 0)
		return NSMakeRange(0, self.markedText.length);

	return NSMakeRange(NSNotFound, 0);
}

- (BOOL)hasMarkedText
{
	return self.markedText.length > 0;
}

- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
	return nil;
}

- (NSArray<NSString *> *)validAttributesForMarkedText
{
	return [NSArray array];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
	return NSMakeRect(0,0,0,0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
	return NSNotFound;
}

#pragma mark - NSDraggingDestination protocol
- (NSDragOperation)dragOperationCallback:(id<NSDraggingInfo>)draggingInfo withState:(DKWindow::DraggingState)draggingState
{
	if (userInstance)
	{
		const DKWindow::WindowCallback& cb = userInstance->Callback();
		if (cb.draggingFeedback)
		{
			NSPasteboard *pboard = [draggingInfo draggingPasteboard];
			NSWindow* target = [draggingInfo draggingDestinationWindow];

			if ([self window] == target)
			{
				if ([pboard.types containsObject:NSFilenamesPboardType])
				{
					NSArray* files = [pboard propertyListForType:NSFilenamesPboardType];

					DKStringArray filenameList;
					filenameList.Reserve(files.count);
					for (NSString* str in files)
					{
						if ([str isKindOfClass:[NSString class]])
							filenameList.Add(str.UTF8String);
					}
					NSPoint pos = [draggingInfo draggingLocation];
					pos = [self convertPoint:pos fromView:nil];

					DKWindow::DragOperation op = cb.draggingFeedback->Invoke(userInstance,
																			 draggingState,
																			 DKPoint(pos.x, pos.y),
																			 filenameList);

					NSDragOperation dragOperation = [draggingInfo draggingSourceOperationMask];
					switch (op)
					{
						case DKWindow::DragOperationCopy:
							dragOperation = dragOperation & NSDragOperationCopy;
							break;
						case DKWindow::DragOperationMove:
							dragOperation = dragOperation & NSDragOperationMove;
							break;
						case DKWindow::DragOperationLink:
							dragOperation = dragOperation & NSDragOperationLink;
							break;
						case DKWindow::DragOperationNone:
							dragOperation = dragOperation & NSDragOperationNone;
							break;
						default:
							dragOperation = dragOperation & NSDragOperationGeneric;
							break;
					}
					return dragOperation;
				}
			}
		}
	}
	return NSDragOperationNone;
}

- (BOOL)wantsPeriodicDraggingUpdates
{
	return NO;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
	return [self dragOperationCallback:sender withState:DKWindow::DraggingEntered];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
	return [self dragOperationCallback:sender withState:DKWindow::DraggingUpdated];
}

- (void)draggingEnded:(id<NSDraggingInfo>)sender
{
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
	[self dragOperationCallback:sender withState:DKWindow::DraggingExited];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
	NSDragOperation op = [self dragOperationCallback:sender withState:DKWindow::DraggingDropped];
	return op != DKWindow::DragOperationNone;
}

#pragma mark - NSWindowDelegate protocol
- (BOOL)windowShouldClose:(id)window
{
	if (userInstance && self.window == window)
	{
		const DKWindow::WindowCallback& cb = userInstance->Callback();
		if (cb.closeRequest)
		{
			if (!cb.closeRequest->Invoke(userInstance))
				return NO;
		}
	}
	return YES;
}

- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)proposedFrameSize
{
	if (userInstance && self.window == window)
	{
		NSRect contentRect = [window contentRectForFrameRect:NSMakeRect(0,0, proposedFrameSize.width, proposedFrameSize.height)];

		const DKWindow::WindowCallback& cb = userInstance->Callback();
		if (cb.contentMinSize)
		{
			DKSize minSize = cb.contentMinSize->Invoke(userInstance);

			if (contentRect.size.width < minSize.width)
				contentRect.size.width = minSize.width;
			if (contentRect.size.height < minSize.height)
				contentRect.size.height = minSize.height;
		}
		if (cb.contentMaxSize)
		{
			DKSize maxSize = cb.contentMaxSize->Invoke(userInstance);

			if (maxSize.width > 0 && contentRect.size.width > maxSize.width)
				contentRect.size.width = maxSize.width;
			if (maxSize.height > 0 && contentRect.size.height > maxSize.height)
				contentRect.size.height = maxSize.height;
		}

		NSRect frameRect = [window frameRectForContentRect:contentRect];
		return frameRect.size;
	}
	return proposedFrameSize;
}

#pragma mark - NSWindow Notifications
- (void)windowDidResizeNotification:(NSNotification*)notification
{
	//	NSRect rc = [[notification object] frame];
	//	NSRect rc = [self bounds];
	//	DKLog("Window resized. (%f x %f)\n", rc.size.width, rc.size.height);

	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowResized];
 }

- (void)windowWillMiniaturizeNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowInactivated];
}

- (void)windowDidMiniaturizeNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowMinimized];
}

- (void)windowDidDeminiaturizeNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowShown];
}

- (void)windowDidBecomeKeyNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
	{
		NSEvent *currentEvent = [NSApp currentEvent];
		[self postWindowEventType:WindowEvent::WindowActivated];
		[self updateModifier:currentEvent.modifierFlags];
	}
}

- (void)windowDidResignKeyNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
	{
		if (self.textInput)
			[self unmarkText];

		[self postWindowEventType:WindowEvent::WindowInactivated];
	}
}

- (void)windowDidMoveNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowMoved];
}

- (void)windowWillCloseNotification:(NSNotification*)notification
{
	if (self.window && self.window == notification.object)
		[self postWindowEventType:WindowEvent::WindowClosed];
}

#pragma mark - NSApplication Notifications
- (void)applicationDidHideNotification:(NSNotification*)notification
{
	if (self.window && self.window.visible)
	{
		[self postWindowEventType:WindowEvent::WindowHidden];
	}
}

- (void)applicationDidUnhideNotification:(NSNotification*)notification
{
	if (self.window && self.window.visible)
	{
		[self postWindowEventType:WindowEvent::WindowShown];
		if (self.window.keyWindow)
			[self postWindowEventType:WindowEvent::WindowActivated];
	}
}
@end

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__)
