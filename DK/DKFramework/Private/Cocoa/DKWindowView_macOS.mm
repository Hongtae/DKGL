//
//  File: DKWindowView_macOS.mm
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE

#include "../../DKVKey.h"
#include "DKWindow_macOS.h"
#import "DKWindowView_macOS.h"

using namespace DKGL;
using namespace DKGL::Private;


@interface DKWindowView_macOS ()
- (void)mouseDownEvent:(NSEvent*)event;
- (void)mouseUpEvent:(NSEvent*)event;
- (void)mouseMoveEvent:(NSEvent*)event;
- (void)updateModifier:(NSUInteger)modifier;
@property (nonatomic, retain) NSString* markedText;
@end

@implementation DKWindowView_macOS
@synthesize handler;
@synthesize markedText;
@synthesize textInput;

////////////////////////////////////////////////////////////////////////////////
// DKWindowView_macOS initialize
- (id)initWithFrame:(NSRect)frame handler:(DKWindow*)h
{
    self = [super initWithFrame:frame];
    if (self)
	{
        // Initialization code here.
		NSLog(@"Initialize DKWindowCocoaView object\n");
		modifierKeyFlags = 0;
		holdMouse = 0;
		textInput = 0;
		handler = h;
		markedText = nil;
		self.wantsBestResolutionOpenGLSurface = YES;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidHide:) name:NSApplicationDidHideNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidUnhide:) name:NSApplicationDidUnhideNotification object:nil];
    }
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
    // Drawing code here.
	handler->PostWindowEvent(DKWindow::EventWindowUpdate, self.contentSize, self.windowOrigin, false);
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
	return NO;
}

- (DKPoint)windowOrigin
{
	NSRect rc = self.window.frame;
	return DKPoint(rc.origin.x, rc.origin.y);
}

- (DKSize)contentSize
{
	NSRect rc = [self convertRectToBacking:[self bounds]];
	return DKSize(rc.size.width, rc.size.height);
}

- (void)holdMouse:(BOOL)hold
{
	holdMouse = hold;
	if (holdMouse)
	{
		// get mouse pos with screen-space
		NSPoint ptInScreen = [NSEvent mouseLocation];
		// convert pos to window-space
		NSRect rcInWindow = [self.window convertRectFromScreen:NSMakeRect(ptInScreen.x, ptInScreen.y, 0, 0)];
		// convert pos to view-space
		NSPoint ptInView = [self convertPoint:rcInWindow.origin fromView:nil];
		// convert pos to pixel-space
		holdPosition = [self convertPointToBacking:ptInView];

		DKLog("Hold mouse (%.0f, %.0f)\n", holdPosition.x, holdPosition.y);
	}				
}

- (BOOL)isMouseHeld
{
	return holdMouse;
}

- (void)setMousePosition:(DKPoint)pt
{
	NSPoint ptInPixel = NSMakePoint(pt.x, pt.y);
	NSPoint ptInView = [self convertPointFromBacking:ptInPixel];
	NSPoint ptInWindow = [self convertPoint:ptInView toView:nil];
	NSRect rcInScreen = [self.window convertRectToScreen:NSMakeRect(ptInWindow.x, ptInWindow.y, 0, 0)];
	NSPoint screenPos = rcInScreen.origin;
	
	DKLog("setting mouse pos: (%d,%d) (screen-position)\n", (int)screenPos.x, (int)screenPos.y);
		  
	CGPoint toMove;
	toMove.x = screenPos.x;
	toMove.y = CGDisplayPixelsHigh(CGMainDisplayID()) - screenPos.y;
	CGWarpMouseCursorPosition(toMove);

	if (holdMouse)
	{
		[self holdMouse: true];
	}
}

- (DKPoint)mousePosition
{
	// get mouse pos with screen-space
	NSPoint ptInScreen = [NSEvent mouseLocation];
	// convert pos to window-space
	NSRect rcInWindow = [self.window convertRectFromScreen:NSMakeRect(ptInScreen.x, ptInScreen.y, 0, 0)];
	// convert pos to view-space
	NSPoint ptInView = [self convertPoint:rcInWindow.origin fromView:nil];
	// convert pos to pixel-space
	NSPoint pos = [self convertPointToBacking:ptInView];

	return DKPoint(pos.x, pos.y);
}

- (void)setTextInput:(BOOL)enabled
{
	if (textInput != enabled)
	{
		self.markedText = nil;
		[[self inputContext] discardMarkedText];
	}
	textInput = enabled;
}

#pragma mark - NSDraggingDestination
- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
	const DKWindow::WindowCallback& cb = self.handler->Callback();
	if (cb.filesDropped)
	{
		NSPasteboard *pboard = [sender draggingPasteboard];
		NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];

		if ( [[pboard types] containsObject:NSFilenamesPboardType] )
		{
			if (sourceDragMask & NSDragOperationCopy)
				return NSDragOperationCopy;
			if (sourceDragMask & NSDragOperationLink)
				return NSDragOperationLink;
			if (sourceDragMask & NSDragOperationGeneric)
				return NSDragOperationGeneric;
		}
	}
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	const DKWindow::WindowCallback& cb = self.handler->Callback();
	if (cb.filesDropped)
	{
		NSPasteboard *pboard = [sender draggingPasteboard];
		NSWindow* target = [sender draggingDestinationWindow];
		NSPoint pos = [sender draggingLocation];

		if ([self window] == target)
		{
			if ( [[pboard types] containsObject:NSFilenamesPboardType] ) {
				NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];

				pos = [self convertPoint:pos fromView:nil];
				pos = [self convertPointToBacking:pos];
				DKPoint pt(pos.x, pos.y);

				DKWindow::WindowCallback::StringArray fileNames;
				fileNames.Reserve(files.count);

				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				for (NSString* f in files)
				{
					if ([f isKindOfClass:[NSString class]])
					{
						fileNames.Add([f UTF8String]);
					}
				}
				[pool release];

				cb.filesDropped->Invoke(self.handler, pt, fileNames);
			}
			return YES;
		}
	}
	return NO;
}

#pragma mark - Mouse Event Handlers
- (void)mouseDownEvent:(NSEvent*)event
{
	if (self.textInput)
		[self unmarkText];

	NSPoint pt = [self convertPoint:[event locationInWindow] fromView: nil];
	pt = [self convertPointToBacking:pt];

	DKPoint pt2(pt.x, pt.y);
	handler->PostMouseEvent(DKWindow::EventMouseDown, 0, (int)[event buttonNumber], pt2, DKVector2(0,0), false);
	if ([[event window] isKeyWindow] == NO)
	{
		[NSApp unhide];
	}
}

- (void)mouseUpEvent:(NSEvent*)event
{
	if (self.textInput)
		[self unmarkText];

	NSPoint pt = [self convertPoint:[event locationInWindow] fromView: nil];
	pt = [self convertPointToBacking:pt];

	DKPoint pt2(pt.x, pt.y);
	handler->PostMouseEvent(DKWindow::EventMouseUp, 0, (int)[event buttonNumber], pt2, DKVector2(0,0), false);
	// in case of window activated by clicking button (which is not left-button)
	// mouse move events turned off, activate again.
	[[event window] setAcceptsMouseMovedEvents : YES];
}

- (void)mouseMoveEvent:(NSEvent*)event
{
	NSPoint pt = [self convertPoint:[event locationInWindow] fromView:nil];
	pt = [self convertPointToBacking:pt];

	int32_t deltaX = 0;
	int32_t deltaY = 0;
	CGGetLastMouseDelta(&deltaX, &deltaY);
	CGFloat scale = self.window.backingScaleFactor;

	if (deltaX == 0 && deltaY == 0)
		return;
	
	DKPoint pos(pt.x, pt.y);
	DKVector2 delta(deltaX * scale, -deltaY * scale);
	
	if (holdMouse)
	{
		// restore position of mouse
		NSPoint ptInView = [self convertPointFromBacking:holdPosition];
		NSPoint ptInWindow = [self convertPoint:ptInView toView:nil];
		NSRect rcInScreen = [self.window convertRectToScreen:NSMakeRect(ptInWindow.x, ptInWindow.y, 0, 0)];
		NSPoint ptToMove = rcInScreen.origin;
		CGPoint toMove;
		toMove.x = ptToMove.x;
		toMove.y = CGDisplayPixelsHigh(CGMainDisplayID()) - ptToMove.y;
		CGWarpMouseCursorPosition(toMove);
		//CGDisplayMoveCursorToPoint(CGMainDisplayID(), toMove);
	}
	
	handler->PostMouseEvent(DKWindow::EventMouseMove, 0, 0, pos, delta, false);
}

////////////////////////////////////////////////////////////////////////////////
// event handlers

// mouse down
- (void)mouseDown:(NSEvent*)event			{[self mouseDownEvent: event];}
- (void)rightMouseDown:(NSEvent*)event		{[self mouseDownEvent: event];}
- (void)otherMouseDown:(NSEvent*)event		{[self mouseDownEvent: event];}
// mouse move
- (void)mouseMoved:(NSEvent*)event			{[self mouseMoveEvent: event];}
- (void)mouseDragged:(NSEvent*)event		{[self mouseMoveEvent: event];}
- (void)rightMouseDragged:(NSEvent*)event	{[self mouseMoveEvent: event];}
- (void)otherMouseDragged:(NSEvent*)event	{[self mouseMoveEvent: event];}
// mouse up
- (void)mouseUp:(NSEvent*)event				{[self mouseUpEvent: event];}
- (void)rightMouseUp:(NSEvent*)event		{[self mouseUpEvent: event];}
- (void)otherMouseUp:(NSEvent*)event		{[self mouseUpEvent: event];}
// mouse wheel
- (void)scrollWheel:(NSEvent*)event
{
	NSPoint pt = [self convertPoint: [event locationInWindow] fromView: nil];
	DKPoint pos(pt.x, pt.y);

	DKVector2 delta(0,0);
	if ([event hasPreciseScrollingDeltas])
	{
		delta.x = [event scrollingDeltaX];
		delta.y = [event scrollingDeltaY];
	}
	else
	{
		delta.x = [event scrollingDeltaX] * 10;
		delta.y = [event scrollingDeltaY] * 10;
	}

	handler->PostMouseEvent(DKWindow::EventMouseWheel, 0, 0, pos, delta, false);
}

#pragma mark - Keyboard Event Handlers
- (void)keyDown:(NSEvent*)event
{
	if (self.textInput)
	{
		[[self inputContext] handleEvent:event];
	}
	// if ([event isARepeat] == 0)
	handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKWindow_macOS::ConvertVKey( [event keyCode] ), L"", false);
}

- (void)keyUp:(NSEvent*)event
{
	if (self.textInput)
	{
	}
	handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKWindow_macOS::ConvertVKey( [event keyCode] ), L"", false);
}

#define LEFT_SHIFT_BIT		0x20002
#define RIGHT_SHIFT_BIT		0x20004
#define LEFT_CONTROL_BIT	0x40001
#define RIGHT_CONTROL_BIT	0x42000
#define LEFT_ALTERNATE_BIT	0x80020
#define RIGHT_ALTERNATE_BIT	0x80040
#define LEFT_COMMAND_BIT	0x100008
#define RIGHT_COMMAND_BIT	0x100010

- (void)updateModifier:(NSUInteger)modifier
{
	if ((modifierKeyFlags & NSAlphaShiftKeyMask) != (modifier & NSAlphaShiftKeyMask))	// caps lock
	{
		if (modifier & NSAlphaShiftKeyMask)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_CAPSLOCK, L"", false);
		}
		else
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_CAPSLOCK, L"", false);
		}
	}
	if ((modifierKeyFlags & NSShiftKeyMask ) != (modifier & NSShiftKeyMask))
	{
		if ((modifier & LEFT_SHIFT_BIT) == LEFT_SHIFT_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_LEFT_SHIFT, L"", false);
		}
		else if ((modifierKeyFlags & LEFT_SHIFT_BIT) == LEFT_SHIFT_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_LEFT_SHIFT, L"", false);
		}
		if ((modifier & RIGHT_SHIFT_BIT) == RIGHT_SHIFT_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_RIGHT_SHIFT, L"", false);
		}
		else if ((modifierKeyFlags & RIGHT_SHIFT_BIT) == RIGHT_SHIFT_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_RIGHT_SHIFT, L"", false);
		}
	}
	if ((modifierKeyFlags & NSControlKeyMask) != (modifier & NSControlKeyMask))			// CONTROL key
	{
		if ((modifier & LEFT_CONTROL_BIT) == LEFT_CONTROL_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_LEFT_CONTROL, L"", false);
		}
		else if ((modifierKeyFlags & LEFT_CONTROL_BIT) == LEFT_CONTROL_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_LEFT_CONTROL, L"", false);
		}
		if ((modifier & RIGHT_CONTROL_BIT) == RIGHT_CONTROL_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_RIGHT_CONTROL, L"", false);
		}
		else if ((modifierKeyFlags & RIGHT_CONTROL_BIT) == RIGHT_CONTROL_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_RIGHT_CONTROL, L"", false);
		}
	}
	if ((modifierKeyFlags & NSAlternateKeyMask) != (modifier & NSAlternateKeyMask))		// ALT key
	{
		if ((modifier & LEFT_ALTERNATE_BIT) == LEFT_ALTERNATE_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_LEFT_OPTION, L"", false);
		}
		else if ((modifierKeyFlags & LEFT_ALTERNATE_BIT) == LEFT_ALTERNATE_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_LEFT_OPTION, L"", false);
		}
		if ((modifier & RIGHT_ALTERNATE_BIT) == RIGHT_ALTERNATE_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_RIGHT_OPTION, L"", false);
		}
		else if ((modifierKeyFlags & RIGHT_ALTERNATE_BIT) == RIGHT_ALTERNATE_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_RIGHT_OPTION, L"", false);
		}
	}
	if ((modifierKeyFlags & NSCommandKeyMask) != (modifier & NSCommandKeyMask))			// CMD key
	{
		if ((modifier & LEFT_COMMAND_BIT) == LEFT_COMMAND_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_LEFT_COMMAND, L"", false);
		}
		else if ((modifierKeyFlags & LEFT_COMMAND_BIT) == LEFT_COMMAND_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_LEFT_COMMAND, L"", false);
		}
		if ((modifier & RIGHT_COMMAND_BIT) == RIGHT_COMMAND_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_RIGHT_COMMAND, L"", false);
		}
		else if ((modifierKeyFlags & RIGHT_COMMAND_BIT) == RIGHT_COMMAND_BIT)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_RIGHT_COMMAND, L"", false);
		}
	}
	if ((modifierKeyFlags & NSNumericPadKeyMask) != (modifier & NSNumericPadKeyMask))	// num pad
	{
	}
	if ((modifierKeyFlags & NSHelpKeyMask) != (modifier & NSHelpKeyMask))				// HELP key
	{
	}
	if ((modifierKeyFlags & NSFunctionKeyMask) != (modifier & NSFunctionKeyMask))		// FN key
	{
		if (modifier & NSFunctionKeyMask)
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardDown, 0, DKVK_FN, L"", false);
		}
		else
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardUp, 0, DKVK_FN, L"", false);
		}
	}
	
	modifierKeyFlags = modifier;	
}

- (void)flagsChanged:(NSEvent*)event
{
	// flags for shift, control, option.
	// we can determine left or right key with modifierFlags.
	NSUInteger mod = [event modifierFlags];
//	NSLog(@"handler::modifierKey (0x%08x)\n", mod);
	[self updateModifier:mod];
}

////////////////////////////////////////////////////////////////////////////////
// NSTextInputClient protocols
#pragma mark - NSTextInputClient
- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
	NSString* str = @"";
	if ([aString isKindOfClass:[NSString class]])
		str = aString;
	else if ([aString isKindOfClass:[NSAttributedString class]])
		str = [aString string];

	handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, [str UTF8String], false);
	if (self.markedText.length > 0)
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);

	self.markedText = nil;
	[[self inputContext] discardMarkedText];
	[[self inputContext] invalidateCharacterCoordinates];
}

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
	return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)thePoint
{
	return NSNotFound;
}

- (NSInteger)conversationIdentifier
{
	return (NSInteger)self;
}

- (void)doCommandBySelector:(SEL)aSelector
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
		if (aSelector == ts.selector)
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
		DKVirtualKey key = DKWindow_macOS::ConvertVKey([event keyCode]);
		NSLog(@"[NSTextInput] doCommandBySelector:(%@) for key:(%@) not processed.\n",
			  NSStringFromSelector(aSelector),
			  [NSString stringWithUTF8String:(const char *)DKStringU8(DKWindow::GetVKName(key))]);
#endif
	}

	self.markedText = nil;
	[[self inputContext] discardMarkedText];
	[[self inputContext] invalidateCharacterCoordinates];
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange actualRange:(NSRangePointer)actualRange
{
	return NSMakeRect(0,0,0,0);
}

- (BOOL)hasMarkedText
{
	return self.markedText.length > 0;
}

- (NSRange)markedRange
{
	if (self.markedText.length > 0)
		return NSMakeRange(0, self.markedText.length);

	return NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange
{
	return NSMakeRange(NSNotFound, 0);
}

- (void)setMarkedText:(id)aString selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
	if (self.textInput)
	{
		NSString* str = @"";
		if ([aString isKindOfClass:[NSString class]])
			str = aString;
		else if ([aString isKindOfClass:[NSAttributedString class]])
			str = [aString string];

		if (str.length > 0)
		{
			self.markedText = str;
			NSLog(@"self.markedText: %@\n", self.markedText);
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, [str UTF8String], false);
		}
		else
		{
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);
			self.markedText = nil;
			[[self inputContext] discardMarkedText];
		}
		[[self inputContext] invalidateCharacterCoordinates]; // recentering
	}
	else
	{
		self.markedText = nil;
		[[self inputContext] discardMarkedText];
	}
}

- (void)unmarkText
{
	if (self.textInput && self.markedText.length > 0)
	{
		[self insertText:self.markedText replacementRange:NSMakeRange(NSNotFound, 0)];
	}

	self.markedText = nil;
	[[self inputContext] discardMarkedText];
}

- (NSArray *)validAttributesForMarkedText
{
	return [NSArray array];
}

////////////////////////////////////////////////////////////////////////////////
// NSApplication Notifications
#pragma mark - NSApplication Notifications
- (void)applicationDidHide:(NSNotification *)notification
{
	NSWindow* window = [self window];
	if (window == nil)
		return;

	if ([window isVisible])
	{
		handler->PostWindowEvent(DKWindow::EventWindowHidden, self.contentSize, self.windowOrigin, false);
	}
}

- (void)applicationDidUnhide:(NSNotification *)notification
{
	NSWindow* window = [self window];
	if (window == nil)
		return;
	
	if ([window isVisible])
	{
		handler->PostWindowEvent(DKWindow::EventWindowShown, self.contentSize, self.windowOrigin, false);
		if ([window isKeyWindow])
			handler->PostWindowEvent(DKWindow::EventWindowActivated, self.contentSize, self.windowOrigin, false);
	}
}

////////////////////////////////////////////////////////////////////////////////
// NSWindow events
#pragma mark - NSWindowDelegate
- (BOOL)windowShouldClose:(id)window
{
	const DKWindow::WindowCallback& cb = handler->Callback();
	if (cb.closeRequest)
	{
		if (!cb.closeRequest->Invoke(handler))
			return NO;
	}
	return YES;
}

- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)proposedFrameSize
{
	NSRect contentRect = [window contentRectForFrameRect:NSMakeRect(0,0, proposedFrameSize.width, proposedFrameSize.height)];

	const DKWindow::WindowCallback& cb = handler->Callback();
	if (cb.contentMinSize)
	{
		DKSize minSize = cb.contentMinSize->Invoke(handler);

		if (contentRect.size.width < minSize.width)
			contentRect.size.width = minSize.width;
		if (contentRect.size.height < minSize.height)
			contentRect.size.height = minSize.height;
	}
	if (cb.contentMaxSize)
	{
		DKSize maxSize = cb.contentMaxSize->Invoke(handler);

		if (maxSize.width > 0 && contentRect.size.width > maxSize.width)
			contentRect.size.width = maxSize.width;
		if (maxSize.height > 0 && contentRect.size.height > maxSize.height)
			contentRect.size.height = maxSize.height;
	}

	NSRect frameRect = [window frameRectForContentRect:contentRect];
	return frameRect.size;
}

- (void)windowDidResize:(NSNotification *)notification
{
//	NSRect rc = [[notification object] frame];
//	NSRect rc = [self bounds];
//	DKLog("Window resized. (%f x %f)\n", rc.size.width, rc.size.height);
	DKLog("Window did resize)\n");		
	handler->PostWindowEvent(DKWindow::EventWindowResized, self.contentSize, self.windowOrigin, false);
}

- (void)windowWillMiniaturize:(NSNotification *)notification
{
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
	handler->PostWindowEvent(DKWindow::EventWindowMinimized, self.contentSize, self.windowOrigin, false);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
	handler->PostWindowEvent(DKWindow::EventWindowShown, self.contentSize, self.windowOrigin, false);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
	NSEvent *currentEvent = [NSApp currentEvent];
	handler->PostWindowEvent(DKWindow::EventWindowActivated, self.contentSize, self.windowOrigin, false);
	[self updateModifier: [currentEvent modifierFlags]];
}

- (void)windowDidResignKey:(NSNotification *)notification
{
	if (self.textInput)
		[self unmarkText];

	handler->PostWindowEvent(DKWindow::EventWindowInactivated, self.contentSize, self.windowOrigin, false);
}

- (void)windowDidMove:(NSNotification *)notification
{
	handler->PostWindowEvent(DKWindow::EventWindowMoved, self.contentSize, self.windowOrigin, false);
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
}

- (void)windowDidResignMain:(NSNotification *)notification
{
}

- (void)windowWillClose:(NSNotification*)notification
{
	handler->PostWindowEvent(DKWindow::EventWindowClosed, self.contentSize, self.windowOrigin, false);
}

@end

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
