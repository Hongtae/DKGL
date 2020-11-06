//
//  File: View.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import "View.h"

#pragma mark - _DKView ()
@interface _DKView ()<UITextFieldDelegate>
@property (readwrite, nonatomic, strong) NSMutableArray* touches;
@property (readwrite, nonatomic, assign) BOOL appActivated;
@property (readwrite, nonatomic) UITextField* textField;
@end

#define DEFAULT_TEXTFIELD_HEIGHT	30
#define DEFAULT_TEXTFIELD_MARGIN	2

@implementation _DKView

using WindowEvent = DKWindow::WindowEvent;
using KeyboardEvent = DKWindow::KeyboardEvent;
using MouseEvent = DKWindow::MouseEvent;

- (void)setup
{
	// Initialization code
	self.touches = [[NSMutableArray alloc] initWithCapacity:10];
	self.userInstance = nil;

	_textInput = NO;

	self.backgroundColor = [UIColor clearColor];
	self.multipleTouchEnabled = YES;
	self.exclusiveTouch = YES;
	self.userInteractionEnabled = YES;
	[super setHidden:YES];

	self.contentScaleFactor = [[UIScreen mainScreen] scale];

	CGRect frame = self.bounds;
	self.textField = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, frame.size.width - DEFAULT_TEXTFIELD_MARGIN * 2, DEFAULT_TEXTFIELD_HEIGHT)];
	self.textField.delegate = self;
	self.textField.textAlignment = NSTextAlignmentCenter;
	self.textField.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
	self.textField.backgroundColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.4];
	self.textField.textColor = [UIColor blackColor];
	self.textField.borderStyle = UITextBorderStyleRoundedRect;
	self.textField.clearButtonMode = UITextFieldViewModeAlways;
	self.textField.hidden = YES;
	self.textField.clearsOnBeginEditing = YES;
	self.textField.autoresizingMask = UIViewAutoresizingFlexibleWidth;
	[self.textField adjustsFontSizeToFitWidth];
	[self.textField addTarget:self action:@selector(updateTextField:) forControlEvents:UIControlEventEditingChanged];

	[self addSubview:self.textField];

	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShowNotification:) name:UIKeyboardWillShowNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHideNotification:) name:UIKeyboardWillHideNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillChangeFrameNotification:) name:UIKeyboardWillChangeFrameNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidChangeFrameNotification:) name:UIKeyboardDidChangeFrameNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActiveNotification:) name:UIApplicationWillResignActiveNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActiveNotification:) name:UIApplicationDidBecomeActiveNotification object:nil];

	self.appActivated = YES;
	if ([[UIApplication sharedApplication] respondsToSelector:@selector(applicationState)])
		self.appActivated = [[UIApplication sharedApplication] applicationState] == UIApplicationStateActive;
}

- (instancetype)initWithFrame:(CGRect)frame
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
	self = [super initWithFrame:[UIScreen mainScreen].bounds];
	if (self)
		[self setup];
	return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	self.textField.delegate = nil;
	self.textField = nil;
	self.touches = nil;
	[super dealloc];
}

+ (Class)layerClass
{
	return [CAMetalLayer class];
}

- (void)layoutSubviews
{
	[super layoutSubviews];
}

- (BOOL)becomeFirstResponder
{
	if ([super becomeFirstResponder])
	{
		if (self.appActivated)
		{
			if (self.userInstance)
				self.userInstance->PostWindowEvent({ WindowEvent::WindowActivated, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
		return YES;
	}
	return NO;
}

- (BOOL)resignFirstResponder
{
	if ([super resignFirstResponder])
	{
		if (self.appActivated)
		{
			if (self.userInstance)
				self.userInstance->PostWindowEvent({ WindowEvent::WindowInactivated, self.windowRect, self.contentRect, self.contentScaleFactor });
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

- (void)setFrame:(CGRect)frame
{
	if (self.userInstance)
	{
		CGRect old = self.frame;
		[super setFrame:frame];
		// if (!CGRectEqualToRect(frame, old)) // CGRectEqualToRect requires CoreAnimation.framework
		if (!CGPointEqualToPoint(frame.origin, old.origin) || !CGSizeEqualToSize(frame.size, old.size))
		{
			if (CGSizeEqualToSize(frame.size, old.size))
				self.userInstance->PostWindowEvent({ WindowEvent::WindowMoved, self.windowRect, self.contentRect, self.contentScaleFactor });
			else
				self.userInstance->PostWindowEvent({ WindowEvent::WindowResized, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
	}
	else
		[super setFrame:frame];
}

- (void)setHidden:(BOOL)flag
{
	if (self.hidden != flag && self.appActivated)
	{
		if (flag)
		{
			if (self.userInstance)
				self.userInstance->PostWindowEvent({ WindowEvent::WindowHidden, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
		else
		{
			if (self.userInstance)
				self.userInstance->PostWindowEvent({ WindowEvent::WindowShown, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
	}
	[super setHidden:flag];
}

- (DKRect)windowRect
{
	CGRect frame = self.bounds;
	if (self.window)
	{
		frame = self.window.frame;
	}
	return DKRect(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
}

- (DKRect)contentRect
{
	CGRect rect = self.bounds;
	return DKRect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

#pragma mark - Handle Touch Events
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch* touch in touches)
	{
		NSUInteger index = self.touches.count;
		// find empty slot in self.touches
		for (NSUInteger i = 0; i < self.touches.count; ++i)
		{
			if (self.touches[i] == [NSNull null])
			{
				index = i;
				break;
			}
		}
		if (index == self.touches.count)	// no empty slot found. add one
			[self.touches addObject:[NSNull null]];

		if (self.userInstance)
		{
			MouseEvent::Device device = touch.type == UITouchTypeStylus ? MouseEvent::Stylus : MouseEvent::Touch;
			CGPoint pos = [touch locationInView:self];
			self.userInstance->PostMouseEvent({
				MouseEvent::ButtonDown,
				device,
				static_cast<int>(index), 0,
				DKPoint(pos.x, pos.y),
				DKVector2::zero,
				static_cast<float>(touch.force),
				static_cast<float>(touch.altitudeAngle)
			});
		}
		self.touches[index] = touch;
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch* touch in touches)
	{
		if (self.userInstance)
		{
			BOOL processed = NO;
			for (NSUInteger index = 0; index < self.touches.count; ++index)
			{
				if (self.touches[index] == touch)
				{
					MouseEvent::Device device = touch.type == UITouchTypeStylus ? MouseEvent::Stylus : MouseEvent::Touch;
					CGPoint pos = [touch locationInView:self];
					CGPoint old = [touch previousLocationInView:self];

					self.userInstance->PostMouseEvent({
						MouseEvent::Move,
						device,
						static_cast<int>(index), 0,
						DKPoint(pos.x, pos.y),
						DKVector2(pos.x - old.x, pos.y - old.y),
						static_cast<float>(touch.force),
						static_cast<float>(touch.altitudeAngle)
					});
					processed = YES;
					break;
				}
			}
			if (!processed)
				NSLog(@"Untrackable touch event: %s", __PRETTY_FUNCTION__);
		}
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch* touch in touches)
	{
		BOOL processed = NO;
		for (NSUInteger index = 0; index < self.touches.count; ++index)
		{
			if (self.touches[index] == touch)
			{
				if (self.userInstance)
				{
					MouseEvent::Device device = touch.type == UITouchTypeStylus ? MouseEvent::Stylus : MouseEvent::Touch;
					CGPoint pos = [touch locationInView:self];
					CGPoint old = [touch previousLocationInView:self];

					self.userInstance->PostMouseEvent({
						MouseEvent::ButtonUp,
						device,
						static_cast<int>(index), 0,
						DKPoint(pos.x, pos.y),
						DKVector2(pos.x - old.x, pos.y - old.y),
						static_cast<float>(touch.force),
						static_cast<float>(touch.altitudeAngle)
					});
				}
				self.touches[index] = [NSNull null];
				processed = YES;
				break;
			}
		}
		if (!processed)
			NSLog(@"Untrackable touch event: %s", __PRETTY_FUNCTION__);
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesEnded:touches withEvent:event];
}

- (void)motionBegan:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	[super motionBegan:motion withEvent:event];
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	[super motionEnded:motion withEvent:event];
}

- (void)motionCancelled:(UIEventSubtype)motion withEvent:(UIEvent *)event
{
	[super motionCancelled:motion withEvent:event];
}

- (DKPoint)touchPositionAtIndex:(NSUInteger)index
{
	UITouch* touch = nil;
	if (index < self.touches.count)
	{
		id obj = self.touches[index];
		if ([obj isKindOfClass:[UITouch class]])
			touch = obj;
	}
	if (touch)
	{
		CGPoint pt = [touch locationInView:self];
		return DKPoint(pt.x, pt.y);
	}
	return DKPoint(-1,-1);
}

#pragma mark - TextInput
- (void)setTextInput:(BOOL)enable
{
	_textInput = enable;
	if (enable)
	{
		DKLog("TextInput enabled.\n");
		self.textField.hidden = NO;
		[self.textField becomeFirstResponder];
	}
	else
	{
		DKLog("TextInput disabled.\n");
		[self.textField resignFirstResponder];
		self.textField.hidden = YES;
	}
}

// UITextFieldDelegate protocol
#pragma mark - UITextFieldDelegate
- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
	return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField
{
	return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
	DKLog("%s\n", DKGL_FUNCTION_NAME);

	if (self.textInput)		// User closed keyboard (iPad)
	{
		if (self.userInstance)
		{
			if ([textField.text length] > 0)
				self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextInput, 0, DKVirtualKey::None, textField.text.UTF8String });
			self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextInput, 0, DKVirtualKey::None, "\e" });
			self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextComposition, 0, DKVirtualKey::None, "" });
		}
		self.textInput = NO;
	}
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	return YES;
}

- (void)updateTextField:(UITextField *)textField
{
	if (self.userInstance)
		self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextComposition, 0, DKVirtualKey::None, textField.text.UTF8String });
}

- (BOOL)textFieldShouldClear:(UITextField *)textField
{
	if (self.userInstance)
		self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextComposition, 0, DKVirtualKey::None, "" });
	return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
	if (self.userInstance)
	{
		if ([textField.text length] > 0)
			self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextInput, 0, DKVirtualKey::None, textField.text.UTF8String });
		self.userInstance->PostKeyboardEvent({ KeyboardEvent::TextInput, 0, DKVirtualKey::None, "\n" });
	}
	textField.text = @"";
	return YES;
}

#pragma mark - Notifications
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

	CGRect textFieldFrame = self.textField.frame;
	textFieldFrame.origin.y = keyboardFrame.origin.y - textFieldFrame.size.height - DEFAULT_TEXTFIELD_MARGIN;

	self.textField.frame = textFieldFrame;
}

- (void)applicationWillResignActiveNotification:(NSNotification*)notification
{
	if (self.hidden == NO)
	{
		if (self.userInstance)
		{
			if ([self isFirstResponder])
				self.userInstance->PostWindowEvent({ WindowEvent::WindowInactivated, self.windowRect, self.contentRect, self.contentScaleFactor });

			self.userInstance->PostWindowEvent({ WindowEvent::WindowHidden, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
	}
	self.appActivated = NO;
}

- (void)applicationDidBecomeActiveNotification:(NSNotification*)notification
{
	if (self.hidden == NO)
	{
		if (self.userInstance)
		{
			self.userInstance->PostWindowEvent({ WindowEvent::WindowShown, self.windowRect, self.contentRect, self.contentScaleFactor });

		if ([self isFirstResponder])
			self.userInstance->PostWindowEvent({ WindowEvent::WindowActivated, self.windowRect, self.contentRect, self.contentScaleFactor });
		}
	}
	self.appActivated = YES;
}

@end

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
