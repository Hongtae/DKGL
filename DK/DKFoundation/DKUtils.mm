//
//  File: DKUtils.mm
//  Platform: OSX, iOS
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#if defined(__APPLE__) && defined(__MACH__)
#import <TargetConditionals.h>

#if TARGET_OS_IPHONE
#warning Compiling DKUtils for iOS
#import <UIKit/UIKit.h>
#else
#warning Compiling DKUtils for macOS
#import <AppKit/AppKit.h>
#endif	// if TARGET_OS_IPHONE

#include <sys/types.h>
#include <sys/sysctl.h>
#include "DKUtils.h"

namespace DKFoundation
{
	namespace Private
	{
		bool InitializeMultiThreadedEnvironment(void)
		{
			if ([NSThread isMultiThreaded] == NO)
			{
				@autoreleasepool
				{
					NSThread *thread = [[[NSThread alloc] init] autorelease];
					[thread start];
				}
			}
			return [NSThread isMultiThreaded] != NO;		
		}

		void PerformOperationInsidePool(DKOperation* op)
		{
			NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
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

	DKGL_API uint32_t DKNumberOfCpuCores(void)
	{
		static int ncpu = []()
		{
			int num = 0;
			size_t len = sizeof(num);
			if (!sysctlbyname("hw.physicalcpu", &num, &len, nullptr, 0))
				return num;
			if (!sysctlbyname("hw.physicalcpu_max", &num, &len, nullptr, 0))
				return num;
			return 1;
		}();

		if (ncpu >= 1)
			return ncpu;
		return 1;
	}

	DKGL_API uint32_t DKNumberOfProcessors(void)
	{
		static int ncpu = []()
		{
			int num = 0;
			int mib[2] = { CTL_HW, HW_AVAILCPU };
			size_t len = sizeof(num);
			if (!sysctl(mib, 2, &num, &len, NULL, 0))
				return num;
			mib[1] = HW_NCPU;
			if (!sysctl(mib, 2, &num, &len, NULL, 0))
				return num;
			if (!sysctlbyname("hw.logicalcpu", &num, &len, nullptr, 0))
				return num;
			if (!sysctlbyname("hw.logicalcpu_max", &num, &len, nullptr, 0))
				return num;
			return 1;
		}();

		if (ncpu >= 1)
			return ncpu;
		return 1;
	}
}
#endif //if defined(__APPLE__) && defined(__MACH__)

