//
//  File: DKTexture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKSize.h"
#include "DKRect.h"
#include "DKResource.h"
#include "DKColor.h"
#include "DKVector3.h"

#define DKL_IS_POWER_OF_TWO(v)		((v & (v-1)) == 0)

////////////////////////////////////////////////////////////////////////////////
// DKTexture
// OpenGL texture interface.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	inline bool IsPowerOfTwo(int value)	{return DKL_IS_POWER_OF_TWO(value);}

	class DKLIB_API DKTexture : public DKResource
	{
		friend class DKRenderTarget;
	public:
		enum Target
		{
			TargetUnknown = 0,
			Target2D,
			Target3D,
			TargetCube,
		};
		enum Format
		{
			FormatUnknown = 0,
			FormatAlpha,
			FormatRGB,
			FormatRGBA,
			FormatDepth16,
			FormatDepth24,
			FormatDepth32,
		};
		enum Type
		{
			TypeSignedByte = 0,
			TypeUnsignedByte,
			TypeSignedShort,
			TypeUnsignedShort,
			TypeSignedInt,
			TypeUnsignedInt,
			TypeFloat,
			TypeDouble,
		};

		DKTexture(void);
		virtual ~DKTexture(void);

		virtual void Bind(void) const;
		virtual bool IsValid(void) const;
	
		Target		ObjectTarget(void) const		{return target;}
		Format		TextureFormat(void) const		{return format;}
		Type		ComponentType(void) const		{return type;}
		int			Width(void) const				{return width;}
		int			Height(void) const				{return height;}
		int			Depth(void) const				{return depth;}
		size_t		NumberOfComponents(void) const	{return components;}

		DKSize		Resolution(void) const; // 2d resolution (width x height)
		DKVector3	Dimensions(void) const; // 3d pixel volume (width x height x depth)
		size_t		BytesPerPixel(void) const;

		// DKResource::Validate() override
		bool Validate(void)					{return IsValid();}
		bool IsColorTexture(void) const;
		bool IsDepthTexture(void) const;

		unsigned int TextureId(void) const	{return resourceId;}		// OpenGL resource id

		DKFoundation::DKObject<DKSerializer> Serializer(void);

		static int MaxTextureSize(void);

	protected:
		DKTexture(Target);

		unsigned int	resourceId;

		int				width;
		int				height;
		int				depth;  // depth of 3d texture, otherwise 1. (2d,cube is 1)
		size_t			components;
		Format			format;
		Type			type;

	private:
		const Target	target;		// tex2D or Cube .. etc
	};
}
