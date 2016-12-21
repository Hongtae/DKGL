//
//  File: DKTextureSampler.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKColor.h"

namespace DKFramework
{
	/// @brief 
	///  texture sampler class. sampler object can define how texture being sampled.
	/// @details
	///  sampler object can be shared between textures, materials.
	class DKGL_API DKTextureSampler : public DKResource
	{
	public:
		enum MinificationFilter
		{
			MinFilterUndefined = 0,
			MinFilterNearest,
			MinFilterLinear,
			MinFilterNearestMipmapNearest,
			MinFilterNearestMipmapLinear,
			MinFilterLinearMipmapNearest,
			MinFilterLinearMipmapLinear,
		};
		enum MagnificationFilter
		{
			MagFilterUndefined = 0,
			MagFilterNearest,
			MagFilterLinear,
		};
		enum Wrap
		{
			WrapUndefined = 0,
			WrapRepeat,
			WrapMirroredRepeat,
			WrapClampToEdge,
		};
		enum Compare
		{
			CompareNever,
			CompareAlways,
			CompareLess,
			CompareGreater,
			CompareEqual,
			CompareNotEqual,
			CompareLessOrEqual,
			ComapreGreaterOrEqual,
		};

		DKTextureSampler(void);
		~DKTextureSampler(void);

		MinificationFilter		minFilter;
		MagnificationFilter		magFilter;
		Wrap					wrap[3];
		float					LODBias;

		DKObject<DKSerializer> Serializer(void) override;
	};
}
