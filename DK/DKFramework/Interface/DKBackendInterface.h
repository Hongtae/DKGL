//
//  File: DKBackendInterface.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../../DKFoundation.h"

namespace DKFramework
{
	/// @brief Base class for platform dependent interface classes.
	class DKBackendInterface
	{
	protected:
		DKBackendInterface(void) {}

	public:
		virtual ~DKBackendInterface(void) {}

		static void* operator new (size_t s) { return DKMalloc(s); }
		static void operator delete (void* p) { DKFree(p); }
	};
}
