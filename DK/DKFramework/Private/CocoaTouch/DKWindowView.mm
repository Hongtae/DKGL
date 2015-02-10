//
//  File: DKWindowView.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKWindowView for iOS

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "DKWindowImpl.h"
#import "DKWindowView.h"

using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		DKString NSStringToIGString(NSString*);
		NSString* DKStringToNSString(const DKString&);

		namespace
		{
			struct TouchInfo
			{
				UITouch*	touch;
				CGPoint		lastPosition;
				int			tick;
			};
		}
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

#pragma mark - DKWindowView
////////////////////////////////////////////////////////////////////////////////
// DKWindowView
@interface DKWindowView ()
{
	DKWindow*			handler;
	DKArray<TouchInfo>	touchTrackingData;
	int					tick;
	
	UITextField*		textInputField;
	BOOL				textInputEnabled;
	float				windowScale;
	BOOL				appActivated;	
}
- (void)trackTouchInfo:(NSArray*)touches;
@end

@implementation DKWindowView
@synthesize handler;
@synthesize origin;
@synthesize contentSize;
@synthesize appActivated;

- (id)initWithFrame:(CGRect)frame handler:(DKWindow*)h;
{
    if ((self = [super initWithFrame:frame]))
	{
        // Initialization code
		tick = 0;
		handler = h;
		
		textInputEnabled = NO;
		
		self.backgroundColor = [UIColor clearColor];  
		self.multipleTouchEnabled = YES;
		self.exclusiveTouch = YES;
		self.userInteractionEnabled = YES;
		[super setHidden:YES];

		self.contentScaleFactor = [[UIScreen mainScreen] scale];

		int textFieldHeight = 30;
		int textFieldWidth = frame.size.width - 30;
		
		CGRect textInputFieldRect = CGRectMake((frame.size.width - textFieldWidth)/2, -textFieldHeight, textFieldWidth, textFieldHeight);
		
		textInputField = [[UITextField alloc] initWithFrame:textInputFieldRect];
		textInputField.delegate = self;
		textInputField.textAlignment = NSTextAlignmentCenter;
		textInputField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
		textInputField.backgroundColor = [UIColor whiteColor];
		textInputField.textColor = [UIColor blackColor];
		textInputField.borderStyle = UITextBorderStyleLine;
		textInputField.clearButtonMode = UITextFieldViewModeAlways;
		textInputField.hidden = YES;
		textInputField.opaque = NO;
		textInputField.alpha = 0.75;
		textInputField.clearsOnBeginEditing = YES;
		[textInputField adjustsFontSizeToFitWidth];
		[textInputField addTarget:self action:@selector(onUpdateTextField:) forControlEvents:UIControlEventEditingChanged];
		
		[self addSubview:textInputField];

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onKeyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAppWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onAppDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
		
		appActivated = YES;
		if ([[UIApplication sharedApplication] respondsToSelector:@selector(applicationState)])
			appActivated = [[UIApplication sharedApplication] applicationState] == UIApplicationStateActive;		
	}
	return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[textInputField release];
	[super dealloc];
}

- (void)setViewOrigin:(DKPoint)pt
{
	CGRect rect = self.frame;
	rect.origin = CGPointMake(pt.x, pt.y);
	self.frame = rect;
}

- (DKPoint)origin
{
	CGPoint pt = self.frame.origin;
	return DKPoint(pt.x, pt.y);
}

- (void)setContentSize:(DKSize)size
{
	if (size.width < 1)
		size.width = 1;
	if (size.height < 1)
		size.height = 1;

	CGFloat scaleFactor = 1.0 / self.contentScaleFactor;
	CGRect rect = self.bounds;
	rect.size.width = size.width * scaleFactor;
	rect.size.height = size.height * scaleFactor;
	self.bounds = rect;
}

- (DKSize)contentSize
{
	CGFloat scaleFactor = self.contentScaleFactor;
	CGRect rect = [self bounds];
	return DKSize(rect.size.width * scaleFactor, rect.size.height * scaleFactor);
}

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (void)layoutSubviews
{
	handler->PostWindowEvent(DKWindow::EventWindowResized, self.contentSize, self.origin, false);
}

- (BOOL)becomeFirstResponder
{
	if ([super becomeFirstResponder])
	{
		if (appActivated)
		{
			handler->PostWindowEvent(DKWindow::EventWindowActivated, self.contentSize, self.origin, false);
		}		
		return YES;
	}
	return NO;
}

- (BOOL)resignFirstResponder
{
	if ([super resignFirstResponder])
	{
		if (appActivated)
		{
			handler->PostWindowEvent(DKWindow::EventWindowInactivated, self.contentSize, self.origin, false);
		}		
		return YES;
	}
	return NO;
}

- (BOOL)canBecomeFirstResponder
{
	return YES;
}

- (BOOL)canResignFirstResponder
{
	return YES;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self trackTouchInfo: [[event touchesForView:self] allObjects]];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self trackTouchInfo: [[event touchesForView:self] allObjects]];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self trackTouchInfo: [[event touchesForView:self] allObjects]];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self trackTouchInfo: [[event touchesForView:self] allObjects]];
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	DKLog("[%s]\n", DKLIB_FUNCTION_NAME);
}

- (void)trackTouchInfo:(NSArray*)touches
{
	tick++;
	
	const float frameHeight = self.bounds.size.height;
	const float scaleFactor = self.contentScaleFactor;
	
	for (UITouch* touch in touches)
	{
		CGPoint pos = [touch locationInView: self];
		pos.x = pos.x * scaleFactor;
		pos.y = (frameHeight - pos.y) * scaleFactor;
		
		// If touch.phase is UITouchPhaseEnded, UITouchPhaseCancelled,
		// and cannot find touch object, then just ignore that.
		bool newTouch = touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled;

		for (int i = 0; i < touchTrackingData.Count(); i++)
		{
			TouchInfo& info = touchTrackingData.Value(i);
			if (info.touch == touch)
			{
				newTouch = false;
				
				CGPoint old = info.lastPosition;
				info.lastPosition = pos;

				if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled)
				{
					// send message and update tick if phase is not ended, not cancelled.
					info.tick = tick;
					if (pos.x != old.x || pos.y != old.y)
					{
						handler->PostMouseEvent(DKWindow::EventMouseMove, i, 0, DKPoint(pos.x, pos.y), DKVector2(pos.x - old.x, pos.y - old.y), false);
					}
				}
				break;
			}
		}			
		if (newTouch)
		{			
			long index = -1;
			for (int i = 0; i < touchTrackingData.Count(); ++i)
			{
				TouchInfo& info = touchTrackingData.Value(i);
				if (info.touch == nil)
				{
					// keep hold state
					info.touch = touch;
					info.tick = tick;
					info.lastPosition = pos;
					index = i;
					break;
				}
			}
			if (index == -1)
			{
				TouchInfo info;
				info.touch = touch;
				info.tick = tick;
				info.lastPosition = pos;
				index = touchTrackingData.Add(info);
			}
			// new position (index) added.
			handler->PostMouseEvent(DKWindow::EventMouseDown, index, 0, DKPoint(pos.x, pos.y), DKVector2(0,0), false);
		}
	}
	// remove unused touches
	for (int i = 0; i < touchTrackingData.Count(); i++)
	{
		TouchInfo& info = touchTrackingData.Value(i);
		if (info.touch != nil && info.tick != tick)
		{
			info.touch = nil;
			info.tick = tick;
			handler->PostMouseEvent(DKWindow::EventMouseUp, i, 0, DKPoint(info.lastPosition.x, info.lastPosition.y), DKVector2(0,0), false);
		}
	}
}

- (void)setTouchPosition:(const DKPoint&)pos atIndex:(NSUInteger)index
{
	if (index < touchTrackingData.Count())
	{
		CGPoint pt;
		pt.x = pos.x;
		pt.y = pos.y;
		touchTrackingData.Value(index).lastPosition = pt;
	}
}

- (DKPoint)touchPositionAtIndex:(NSUInteger)index
{
	if (index < touchTrackingData.Count())
		return DKPoint(touchTrackingData.Value(index).lastPosition.x, touchTrackingData.Value(index).lastPosition.y);
	return DKPoint(-1,-1);
}

- (void)enableTextInput:(BOOL)enable
{
	// 2015-01-22 by Hongtae Kim
	// when text-input enabled, screen freeze can be occurred.
	// disable text-input temporary.

	DKLog("EnableTextInput is temporary disabled on iOS.\n");
	textInputEnabled = NO;
	textInputField.hidden = YES;
	return;

/*
	if (enable)
	{
		textInputEnabled = YES;
		DKLog("TextInput enabled.\n");
		[textInputField performSelectorOnMainThread:@selector(becomeFirstResponder) withObject:nil waitUntilDone:YES];
	}
	else
	{
		textInputEnabled = NO;
		DKLog("TextInput disabled.\n");
		[textInputField performSelectorOnMainThread:@selector(resignFirstResponder) withObject:nil waitUntilDone:YES];
		textInputField.hidden = YES;
	}
*/
}

// UITextFieldDelegate protocol
#pragma mark UITextFieldDelegate
- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
	textInputField.text = @"";
	textInputField.hidden = NO;
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField
{
	return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	DKLog("%s\n", DKLIB_FUNCTION_NAME);
	
	if (textInputEnabled)		// User closed keyboard (iPad)
	{
		if ([textField.text length] > 0)
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, NSStringToIGString(textField.text), false);
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, L"\e", false);
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);

		handler->SetTextInputEnabled(0, false);
	}
	textInputField.hidden = YES;
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	return YES;
}

- (void)onUpdateTextField:(UITextField *)textField
{
	if (textInputEnabled)
	{
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, NSStringToIGString(textInputField.text), false);
	}
}

- (BOOL)textFieldShouldClear:(UITextField *)textField
{
	handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, L"", false);
	return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	if (textInputEnabled)
	{
		if ([textInputField.text length] > 0)
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, NSStringToIGString(textField.text), false);
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, L"\n", false);
	}
	
	textInputField.text = @"";
	return YES;
}

////////////////////////////////////////////////////////////////////////////////
// Notificaitons
#pragma mark Notifications
- (void)onKeyboardWillShow:(NSNotification*)notification
{
	NSDictionary* info = [notification userInfo];	
	
	// Get the size of the keyboard.
    NSValue* aValue = [info objectForKey:UIKeyboardFrameEndUserInfoKey];
    CGRect keyboardRect = [self convertRect:[aValue CGRectValue] fromView:nil];
	CGRect viewRect = self.bounds;
	
	CGRect textFieldRect = textInputField.frame;
	
	// set position to below of input-frame and above of on-screen keyboard.
	int textFieldH = textFieldRect.size.height;
	int textFieldW = textFieldRect.size.width;
	int textFieldX = textFieldRect.origin.x;
	int textFieldY = Min<int>(viewRect.origin.y + viewRect.size.height - textFieldH - 15, keyboardRect.origin.y - textFieldH - 15);
	
	textFieldRect = CGRectMake(textFieldX, textFieldY, textFieldW, textFieldH);
	
	textInputField.frame = textFieldRect;
}

- (void)onAppWillResignActive:(NSNotification*)notification
{
	if (self.hidden == NO)
	{
		if ([self isFirstResponder])
			handler->PostWindowEvent(DKWindow::EventWindowInactivated, self.contentSize, self.origin, false);

		// Wait until event processed.
		// access OpenGL will die when program has gone to background.
		handler->PostWindowEvent(DKWindow::EventWindowHidden, self.contentSize, self.origin, true);
	}
	appActivated = NO;
}

- (void)onAppDidBecomeActive:(NSNotification*)notification
{
	if (self.hidden == NO)
	{
		handler->PostWindowEvent(DKWindow::EventWindowShown, self.contentSize, self.origin, false);
		if ([self isFirstResponder])
			handler->PostWindowEvent(DKWindow::EventWindowActivated, self.contentSize, self.origin, false);
	}
	appActivated = YES;
}

- (void)setHidden:(BOOL)flag
{
	if (self.hidden != flag && appActivated)
	{
		if (flag)
		{
			handler->PostWindowEvent(DKWindow::EventWindowHidden, self.contentSize, self.origin, false);
		}
		else
		{
			handler->PostWindowEvent(DKWindow::EventWindowShown, self.contentSize, self.origin, false);
		}
	}
	[super setHidden:flag];	
}

@end

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)

