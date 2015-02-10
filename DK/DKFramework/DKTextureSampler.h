//
//  File: DKTextureSampler.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKColor.h"
#include "DKTexture.h"

///////////////////////////////////////////////////////////////////////////////
// DKTextureSampler
// texture sampler class. sampler object can define how texture being sampled.
// sampler object can be shared between textures, materials.
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKTextureSampler : public DKResource
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

		void Bind(DKTexture::Target) const;

		MinificationFilter		minFilter;
		MagnificationFilter		magFilter;
		Wrap					wrap[3];
		float					LODBias;

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;
	};
}
