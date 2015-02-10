//
//  File: DKVoxel32Storage.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVoxelVolume.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxel32Storage
// abstract class, interface for 32bit voxel storage.
// using UUID for storage-id, you need to subclass to override load, unload
// data from your storage that can be file or memory or anything else.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	union DKVoxel32 // 32bit voxel
	{
		struct
		{
			unsigned char level;   // iso surface level
			unsigned char data[3]; // custom data (can be color or texture uv, etc.)
		};
		unsigned int uintValue;
	};

	class DKLIB_API DKVoxel32Storage
	{
	public:
		DKVoxel32Storage(void) {}
		virtual ~DKVoxel32Storage(void) {}

		typedef DKFoundation::DKUUID StorageId;
	
		virtual size_t MaxActiveUnits(void) const = 0;

		virtual DKVoxel32* Create(const StorageId&) = 0;
		virtual void Delete(const StorageId&) = 0;
		virtual DKVoxel32* Load(const StorageId&) = 0;
		virtual void Unload(const StorageId&) = 0;
	};
}
