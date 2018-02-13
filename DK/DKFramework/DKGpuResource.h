//
//  File: DKGpuResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

namespace DKFramework
{
	enum DKCpuCacheMode
	{
		DKCpuCacheModeDefault = 0,
		DKCpuCacheModeWriteCombined,
	};
	enum DKGpuStorageMode
	{
		DKGpuStorageShared = 0,
		DKGpuStorageManaged,
		DKGpuStoragePrivate,
		DKGpuStorageMemoryless,
	};
}
