//
//  File: DKVoxelVolume.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2009-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKVoxelVolume
// abstract class, interface for voxel volume.
// subclass should override functions that read, write to voxel.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	template <typename VoxelType> class DKVoxelVolume
	{
	public:
		virtual bool GetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, VoxelType& v) = 0;
		virtual bool SetVoxelAtLocation(unsigned int x, unsigned int y, unsigned int z, const VoxelType& v) = 0;

		virtual void GetDimensions(size_t* width, size_t* height, size_t* depth) = 0;
		virtual bool SetDimensions(size_t width, size_t height, size_t depth) = 0;

		virtual ~DKVoxelVolume(void) {}

	protected:
		DKVoxelVolume(void) {}

	private:
		DKVoxelVolume(const DKVoxelVolume&);
		DKVoxelVolume& operator = (const DKVoxelVolume&);
	};
}
