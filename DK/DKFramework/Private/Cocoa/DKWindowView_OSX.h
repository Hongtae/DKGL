//
//  File: DKWindowView_OSX.h
//  Platform: Mac OS X
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#pragma once

#if defined(__APPLE__) && defined(__MACH__) && defined(__OBJC__)

#import <TargetConditionals.h>
#if !TARGET_OS_IPHONE

#import <Cocoa/Cocoa.h>

#include "../../../DKFoundation.h"
#include "../../DKWindow.h"

using namespace DKGL;

@interface DKWindowView_OSX : NSView <NSTextInputClient, NSWindowDelegate>
{
@private
	DKWindow* handler;
	BOOL holdMouse;
	BOOL textInput;
	NSPoint holdPosition;
	NSUInteger modifierKeyFlags;
	NSString* markedText;
}

@property (readonly, nonatomic) DKWindow* handler;
@property (readonly, nonatomic) DKPoint windowOrigin;
@property (readonly, nonatomic) DKSize contentSize;
@property (readwrite, nonatomic) DKPoint mousePosition;
@property (readwrite, nonatomic) BOOL textInput;

- (id)initWithFrame:(NSRect)frame handler:(DKWindow*)handler;
- (void)holdMouse:(BOOL)hold;
- (BOOL)isMouseHeld;
@end

#endif	//if !TARGET_OS_IPHONE
#endif	//if defined(__APPLE__) && defined(__MACH__) && defined(__OBJC__)
