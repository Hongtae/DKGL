//
//  File: DKVoxel32SparseVolume.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKMath.h"
#include "DKVoxel32SparseVolume.h"

using namespace DKGL;
namespace DKGL
{
	namespace Private
	{
		struct Voxel32MemoryStorage : public DKVoxel32Storage
		{
			enum {Blocks = DKVoxel32SparseVolume::UnitDimensions};
			typedef DKMap<StorageId, DKVoxel32*> StorageMap;

			Voxel32MemoryStorage(void)
			{
			}
			~Voxel32MemoryStorage(void)
			{
				auto deleteChunk = [](StorageMap::Pair& p)
				{
					DKLOG_DEBUG("Voxel32MemoryStorage: Delete SID(%ls)\n", (const wchar_t*)p.key.String());
					delete[] p.value;
				};

				inactiveChunks.EnumerateForward(deleteChunk);
				activeChunks.EnumerateForward(deleteChunk);
			}
			void LogStatus(void)
			{
				uint64_t active = activeChunks.Count();
				uint64_t total = active + inactiveChunks.Count();
				uint64_t activeSize = active * Blocks;
				uint64_t totalSize = total * Blocks;
				DKLOG_DEBUG("Voxel32MemoryStorage Usage: %llu / %llu (%llu / %llu)\n", active, total, activeSize, totalSize);
			}
			size_t MaxActiveUnits(void) const
			{
				return (size_t)-1;
			}
			DKVoxel32* Create(const StorageId& sid)
			{
				DKASSERT_DEBUG(activeChunks.Find(sid) == NULL);
				DKASSERT_DEBUG(inactiveChunks.Find(sid) == NULL);

				DKVoxel32* data = new DKVoxel32[Blocks];
				bool b = activeChunks.Insert(sid, data);
				DKASSERT_DEBUG(b);

				DKLOG_DEBUG("Voxel32MemoryStorage: Create SID(%ls)\n", (const wchar_t*)sid.String());
				LogStatus();
				return data;
			}
			void Delete(const StorageId& sid)
			{
				StorageMap::Pair* p = NULL;
				p = inactiveChunks.Find(sid);
				if (p)
				{
					delete[] p->value;
					inactiveChunks.Remove(sid);
				}
				else
				{
					p = activeChunks.Find(sid);
					DKASSERT_DEBUG(p);
					delete[] p->value;
					activeChunks.Remove(sid);
				}
				DKLOG_DEBUG("Voxel32MemoryStorage: Delete SID(%ls)\n", (const wchar_t*)sid.String());
				LogStatus();
			}
			DKVoxel32* Load(const StorageId& sid)
			{
				auto p = inactiveChunks.Find(sid);
				DKASSERT_DEBUG(p);
				DKVoxel32* data = p->value;
				bool b = activeChunks.Insert(sid, data);
				DKASSERT_DEBUG(b);
				inactiveChunks.Remove(sid);

				DKLOG_DEBUG("Voxel32MemoryStorage: Load SID(%ls)\n", (const wchar_t*)sid.String());
				LogStatus();
				return data;
			}
			void Unload(const StorageId& sid)
			{
				auto p = activeChunks.Find(sid);
				DKASSERT_DEBUG(p);
				bool b = inactiveChunks.Insert(sid, p->value);
				DKASSERT_DEBUG(b);
				activeChunks.Remove(sid);

				DKLOG_DEBUG("Voxel32MemoryStorage: Unload SID(%ls)\n", (const wchar_t*)sid.String());
				LogStatus();
			}
			StorageMap activeChunks;
			StorageMap inactiveChunks;
		};
		template <typename T> inline T VolumeSizeDiv(T s, T unit)
		{
			return ((s / unit) + (s%unit ? 1 : 0));
		}
		template <typename T> inline T LocationIndex(T x, T y, T z, T w, T h, T d)
		{
			return (x + (w*y) + (w*h*z));
		}
	}
}
using namespace DKGL;
using namespace DKGL::Private;

#define MINIMUM_LOADED_BLOCKS	2

DKVoxel32SparseVolume::DKVoxel32SparseVolume(Storage* stor)
	: blocksLoaded(0)
	, width(0), height(0), depth(0)
	, storage(stor)
	, volumeBlocks(NULL)
	, lastCompactedTS(0)
{
	if (storage == NULL)
		storage = DKObject<Private::Voxel32MemoryStorage>::New();
}

DKVoxel32SparseVolume::~DKVoxel32SparseVolume(void)
{
	if (blocksLoaded > 0)
	{
		size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
		size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
		size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);
		size_t numBlocks = w * h * d;

		for (size_t i = 0; i < numBlocks; ++i)
		{
			VolumetricBlock& b = volumeBlocks[i];
			if (b.storageId.IsZero()) continue;
			if (b.voxels)
			{
				storage->Unload(b.storageId);
				blocksLoaded--;
			}
		}
	}
	DKASSERT_DEBUG(blocksLoaded == 0);

	if (volumeBlocks)
		delete[] volumeBlocks;
	storage = NULL;
}

bool DKVoxel32SparseVolume::ResetContents(void)
{
	DKCriticalSection<DKSharedLock> guard(volumeLock);	// exclusive lock

	size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
	size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
	size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);
	size_t numBlocks = w * h * d;
	for (size_t i = 0; i < numBlocks; ++i)
	{
		VolumetricBlock& b = volumeBlocks[i];
		if (!b.storageId.IsZero())
		{
			storage->Delete(b.storageId);
			b.storageId.SetZero();
		}
		b.voxels = NULL;
		b.ts = 0;
	}
	blocksLoaded = 0;

	lastCompactedTS = 0;
	return true;
}

DKVoxel32SparseVolume::Storage* DKVoxel32SparseVolume::VolumeStorage(void)
{
	return storage;
}

const DKVoxel32SparseVolume::Storage* DKVoxel32SparseVolume::VolumeStorage(void) const
{
	return storage;
}

bool DKVoxel32SparseVolume::GetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, DKVoxel32& v)
{
	DKSharedLockReadOnlySection guard(volumeLock);
	if (x < width && y < height && z < depth)
	{
		size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
		size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
		size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);

		unsigned int bx = x / UnitSize;
		unsigned int by = y / UnitSize;
		unsigned int bz = z / UnitSize;

		size_t idx = LocationIndex<size_t>(bx, by, bz, w, h, d);
		VolumetricBlock& block = volumeBlocks[idx];

		DKCriticalSection<DKSpinLock> blockGuard(block.lock);
		if (block.storageId.IsZero())
		{
			v = block.solid;
		}
		else
		{
			if (block.voxels == NULL)
			{
				size_t maxLoadable = Max<size_t>(storage->MaxActiveUnits(), 1);

				if (blocksLoaded >= maxLoadable)
					UnloadOldBlocks(maxLoadable - blocksLoaded + 1);

				block.voxels = storage->Load(block.storageId);
				blocksLoaded++;
			}
			size_t idx2 = LocationIndex<size_t>(x % UnitSize, y % UnitSize, z % UnitSize, UnitSize, UnitSize, UnitSize);
			v = block.voxels[idx2];
			block.ts = DKTimer::SystemTick();
		}
		return true;
	}
	return false;
}

bool DKVoxel32SparseVolume::SetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, const DKVoxel32& v)
{
	DKSharedLockReadOnlySection guard(volumeLock);
	if (x < width && y < height && z < depth)
	{
		size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
		size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
		size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);

		unsigned int bx = x / UnitSize;
		unsigned int by = y / UnitSize;
		unsigned int bz = z / UnitSize;

		size_t idx = LocationIndex<size_t>(bx, by, bz, w, h, d);
		size_t idx2 = LocationIndex<size_t>(x % UnitSize, y % UnitSize, z % UnitSize, UnitSize, UnitSize, UnitSize);
		const size_t numVoxels = UnitSize * UnitSize * UnitSize;

		size_t maxLoadable = Max<size_t>(storage->MaxActiveUnits(), 1);

		VolumetricBlock& block = volumeBlocks[idx];

		DKCriticalSection<DKSpinLock> blockGuard(block.lock);
		if (block.storageId.IsZero())
		{
			if (block.solid.uintValue != v.uintValue)
			{
				if (blocksLoaded >= maxLoadable)
					UnloadOldBlocks(maxLoadable - blocksLoaded + 1);

				DKVoxel32 solid = block.solid;
				block.storageId = DKUUID::Create();
				block.voxels = storage->Create(block.storageId);
				DKASSERT_DEBUG(block.voxels);
				for (size_t i = 0; i < numVoxels; ++i)
				{
					block.voxels[i] = solid;
				}
				block.voxels[idx2] = v;
				block.ts = DKTimer::SystemTick();
				blocksLoaded++;
			}
		}
		else
		{
			if (block.voxels == NULL)
			{
				if (blocksLoaded >= maxLoadable)
					UnloadOldBlocks(maxLoadable - blocksLoaded + 1);

				block.voxels = storage->Load(block.storageId);
				blocksLoaded++;
			}
			DKASSERT_DEBUG(block.voxels);

			block.voxels[idx2] = v;
			block.ts = DKTimer::SystemTick();
		}
		return true;
	}
	return false;
}

void DKVoxel32SparseVolume::GetDimensions(size_t* w, size_t* h, size_t* d)
{
	DKSharedLockReadOnlySection guard(volumeLock);
	if (w)		*w = this->width;
	if (h)		*h = this->height;
	if (d)		*d = this->depth;
}

bool DKVoxel32SparseVolume::SetDimensions(size_t w, size_t h, size_t d)
{
	DKSharedLockReadOnlySection guard(volumeLock);

	size_t w1 = VolumeSizeDiv<size_t>(width, UnitSize);
	size_t h1 = VolumeSizeDiv<size_t>(height, UnitSize);
	size_t d1 = VolumeSizeDiv<size_t>(depth, UnitSize);
	size_t w2 = VolumeSizeDiv<size_t>(w, UnitSize);
	size_t h2 = VolumeSizeDiv<size_t>(h, UnitSize);
	size_t d2 = VolumeSizeDiv<size_t>(d, UnitSize);

	if (w1 != w2 || h1 != h2 || d1 != d2)
	{
		size_t numBlocks1 = w1 * h1 * d1;
		size_t numBlocks2 = w2 * h2 * d2;

		if (numBlocks1 > 0 && numBlocks2 > 0)
		{
			VolumetricBlock* volumeBlocks2 = new VolumetricBlock[numBlocks2];

			for (size_t z = 0; z < d2; ++z)
			{
				for (size_t y = 0; y < h2; ++y)
				{
					for (size_t x = 0; x < w2; ++x)
					{
						size_t idx = LocationIndex<size_t>(x,y,z,w2,h2,d2);
						VolumetricBlock& b = volumeBlocks2[idx];
						b.ts = 0;

						if (x < w1 && y < h1 && z < d1)
						{
							size_t idx2 = LocationIndex<size_t>(x,y,z,w1,h1,d1);
							VolumetricBlock& b2 = volumeBlocks[idx2];
							b.storageId = b2.storageId;
							b.voxels = b2.voxels;
							b.ts = DKTimer::SystemTick();

							b2.storageId.SetZero();
							b2.voxels = NULL;
						}
						else
						{
							b.storageId.SetZero();
							b.voxels = NULL;
						}
					}
				}
			}
			for (size_t i = 0; i < numBlocks1; ++i)
			{
				VolumetricBlock& b = volumeBlocks[i];
				if (b.storageId.IsZero()) continue;
				if (b.voxels)
				{
					blocksLoaded--;
				}
				storage->Delete(b.storageId);
			}
			delete[] volumeBlocks;
			volumeBlocks = volumeBlocks2;
		}
		else if (numBlocks1 > 0)
		{
			for (size_t i = 0; i < numBlocks1; ++i)
			{
				VolumetricBlock& b = volumeBlocks[i];
				if (b.storageId.IsZero()) continue;
				if (b.voxels)
				{
					blocksLoaded--;
				}
				storage->Delete(b.storageId);
			}
			delete[] volumeBlocks;
			volumeBlocks = NULL;
		}
		else if (numBlocks2 > 0)
		{
			DKASSERT_DEBUG(volumeBlocks == NULL);
			volumeBlocks = new VolumetricBlock[numBlocks2];
			for (size_t i = 0; i < numBlocks2; ++i)
			{
				volumeBlocks[i].storageId.SetZero();
				volumeBlocks[i].voxels = NULL;
				volumeBlocks[i].ts = 0;
			}
		}
	}
	width = w;
	height = h;
	depth = d;
	return true;
}

void DKVoxel32SparseVolume::Compact(void)
{
	DKSharedLockReadOnlySection guard(volumeLock);

	size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
	size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
	size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);
	size_t numBlocks = w * h * d;
	VolumetricBlock* blocks = this->volumeBlocks;

	const size_t numVoxels = UnitSize * UnitSize * UnitSize;

	TimeStamp ts = DKTimer::SystemTick();

	for (size_t i = 0; i < numBlocks; ++i)
	{
		VolumetricBlock& block = this->volumeBlocks[i];

		DKCriticalSection<DKSpinLock> blockGuard(block.lock);
		if (block.storageId.IsZero())
			continue;
		if (block.voxels && block.ts >= lastCompactedTS)
		{
			DKVoxel32 firstValue = block.voxels[0];
			DKVoxel32 lastValue = block.voxels[numVoxels-1];
			DKVoxel32 middleValue = block.voxels[numVoxels/2];

			bool solid = firstValue.uintValue == lastValue.uintValue;
			if (solid)
				solid = firstValue.uintValue == middleValue.uintValue;

			// if solid is true, check all voxel's values are equal.
			for (size_t n = 1; n < numVoxels && solid ; ++n)
			{
				if (block.voxels[n].uintValue != firstValue.uintValue)
					solid = false;
			}
			if (solid) // every voxel's values are equal. change type to solid-block
			{
				storage->Delete(block.storageId);
				block.storageId.SetZero();
				block.ts = 0;
				block.solid = firstValue;
			}
		}
	}
	lastCompactedTS = ts;
}

void DKVoxel32SparseVolume::UnloadOldBlocks(size_t num)
{
	DKSharedLockReadOnlySection guard(volumeLock);

	if (blocksLoaded == 0)
		return;

	size_t w = VolumeSizeDiv<size_t>(width, UnitSize);
	size_t h = VolumeSizeDiv<size_t>(height, UnitSize);
	size_t d = VolumeSizeDiv<size_t>(depth, UnitSize);
	size_t numBlocks = w * h * d;

	if (num > 0 && numBlocks > 0 )
	{
		if (numBlocks > num)	// sort ascending by timestamp
		{
			typedef VolumetricBlock* Value;
			auto sortByTSDesc = [](VolumetricBlock* const & lhs, VolumetricBlock* const & rhs)->bool
			{
				return lhs->ts > rhs->ts;
			};

			DKOrderedArray<VolumetricBlock*> blocks(sortByTSDesc);
			blocks.Reserve(numBlocks);
			for (size_t i = 0; i < numBlocks; ++i)
			{
				VolumetricBlock& b = this->volumeBlocks[i];
				DKCriticalSection<DKSpinLock> blockGuard(b.lock);
				if (b.storageId.IsZero())
					continue;
				if (b.voxels)
					blocks.Insert(&b);
			}

			size_t n = Min(num, blocks.Count());
			for (size_t i = 0; i < n; ++i)
			{
				VolumetricBlock* p = blocks.Value(i);
				DKCriticalSection<DKSpinLock> blockGuard(p->lock);
				DKASSERT_DEBUG(p->voxels);
				storage->Unload(p->storageId);
				p->voxels = NULL;
				blocksLoaded--;
			}
		}
		else		// volumeBlocks is smaller, unload all
		{
			for (size_t i = 0; i < numBlocks; ++i)
			{
				VolumetricBlock& b = this->volumeBlocks[i];
				DKCriticalSection<DKSpinLock> blockGuard(b.lock);
				if (b.storageId.IsZero())
					continue;
				if (b.voxels)
				{
					storage->Unload(b.storageId);
					b.voxels = NULL;
					blocksLoaded--;
				}
			}
		}
	}
}
