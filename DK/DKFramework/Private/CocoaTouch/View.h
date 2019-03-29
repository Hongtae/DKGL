//
//  File: View.h
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
#include "../../DKWindow.h"

using namespace DKFramework;

@interface _DKView : UIView

@property (readwrite, nonatomic) DKWindow* userInstance;
@property (readwrite, nonatomic) BOOL textInput;

@property (readonly, nonatomic) DKRect windowRect;
@property (readonly, nonatomic) DKRect contentRect;
@property (readonly, nonatomic) UITextField* textField;

- (DKPoint)touchPositionAtIndex:(NSUInteger)index;

@end

#endif	//if TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__) && defined(__OBJC__)
