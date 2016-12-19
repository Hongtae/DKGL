//
//  File: ViewController.mm
//  Platform: iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import "ViewController.h"
#import "View.h"

@interface _DKViewController ()
@property (nonatomic, strong) _DKView* dkview;
@end

@implementation _DKViewController

- (void)loadView
{
	if (self.dkview == nil)
		self.dkview = [[[_DKView alloc] init] autorelease];

	self.view = self.dkview;
}

- (void)dealloc
{
	self.dkview = nil;
	[super dealloc];
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations
{
	return UIInterfaceOrientationMaskAll;
}

@end

#endif //if !TARGET_OS_IPHONE
#endif //if defined(__APPLE__) && defined(__MACH__)
