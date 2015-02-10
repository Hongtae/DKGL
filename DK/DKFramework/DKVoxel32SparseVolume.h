//
//  File: DKVoxel32SparseVolume.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVoxelVolume.h"
#include "DKVoxel32Storage.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxel32SparseVolume
// voxel data storage interface, manages voxel with sparse volume.
// this class uses DKVoxel32 as data unit for voxel.
// You need to provide storage object, which can be file or memory that can
// store voxel data. (see DKVoel32Storage.h)
//
// Note:
//   If you want to polygonize voxels, see DKVoxelPolygonizer.h
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKVoxel32SparseVolume : public DKVoxelVolume<DKVoxel32>
	{
	public:
		typedef DKVoxel32Storage Storage;
		typedef Storage::StorageId StorageId;
		typedef DKFoundation::DKTimer::Tick TimeStamp;

		enum {UnitSize = 16};
		enum {UnitDimensions = UnitSize * UnitSize * UnitSize};
		enum {UnitBytes = UnitDimensions * sizeof(DKVoxel32)};

		DKVoxel32SparseVolume(Storage* stor = NULL);
		virtual ~DKVoxel32SparseVolume(void);

		bool GetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, DKVoxel32& v);
		bool SetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, const DKVoxel32& v);

		void GetDimensions(size_t* width, size_t* height, size_t* depth);
		bool SetDimensions(size_t width, size_t height, size_t depth);

		bool ResetContents(void);

		Storage* VolumeStorage(void);
		const Storage* VolumeStorage(void) const;

		void Compact(void);

	private:
		struct VolumetricBlock
		{
			DKFoundation::DKSpinLock lock;
			StorageId storageId;			// zero for solid-block
			TimeStamp ts;
			union
			{
				DKVoxel32 solid;
				DKVoxel32* voxels;
			};
		};

		size_t width;
		size_t height;
		size_t depth;

		size_t blocksLoaded;				// number of blocks loaded.
		VolumetricBlock* volumeBlocks;		// all blocks

		DKFoundation::DKObject<Storage> storage;
		DKFoundation::DKSharedLock volumeLock;	// rw-lock for changing volume.
		TimeStamp lastCompactedTS;

		void UnloadOldBlocks(size_t);
	};
}
