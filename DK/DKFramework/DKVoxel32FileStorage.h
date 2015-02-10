//
//  File: DKVoxel32FileStorage.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVoxel32Storage.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxel32FileStorage
// a 32bit voxel storage, it store data into file.
//
// THIS CLASS NOT IMPLEMENTED YET.
// Use DKVoxel32SparseVolume instead.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	// NOT IMPLEMENTED. (Use DKVoxel32SparseVolume)
	class DKLIB_API DKVoxel32FileStorage : public DKVoxel32Storage
	{
	public:
		DKVoxel32FileStorage(void);
		~DKVoxel32FileStorage(void);

		DKVoxel32* Create(const StorageId&);
		void Delete(const StorageId&);
		DKVoxel32* Load(const StorageId&);
		void Unload(const StorageId&);

		size_t MaxActiveUnits(void) const {return maxLoadableUnits;}

	protected:
		size_t maxLoadableUnits;
	};
}
