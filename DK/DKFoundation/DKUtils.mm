//
//  File: DKUtils.mm
//  Platform: OSX, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)

#import <TargetConditionals.h>
#if TARGET_OS_IPHONE
#warning Compiling DKUtils for iOS
#import <UIKit/UIKit.h>
#else
#warning Compiling DKUtils for Mac OS X
#import <AppKit/AppKit.h>
#endif	// if TARGET_OS_IPHONE

#include <sys/types.h>
#include <sys/sysctl.h>
#include "DKUtils.h"

namespace DKGL
{
	namespace Private
	{
		void PerformOperationInsidePool(DKOperation* op)
		{
			NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

			// initialize multi-threading mode for Cocoa
			if ([NSThread isMultiThreaded] == NO)
			{
				NSThread *thread = [[[NSThread alloc] init] autorelease];
				[thread start];
			}

			op->Perform();
			[pool release];
		}
	}

	DKGL_API uint32_t DKRandom(void)
	{
		return arc4random();
	}

	DKGL_API DKString DKTemporaryDirectory(void)
	{
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSString* tmp = NSTemporaryDirectory();
		DKString ret([tmp UTF8String]);
		[pool release];

		return ret;
	}
	
	DKGL_API DKArray<DKString> DKProcessArguments(void)
	{
		DKArray<DKString> result;
		
		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		NSProcessInfo* pinfo = [NSProcessInfo processInfo];
		NSArray* args = [pinfo arguments];
		
		for (NSString* s in args)
		{
			result.Add(DKString([s UTF8String]));
		}
		[pool release];
		return result;
	}
	
	DKGL_API DKMap<DKString, DKString> DKProcessEnvironments(void)
	{
		__block DKMap<DKString, DKString> result;

		NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
		
		NSProcessInfo* pinfo = [NSProcessInfo processInfo];
		NSDictionary* dict = [pinfo environment];
		[dict enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop)
		 {
			 if ([key isKindOfClass:[NSString class]] && [obj isKindOfClass:[NSString class]])
			 {
				 DKString k([key UTF8String]);
				 DKString v([obj UTF8String]);
				 result.Update(k, v);
			 }
		 }];
		[pool release];
		return result;
	}

	DKGL_API uint32_t DKNumberOfProcessors(void)
	{
		int ncpu = 0;
		int mib[2];
		size_t len = sizeof(ncpu);
		mib[0] = CTL_HW;
		mib[1] = HW_NCPU;
		if (sysctl(mib, 2, &ncpu, &len, NULL, 0) != 0)
			ncpu = 0;

		if (ncpu >= 1)
			return ncpu;
		return 1;
	}
}
#endif //if defined(__APPLE__) && defined(__MACH__)

