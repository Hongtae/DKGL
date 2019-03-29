//
//  File: View.h
//  Platform: macOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#if defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>

#if !TARGET_OS_IPHONE
#import <AppKit/AppKit.h>
#include "../../DKWindow.h"

using namespace DKFramework;

@interface _DKView : NSView<NSTextInputClient, NSWindowDelegate, NSDraggingDestination>
{
@private
	DKWindow* userInstance;
	BOOL holdMouse;
	BOOL textInput;
	NSUInteger modifierKeyFlags;
	NSString* markedText;
}

@property (nonatomic, assign) DKWindow* userInstance;
@property (nonatomic) BOOL textInput;
@property (nonatomic) DKPoint mousePosition;
@property (nonatomic, getter=isMouseHeld) BOOL holdMouse;
@property (readonly, nonatomic) DKRect windowRect;
@property (readonly, nonatomic) DKRect contentRect;
@property (readonly, nonatomic) CGFloat contentScaleFactor;

@end

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
