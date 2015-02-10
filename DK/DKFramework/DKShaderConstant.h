//
//  File: DKShaderConstant.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"


////////////////////////////////////////////////////////////////////////////////
// DKSahderConstant
// predefined shader constant (uniform)
// predefined value will be applied to entire scene. each model or material
// can be overriden.
// You can use UniformUserDefined type for your custom value.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	struct DKShaderConstant
	{
		enum Uniform : unsigned char
		{
			UniformUnknown = 0,								// error
			UniformModelMatrix,								// float4x4	
			UniformModelMatrixInverse,						// float4x4	
			UniformViewMatrix,								// float4x4	
			UniformViewMatrixInverse,						// float4x4	
			UniformProjectionMatrix,						// float4x4	
			UniformProjectionMatrixInverse,					// float4x4	
			UniformViewProjectionMatrix,					// float4x4	
			UniformViewProjectionMatrixInverse,				// float4x4	
			UniformModelViewMatrix,							// float4x4	
			UniformModelViewMatrixInverse,					// float4x4	
			UniformModelViewProjectionMatrix,				// float4x4	
			UniformModelViewProjectionMatrixInverse,		// float4x4	
			UniformLinearTransformArray,					// float3x3	
			UniformAffineTransformArray,					// float4x4	
			UniformPositionArray,							// float3	
			UniformTexture2D,								// uint1	
			UniformTexture3D,								// uint1	
			UniformTextureCube,								// uint1	
			UniformDirectionalLightColor,					// float3	
			UniformDirectionalLightDirection,				// float3	
			UniformPointLightColor,							// float3	
			UniformPointLightPosition,						// float3
			UniformPointLightAttenuation,					// float3	(x=const, y=linear, z=quadratic)
			UniformAmbientColor,							// float3
			UniformCameraPosition,							// float3
			UniformUserDefine,								// user define
			UniformMax,
		};
		enum Type : unsigned char
		{
			TypeUnknown = 0,
			TypeFloat1,
			TypeFloat2,
			TypeFloat3,
			TypeFloat4,
			TypeInt1,
			TypeInt2,
			TypeInt3,
			TypeInt4,
			TypeBool1,
			TypeBool2,
			TypeBool3,
			TypeBool4,
			TypeFloat2x2,
			TypeFloat3x3,
			TypeFloat4x4,
			TypeSampler2D,
			TypeSamplerCube,
			TypeMax,
		};
		enum BaseType : unsigned char
		{
			BaseTypeUnknown = 0,
			BaseTypeBoolean,
			BaseTypeInteger,
			BaseTypeFloating,
			BaseTypeSampler,
		};

		DKFoundation::DKString	name;			// value name (should be matched with defined on code)
		Uniform 				id;				// Uniform id
		Type					type;			// value type
		size_t					components;		// value components
		int						location;		// binding location of program module

		static inline BaseType GetBaseType(Type t)
		{
			switch (t)
			{
			case TypeFloat1:
			case TypeFloat2:
			case TypeFloat3:
			case TypeFloat4:
				return BaseTypeFloating;
			case TypeInt1:
			case TypeInt2:
			case TypeInt3:
			case TypeInt4:
				return BaseTypeInteger;
			case TypeBool1:
			case TypeBool2:
			case TypeBool3:
			case TypeBool4:
				return BaseTypeBoolean;
			case TypeFloat2x2:
			case TypeFloat3x3:
			case TypeFloat4x4:
				return BaseTypeFloating;
			case TypeSampler2D:
			case TypeSamplerCube:
				return BaseTypeSampler;
			default:
				break;
			}
			return BaseTypeUnknown;
		}

		static inline DKFoundation::DKString UniformToString(Uniform u)
		{
			switch (u)
			{
			case UniformModelMatrix:							return L"ModelMatrix";
			case UniformModelMatrixInverse:						return L"ModelMatrixInverse";
			case UniformViewMatrix:								return L"ViewMatrix";
			case UniformViewMatrixInverse:						return L"ViewMatrixInverse";
			case UniformProjectionMatrix:						return L"ProjectionMatrix";
			case UniformProjectionMatrixInverse:				return L"ProjectionMatrixInverse";
			case UniformViewProjectionMatrix:					return L"ViewProjectionMatrix";
			case UniformViewProjectionMatrixInverse:			return L"ViewProjectionMatrixInverse";
			case UniformModelViewMatrix:						return L"ModelViewMatrix";
			case UniformModelViewMatrixInverse:					return L"ModelViewMatrixInverse";
			case UniformModelViewProjectionMatrix:				return L"ModelViewProjectionMatrix";
			case UniformModelViewProjectionMatrixInverse:		return L"ModelViewProjectionMatrixInverse";
			case UniformLinearTransformArray:					return L"LinearTransformArray";
			case UniformAffineTransformArray:					return L"AffineTransformArray";
			case UniformPositionArray:							return L"PositionArray";
			case UniformTexture2D:								return L"Texture2D";
			case UniformTexture3D:								return L"Texture3D";
			case UniformTextureCube:							return L"TextureCube";
			case UniformDirectionalLightColor:					return L"DirectionalLightColor";
			case UniformDirectionalLightDirection:				return L"DirectionalLightDirection";
			case UniformPointLightColor:						return L"PointLightColor";
			case UniformPointLightPosition:						return L"PointLightPosition";
			case UniformPointLightAttenuation:					return L"PointLightAttenuation";
			case UniformAmbientColor:							return L"AmbientColor";
			case UniformCameraPosition:							return L"CameraPosition";
			case UniformUserDefine:								return L"UserDefine";
			default:
				break;
			}
			return L"Unknown";
		}
		static inline Uniform StringToUniform(const DKFoundation::DKString& str)
		{
			if		(!str.CompareNoCase(UniformToString(UniformUnknown)))								return UniformUnknown;
			else if (!str.CompareNoCase(UniformToString(UniformModelMatrix)))							return UniformModelMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformModelMatrixInverse)))					return UniformModelMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformViewMatrix)))							return UniformViewMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformViewMatrixInverse)))						return UniformViewMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformProjectionMatrix)))						return UniformProjectionMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformProjectionMatrixInverse)))				return UniformProjectionMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformViewProjectionMatrix)))					return UniformViewProjectionMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformViewProjectionMatrixInverse)))			return UniformViewProjectionMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformModelViewMatrix)))						return UniformModelViewMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformModelViewMatrixInverse)))				return UniformModelViewMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformModelViewProjectionMatrix)))				return UniformModelViewProjectionMatrix;
			else if (!str.CompareNoCase(UniformToString(UniformModelViewProjectionMatrixInverse)))		return UniformModelViewProjectionMatrixInverse;
			else if (!str.CompareNoCase(UniformToString(UniformLinearTransformArray)))					return UniformLinearTransformArray;
			else if (!str.CompareNoCase(UniformToString(UniformAffineTransformArray)))					return UniformAffineTransformArray;
			else if (!str.CompareNoCase(UniformToString(UniformPositionArray)))							return UniformPositionArray;
			else if (!str.CompareNoCase(UniformToString(UniformTexture2D)))								return UniformTexture2D;
			else if (!str.CompareNoCase(UniformToString(UniformTexture3D)))								return UniformTexture3D;
			else if (!str.CompareNoCase(UniformToString(UniformTextureCube)))							return UniformTextureCube;
			else if (!str.CompareNoCase(UniformToString(UniformDirectionalLightColor)))					return UniformDirectionalLightColor;
			else if (!str.CompareNoCase(UniformToString(UniformDirectionalLightDirection)))				return UniformDirectionalLightDirection;
			else if (!str.CompareNoCase(UniformToString(UniformPointLightColor)))						return UniformPointLightColor;
			else if (!str.CompareNoCase(UniformToString(UniformPointLightPosition)))					return UniformPointLightPosition;
			else if (!str.CompareNoCase(UniformToString(UniformPointLightAttenuation)))					return UniformPointLightAttenuation;
			else if (!str.CompareNoCase(UniformToString(UniformAmbientColor)))							return UniformAmbientColor;
			else if (!str.CompareNoCase(UniformToString(UniformCameraPosition)))						return UniformCameraPosition;
			else if (!str.CompareNoCase(UniformToString(UniformUserDefine)))							return UniformUserDefine;
			return UniformUnknown;
		}

		static inline DKFoundation::DKString TypeToString(Type t)
		{
			switch (t)
			{
			case TypeFloat1:							return L"Float1";
			case TypeFloat2:							return L"Float2";
			case TypeFloat3:							return L"Float3";
			case TypeFloat4:							return L"Float4";
			case TypeInt1:								return L"Int1";
			case TypeInt2:								return L"Int2";
			case TypeInt3:								return L"Int3";
			case TypeInt4:								return L"Int4";
			case TypeBool1:								return L"Bool1";
			case TypeBool2:								return L"Bool2";
			case TypeBool3:								return L"Bool3";
			case TypeBool4:								return L"Bool4";
			case TypeFloat2x2:							return L"Float2x2";
			case TypeFloat3x3:							return L"Float3x3";
			case TypeFloat4x4:							return L"Float4x4";
			case TypeSampler2D:							return L"Sampler2D";
			case TypeSamplerCube:						return L"SamplerCube";
			default:
				break;
			}
			return L"Unknown";
		}
		static inline Type StringToType(const DKFoundation::DKString& str)
		{
			if		(!str.CompareNoCase(TypeToString(TypeFloat1)))							return TypeFloat1;
			else if (!str.CompareNoCase(TypeToString(TypeFloat2)))							return TypeFloat2;
			else if (!str.CompareNoCase(TypeToString(TypeFloat3)))							return TypeFloat3;
			else if (!str.CompareNoCase(TypeToString(TypeFloat4)))							return TypeFloat4;
			else if (!str.CompareNoCase(TypeToString(TypeInt1)))							return TypeInt1;
			else if (!str.CompareNoCase(TypeToString(TypeInt2)))							return TypeInt2;
			else if (!str.CompareNoCase(TypeToString(TypeInt3)))							return TypeInt3;
			else if (!str.CompareNoCase(TypeToString(TypeInt4)))							return TypeInt4;
			else if (!str.CompareNoCase(TypeToString(TypeBool1)))							return TypeBool1;
			else if (!str.CompareNoCase(TypeToString(TypeBool2)))							return TypeBool2;
			else if (!str.CompareNoCase(TypeToString(TypeBool3)))							return TypeBool3;
			else if (!str.CompareNoCase(TypeToString(TypeBool4)))							return TypeBool4;
			else if (!str.CompareNoCase(TypeToString(TypeFloat2x2)))						return TypeFloat2x2;
			else if (!str.CompareNoCase(TypeToString(TypeFloat3x3)))						return TypeFloat3x3;
			else if (!str.CompareNoCase(TypeToString(TypeFloat4x4)))						return TypeFloat4x4;
			else if (!str.CompareNoCase(TypeToString(TypeSampler2D)))						return TypeSampler2D;
			else if (!str.CompareNoCase(TypeToString(TypeSamplerCube)))						return TypeSamplerCube;
			return TypeUnknown;
		}
	};
}
