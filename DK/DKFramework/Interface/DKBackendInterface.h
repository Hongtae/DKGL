//
//  File: DKBackendInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKFoundation.h"

namespace DKGL
{
	class DKBackendInterface
	{
	protected:
		DKBackendInterface(void) {}

	public:
		virtual ~DKBackendInterface(void) {}

		static void* operator new (size_t s)
		{
			return DKAllocator::DefaultAllocator().Alloc(s);
		}
		static void operator delete (void* p)
		{
			DKAllocator::DefaultAllocator().Dealloc(p);
		}
	};
}
