//
//  File: DKWindowView.h
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__) && defined(__OBJC__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE

#import <UIKit/UIKit.h>

#import "../../../DKFoundation.h"
#import "../../DKWindow.h"

////////////////////////////////////////////////////////////////////////////////
// DKWindowView
// using UITextFile to process text input.
//

@interface DKWindowView : UIView <UITextFieldDelegate>

@property (readonly, nonatomic)		DKFramework::DKWindow* handler;
@property (readwrite, nonatomic)	DKFramework::DKPoint origin;		// OS unit based
@property (readwrite, nonatomic)	DKFramework::DKSize contentSize;	// pixel based
@property (readonly, nonatomic)		BOOL appActivated;

- (id)initWithFrame:(CGRect)frame handler:(DKFramework::DKWindow*)handler;
- (void)setTouchPosition:(const DKFramework::DKPoint&)pos atIndex:(NSUInteger)index;
- (DKFramework::DKPoint)touchPositionAtIndex:(NSUInteger)index;
- (void)enableTextInput:(BOOL)enable;
@end

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__) && defined(__OBJC__)

