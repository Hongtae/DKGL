//
//  File: Types.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2016-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../GraphicsAPI.h"
#if DKGL_ENABLE_VULKAN
#include <vulkan/vulkan.h>
#include "../../../Libs/SPIRV-Cross/src/spirv_cross.hpp"
#include "../../DKShaderFunction.h"

namespace DKFramework::Private::Vulkan
{

	DKShaderDataType ShaderDataTypeFromSPIRType(const spirv_cross::SPIRType& spType)
	{
		DKShaderDataType dataType = DKShaderDataType::Unknown;
		// get item type
		switch (spType.basetype)
		{
		case spirv_cross::SPIRType::Void:
			dataType = DKShaderDataType::None;
			break;
		case spirv_cross::SPIRType::Struct:
			dataType = DKShaderDataType::Struct;
			break;
		case spirv_cross::SPIRType::Image:
		case spirv_cross::SPIRType::SampledImage:
			dataType = DKShaderDataType::Texture;
			break;
		case spirv_cross::SPIRType::Sampler:
			dataType = DKShaderDataType::Sampler;
			break;
		case spirv_cross::SPIRType::Boolean:
			switch (spType.vecsize)
			{
			case 2:		dataType = DKShaderDataType::Bool2;		break;
			case 3:		dataType = DKShaderDataType::Bool3;		break;
			case 4:		dataType = DKShaderDataType::Bool4;		break;
			default:	dataType = DKShaderDataType::Bool;		break;
			}
			break;
		case spirv_cross::SPIRType::Char:
			switch (spType.vecsize)
			{
			case 2:		dataType = DKShaderDataType::Char2;		break;
			case 3:		dataType = DKShaderDataType::Char3;		break;
			case 4:		dataType = DKShaderDataType::Char4;		break;
			default:	dataType = DKShaderDataType::Char;		break;
			}
			break;
		case spirv_cross::SPIRType::Int:
		case spirv_cross::SPIRType::Int64:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::Short2;	break;
				case 3:		dataType = DKShaderDataType::Short3;	break;
				case 4:		dataType = DKShaderDataType::Short4;	break;
				default:	dataType = DKShaderDataType::Short;		break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::Int2;		break;
				case 3:		dataType = DKShaderDataType::Int3;		break;
				case 4:		dataType = DKShaderDataType::Int4;		break;
				default:	dataType = DKShaderDataType::Int;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Int %d bit)", spType.width);
			}
			break;
		case spirv_cross::SPIRType::UInt:
		case spirv_cross::SPIRType::UInt64:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::UShort2;	break;
				case 3:		dataType = DKShaderDataType::UShort3;	break;
				case 4:		dataType = DKShaderDataType::UShort4;	break;
				default:	dataType = DKShaderDataType::UShort;	break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:		dataType = DKShaderDataType::UInt2;		break;
				case 3:		dataType = DKShaderDataType::UInt3;		break;
				case 4:		dataType = DKShaderDataType::UInt4;		break;
				default:	dataType = DKShaderDataType::UInt;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Int %d bit)", spType.width);
			}
			break;
		case spirv_cross::SPIRType::Half:
		case spirv_cross::SPIRType::Float:
		case spirv_cross::SPIRType::Double:
			if (spType.width == 16)
			{
				switch (spType.vecsize)
				{
				case 2:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half2x2;	break;
					case 3:		dataType = DKShaderDataType::Half2x3;	break;
					case 4:		dataType = DKShaderDataType::Half2x4;	break;
					default:	dataType = DKShaderDataType::Half2;		break;
					}
					break;
				case 3:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half3x2;	break;
					case 3:		dataType = DKShaderDataType::Half3x3;	break;
					case 4:		dataType = DKShaderDataType::Half3x4;	break;
					default:	dataType = DKShaderDataType::Half3;		break;
					}
					break;
				case 4:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Half4x2;	break;
					case 3:		dataType = DKShaderDataType::Half4x3;	break;
					case 4:		dataType = DKShaderDataType::Half4x4;	break;
					default:	dataType = DKShaderDataType::Half4;		break;
					}
					break;
				default:
					dataType = DKShaderDataType::Half;		break;
				}
			}
			else if (spType.width == 32)
			{
				switch (spType.vecsize)
				{
				case 2:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float2x2;	break;
					case 3:		dataType = DKShaderDataType::Float2x3;	break;
					case 4:		dataType = DKShaderDataType::Float2x4;	break;
					default:	dataType = DKShaderDataType::Float2;	break;
					}
					break;
				case 3:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float3x2;	break;
					case 3:		dataType = DKShaderDataType::Float3x3;	break;
					case 4:		dataType = DKShaderDataType::Float3x4;	break;
					default:	dataType = DKShaderDataType::Float3;	break;
					}
					break;
				case 4:
					switch (spType.columns)
					{
					case 2:		dataType = DKShaderDataType::Float4x2;	break;
					case 3:		dataType = DKShaderDataType::Float4x3;	break;
					case 4:		dataType = DKShaderDataType::Float4x4;	break;
					default:	dataType = DKShaderDataType::Float4;	break;
					}
					break;
				default:
					dataType = DKShaderDataType::Float;		break;
				}
			}
			else
			{
				DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type! (Float %d bit)", spType.width);
			}
			break;
		default:
			DKLogE("ERROR: DKShaderModule Unsupported stage input attribute type!");
		}
		return dataType;
	}
}
#endif
