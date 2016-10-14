//
//  getpath.m
//  PyDKLauncher
//
//  Created by tiff on 2015. 1. 18..
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//


// getting script path, located in PyDK framework.
#import <Foundation/Foundation.h>

#include <DK/dk.h>

using namespace DKGL;

DKString AbsolutePathStringOfPyStd(void)
{
	DKString str;
	@autoreleasepool {
		NSBundle* bundle = [NSBundle bundleWithIdentifier:@"com.icondb.PyDK"];
		NSString* s = [bundle pathForResource:@"python35" ofType:@"zip"];
		NSLog(@"python std lib: %@", s);
		str.SetValue(s.UTF8String);
	}
	return str;
}
