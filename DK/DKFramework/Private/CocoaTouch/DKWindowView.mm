//
//  File: DKWindowView.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKWindowView for iOS

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#include "DKWindowImpl.h"
#import "DKWindowView.h"

using namespace DKGL;
namespace DKGL
{
	namespace Private
	{
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
using namespace DKGL;
using namespace DKGL::Private;

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

constexpr int defaultTextFieldHeight = 30;
constexpr int defaultTextFieldMargin = 2;

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

		textInputField = [[UITextField alloc] initWithFrame:CGRectMake(defaultTextFieldMargin, frame.size.height - defaultTextFieldHeight - defaultTextFieldMargin, frame.size.width - defaultTextFieldMargin * 2, defaultTextFieldHeight)];
		textInputField.delegate = self;
		textInputField.textAlignment = NSTextAlignmentCenter;
		textInputField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
		textInputField.backgroundColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.4];
		textInputField.textColor = [UIColor blackColor];
		textInputField.borderStyle = UITextBorderStyleRoundedRect;
		textInputField.clearButtonMode = UITextFieldViewModeAlways;
		textInputField.hidden = YES;
		textInputField.clearsOnBeginEditing = YES;
		textInputField.autoresizingMask = UIViewAutoresizingFlexibleWidth;
		[textInputField adjustsFontSizeToFitWidth];
		[textInputField addTarget:self action:@selector(onUpdateTextField:) forControlEvents:UIControlEventEditingChanged];
		
		[self addSubview:textInputField];


		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShowNotification:) name:UIKeyboardWillShowNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHideNotification:) name:UIKeyboardWillHideNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillChangeFrameNotification:) name:UIKeyboardWillChangeFrameNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidChangeFrameNotification:) name:UIKeyboardDidChangeFrameNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActiveNotification:) name:UIApplicationWillResignActiveNotification object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActiveNotification:) name:UIApplicationDidBecomeActiveNotification object:nil];
		
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
	[super layoutSubviews];
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
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);
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
	if (enable)
	{
		textInputEnabled = YES;
		dispatch_async(dispatch_get_main_queue(), ^{
			DKLog("TextInput enabled.\n");
			textInputField.hidden = NO;
			[textInputField becomeFirstResponder];
		});
	}
	else
	{
		textInputEnabled = NO;
		dispatch_async(dispatch_get_main_queue(), ^{
			DKLog("TextInput disabled.\n");
			[textInputField resignFirstResponder];
			//textInputField.hidden = YES;
		});
	}
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
	DKLog("%s\n", DKGL_FUNCTION_NAME);
	
	if (textInputEnabled)		// User closed keyboard (iPad)
	{
		if ([textField.text length] > 0)
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, [textField.text UTF8String], false);
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
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInputCandidate, 0, DKVK_NONE, [textInputField.text UTF8String], false);
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
			handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, [textField.text UTF8String], false);
		handler->PostKeyboardEvent(DKWindow::EventKeyboardTextInput, 0, DKVK_NONE, L"\n", false);
	}
	
	textInputField.text = @"";
	return YES;
}

////////////////////////////////////////////////////////////////////////////////
// Notificaitons
#pragma mark Notifications
- (void)keyboardWillShowNotification:(NSNotification*)notification
{
}

- (void)keyboardWillHideNotification:(NSNotification*)notification
{
}

- (void)keyboardWillChangeFrameNotification:(NSNotification*)notification
{
}

- (void)keyboardDidChangeFrameNotification:(NSNotification*)notification
{
	NSDictionary* info = [notification userInfo];

	CGRect keyboardFrame = [[info objectForKey:UIKeyboardFrameEndUserInfoKey] CGRectValue];
	keyboardFrame = [self convertRect:keyboardFrame fromView:nil];

	CGRect textFieldFrame = textInputField.frame;
	textFieldFrame.origin.y = keyboardFrame.origin.y - textFieldFrame.size.height - defaultTextFieldMargin;

	textInputField.frame = textFieldFrame;
}

- (void)applicationWillResignActiveNotification:(NSNotification*)notification
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

- (void)applicationDidBecomeActiveNotification:(NSNotification*)notification
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

