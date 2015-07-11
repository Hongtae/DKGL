//
//  File: DKTexture.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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
		// Texture Format
		// 8: unsigned char (normalize to 0.0~1.0)
		// 16: unsigned short (normalize to 0.0~1.0)
		// 32: unsigned int (normalize to 0.0~1.0)
		// 8I: char, 8UI: unsigned char (not normalized)
		// 16I: short, 16UI: unsigned short (not normalized)
		// 32I: int, 32UI: unsigned int (not normalized)
		// 16F, 32F: float
		enum Format
		{
			FormatUnknown = 0,

			// R
			FormatR8,
			FormatR16F,		// float (16bit)
			FormatR32F,		// float (32bit)
			// RG
			FormatRG8,
			FormatRG16F,
			FormatRG32F,
			// RGB
			FormatRGB8,
			FormatRGB16F,
			FormatRGB32F,
			// RGBA
			FormatRGBA8,
			FormatRGBA16F,
			FormatRGBA32F,
			// R (not normalized)
			FormatR8I,
			FormatR8UI,		
			FormatR16I,		
			FormatR16UI,	
			FormatR32I,		
			FormatR32UI,
			// RG (not normalized)
			FormatRG8I,
			FormatRG8UI,
			FormatRG16I,
			FormatRG16UI,
			FormatRG32I,
			FormatRG32UI,
			// RGB (not normalized)
			FormatRGB8I,
			FormatRGB8UI,
			FormatRGB16I,
			FormatRGB16UI,
			FormatRGB32I,
			FormatRGB32UI,
			// RGBA (not normalized)
			FormatRGBA8I,
			FormatRGBA8UI,
			FormatRGBA16I,
			FormatRGBA16UI,
			FormatRGBA32I,
			FormatRGBA32UI,
			// depth format
			FormatDepth16,		// unsigned short, unsigned int
			FormatDepth24,		// unsigned int
			FormatDepth32F,		// float
		};
		enum Type
		{
			TypeByte = 0,
			TypeUnsignedByte,
			TypeShort,
			TypeUnsignedShort,
			TypeInt,
			TypeUnsignedInt,
			TypeFloat16,
			TypeFloat32,
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
