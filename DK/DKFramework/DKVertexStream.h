//
//  File: DKVertexStream.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

///////////////////////////////////////////////////////////////////////////////
// DKVertexStream
// predefined stream type. stream type should be matched with defined in shader.
// you can retrieve stream types from shader program. (see DKShaderProgram.h)
///////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	struct DKVertexStream
	{
		enum Stream : unsigned char
		{
			StreamUnknown = 0,		// regard to error
			StreamPosition,
			StreamNormal,
			StreamColor,
			StreamTexCoord,
			StreamTangent,
			StreamBitangent,
			StreamBlendIndices,
			StreamBlendWeights,
			StreamUserDefine,		// user-define (you can access by name, at shader)
			StreamMax,
		};
		enum Type : unsigned char
		{
			TypeUnknown = 0,
			TypeFloat1,
			TypeFloat2,
			TypeFloat3,
			TypeFloat4,
			TypeFloat2x2,
			TypeFloat3x3,
			TypeFloat4x4,
			TypeByte1,
			TypeByte2,
			TypeByte3,
			TypeByte4,
			TypeUByte1,
			TypeUByte2,
			TypeUByte3,
			TypeUByte4,
			TypeShort1,
			TypeShort2,
			TypeShort3,
			TypeShort4,
			TypeUShort1,
			TypeUShort2,
			TypeUShort3,
			TypeUShort4,
			TypeMax,
		};

		DKFoundation::DKString	name;
		Stream					id;
		Type					type;
		size_t					components;
		int						location;

		// utils
		static size_t TypeSize(Type t)
		{
			switch (t)
			{
			case TypeFloat1:		return sizeof(float)*1;
			case TypeFloat2:		return sizeof(float)*2;
			case TypeFloat3:		return sizeof(float)*3;
			case TypeFloat4:		return sizeof(float)*4;
			case TypeFloat2x2:		return sizeof(float)*4;
			case TypeFloat3x3:		return sizeof(float)*9;
			case TypeFloat4x4:		return sizeof(float)*16;
			case TypeByte1:			return sizeof(char)*1;
			case TypeByte2:			return sizeof(char)*2;
			case TypeByte3:			return sizeof(char)*3;
			case TypeByte4:			return sizeof(char)*4;
			case TypeUByte1:		return sizeof(unsigned char)*1;
			case TypeUByte2:		return sizeof(unsigned char)*2;
			case TypeUByte3:		return sizeof(unsigned char)*3;
			case TypeUByte4:		return sizeof(unsigned char)*4;
			case TypeShort1:		return sizeof(short)*1;
			case TypeShort2:		return sizeof(short)*2;
			case TypeShort3:		return sizeof(short)*3;
			case TypeShort4:		return sizeof(short)*4;
			case TypeUShort1:		return sizeof(unsigned short)*1;
			case TypeUShort2:		return sizeof(unsigned short)*2;
			case TypeUShort3:		return sizeof(unsigned short)*3;
			case TypeUShort4:		return sizeof(unsigned short)*4;
			default:
				break;
			}
			return 0;
		}
		static DKFoundation::DKString StreamToString(Stream s)
		{
			switch (s)
			{
			case StreamPosition:		return L"Position";
			case StreamNormal:			return L"Normal";
			case StreamColor:			return L"Color";
			case StreamTexCoord:		return L"TexCoord";
			case StreamTangent:			return L"Tangent";
			case StreamBitangent:		return L"Bitangent";
			case StreamBlendIndices:	return L"BlendIndices";
			case StreamBlendWeights:	return L"BlendWeights";
			case StreamUserDefine:		return L"UserDefine";
			default:
				break;
			}
			return L"Unknown";
		}
		static Stream StringToStream(const DKFoundation::DKString& str)
		{
					if (!str.CompareNoCase(StreamToString(StreamPosition)))			return StreamPosition;
			else	if (!str.CompareNoCase(StreamToString(StreamNormal)))			return StreamNormal;
			else	if (!str.CompareNoCase(StreamToString(StreamColor)))			return StreamColor;
			else	if (!str.CompareNoCase(StreamToString(StreamTexCoord)))			return StreamTexCoord;
			else	if (!str.CompareNoCase(StreamToString(StreamTangent)))			return StreamTangent;
			else	if (!str.CompareNoCase(StreamToString(StreamBitangent)))		return StreamBitangent;
			else	if (!str.CompareNoCase(StreamToString(StreamBlendIndices)))		return StreamBlendIndices;
			else	if (!str.CompareNoCase(StreamToString(StreamBlendWeights)))		return StreamBlendWeights;
			else	if (!str.CompareNoCase(StreamToString(StreamUserDefine)))		return StreamUserDefine;
			return StreamUnknown;
		}
		static DKFoundation::DKString TypeToString(Type t)
		{
			switch (t)
			{
			case TypeFloat1:		return L"Float1";
			case TypeFloat2:		return L"Float2";
			case TypeFloat3:		return L"Float3";
			case TypeFloat4:		return L"Float4";
			case TypeFloat2x2:		return L"Float2x2";
			case TypeFloat3x3:		return L"Float3x3";
			case TypeFloat4x4:		return L"Float4x4";
			case TypeByte1:			return L"Byte1";
			case TypeByte2:			return L"Byte2";
			case TypeByte3:			return L"Byte3";
			case TypeByte4:			return L"Byte4";
			case TypeUByte1:		return L"UByte1";
			case TypeUByte2:		return L"UByte2";
			case TypeUByte3:		return L"UByte3";
			case TypeUByte4:		return L"UByte4";
			case TypeShort1:		return L"Short1";
			case TypeShort2:		return L"Short2";
			case TypeShort3:		return L"Short3";
			case TypeShort4:		return L"Short4";
			case TypeUShort1:		return L"UShort1";
			case TypeUShort2:		return L"UShort2";
			case TypeUShort3:		return L"UShort3";
			case TypeUShort4:		return L"UShort4";
			default:
				break;
			}
			return L"Unknown";
		}
		static Type StringToType(const DKFoundation::DKString& str)
		{
			if		(!str.CompareNoCase(TypeToString(TypeFloat1)))		return TypeFloat1;
			else if (!str.CompareNoCase(TypeToString(TypeFloat2)))		return TypeFloat2;
			else if (!str.CompareNoCase(TypeToString(TypeFloat3)))		return TypeFloat3;
			else if (!str.CompareNoCase(TypeToString(TypeFloat4)))		return TypeFloat4;
			else if (!str.CompareNoCase(TypeToString(TypeFloat2x2)))	return TypeFloat2x2;
			else if (!str.CompareNoCase(TypeToString(TypeFloat3x3)))	return TypeFloat3x3;
			else if (!str.CompareNoCase(TypeToString(TypeFloat4x4)))	return TypeFloat4x4;
			else if (!str.CompareNoCase(TypeToString(TypeByte1)))		return TypeByte1;
			else if (!str.CompareNoCase(TypeToString(TypeByte2)))		return TypeByte2;
			else if (!str.CompareNoCase(TypeToString(TypeByte3)))		return TypeByte3;
			else if (!str.CompareNoCase(TypeToString(TypeByte4)))		return TypeByte4;
			else if (!str.CompareNoCase(TypeToString(TypeUByte1)))		return TypeUByte1;
			else if (!str.CompareNoCase(TypeToString(TypeUByte2)))		return TypeUByte2;
			else if (!str.CompareNoCase(TypeToString(TypeUByte3)))		return TypeUByte3;
			else if (!str.CompareNoCase(TypeToString(TypeUByte4)))		return TypeUByte4;
			else if (!str.CompareNoCase(TypeToString(TypeShort1)))		return TypeShort1;
			else if (!str.CompareNoCase(TypeToString(TypeShort2)))		return TypeShort2;
			else if (!str.CompareNoCase(TypeToString(TypeShort3)))		return TypeShort3;
			else if (!str.CompareNoCase(TypeToString(TypeShort4)))		return TypeShort4;
			else if (!str.CompareNoCase(TypeToString(TypeUShort1)))		return TypeUShort1;
			else if (!str.CompareNoCase(TypeToString(TypeUShort2)))		return TypeUShort2;
			else if (!str.CompareNoCase(TypeToString(TypeUShort3)))		return TypeUShort3;
			else if (!str.CompareNoCase(TypeToString(TypeUShort4)))		return TypeUShort4;
			return TypeUnknown;
		}
	};
}

