//
//  File: DKTextureSampler.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKTextureSampler.h"

using namespace DKGL;
namespace DKGL
{
	namespace Private
	{
		GLenum GetTextureTargetGLValue(DKTexture::Target t);
	}
}
using namespace DKGL;
using namespace DKGL::Private;

DKTextureSampler::DKTextureSampler(void)
: minFilter(MinFilterUndefined)
, magFilter(MagFilterUndefined)
, LODBias(0.0f)
{
	wrap[0] = WrapUndefined;
	wrap[1] = WrapUndefined;
	wrap[2] = WrapUndefined;
}

DKTextureSampler::~DKTextureSampler(void)
{
}

void DKTextureSampler::Bind(DKTexture::Target t) const
{
	GLenum target = GetTextureTargetGLValue(t);
	if (target == 0)
		return;

	switch (this->minFilter)
	{
	case MagFilterNearest:
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case MagFilterLinear:
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}
	switch (this->magFilter)
	{
	case MinFilterNearest:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case MinFilterLinear:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case MinFilterNearestMipmapNearest:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		break;
	case MinFilterNearestMipmapLinear:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		break;
	case MinFilterLinearMipmapNearest:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		break;
	case MinFilterLinearMipmapLinear:
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	}
	
	GLenum wrapModes[] = {
		GL_TEXTURE_WRAP_S,
		GL_TEXTURE_WRAP_T,
		GL_TEXTURE_WRAP_R,
	};
	int numModes = sizeof(wrapModes) / sizeof(wrapModes[0]);
	for (int i = 0; i < numModes; i++)
	{
		switch (wrap[i])
		{
		case WrapRepeat:
			glTexParameteri(target, wrapModes[i], GL_REPEAT);
			break;
		case WrapMirroredRepeat:
			glTexParameteri(target, wrapModes[i], GL_MIRRORED_REPEAT);
			break;
		case WrapClampToEdge:
			glTexParameteri(target, wrapModes[i], GL_CLAMP_TO_EDGE);
			break;
		}
	}
}

DKObject<DKSerializer> DKTextureSampler::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKObject<DKTextureSampler> target;

		DKString MinFilterToString(MinificationFilter f) const
		{
			switch (f)
			{
			case MinFilterUndefined:				return L"undefined";
			case MinFilterNearest:					return L"nearest";
			case MinFilterLinear:					return L"linear";
			case MinFilterNearestMipmapNearest:		return L"nearestMipmapNearest";
			case MinFilterNearestMipmapLinear:		return L"nearestMipmapLinear";
			case MinFilterLinearMipmapNearest:		return L"linearMipmapNearest";
			case MinFilterLinearMipmapLinear:		return L"linearMipmapLinear";
			}
			return L"unknown";
		}
		bool StringToMinFilter(const DKString& s, MinificationFilter* f) const
		{
			if (s.CompareNoCase(MinFilterToString(MinFilterUndefined)) == 0)			{ *f = MinFilterUndefined; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterNearest)) == 0)				{ *f = MinFilterNearest; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterLinear)) == 0)				{ *f = MinFilterLinear; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterNearestMipmapNearest)) == 0)	{ *f = MinFilterNearestMipmapNearest; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterNearestMipmapLinear)) == 0)	{ *f = MinFilterNearestMipmapLinear; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterLinearMipmapNearest)) == 0)	{ *f = MinFilterLinearMipmapNearest; return true; }
			if (s.CompareNoCase(MinFilterToString(MinFilterLinearMipmapLinear)) == 0)	{ *f = MinFilterLinearMipmapLinear; return true; }
			return false;
		}
		DKString MagFilterToString(MagnificationFilter f) const
		{
			switch (f)
			{
			case MagFilterUndefined:			return L"undefined";
			case MagFilterNearest:				return L"nearest";
			case MagFilterLinear:				return L"linear";
			}
			return L"unknown";
		}
		bool StringToMagFilter(const DKString& s, MagnificationFilter* f) const
		{
			if (s.CompareNoCase(MagFilterToString(MagFilterUndefined)) == 0)	{ *f = MagFilterUndefined; return true; }
			if (s.CompareNoCase(MagFilterToString(MagFilterNearest)) == 0)		{ *f = MagFilterNearest; return true; }
			if (s.CompareNoCase(MagFilterToString(MagFilterLinear)) == 0)		{ *f = MagFilterLinear; return true; }
			return false;
		}
		DKString WrapToString(Wrap w) const
		{
			switch (w)
			{
			case WrapUndefined:			return L"undefined";
			case WrapRepeat:			return L"repeat";
			case WrapMirroredRepeat:	return L"mirroredRepeat";
			case WrapClampToEdge:		return L"clampToEdge";
			}
			return L"unknown";
		}
		bool StringToWrap(const DKString& s, Wrap* w) const
		{
			if (s.CompareNoCase(WrapToString(WrapUndefined)) == 0)		{ *w = WrapUndefined; return true; }
			if (s.CompareNoCase(WrapToString(WrapRepeat)) == 0)			{ *w = WrapRepeat; return true; }
			if (s.CompareNoCase(WrapToString(WrapMirroredRepeat)) == 0)	{ *w = WrapMirroredRepeat; return true; }
			if (s.CompareNoCase(WrapToString(WrapClampToEdge)) == 0)	{ *w = WrapClampToEdge; return true; }
			return false;
		}
		DKSerializer* Init(DKTextureSampler* p)
		{
			if (p == NULL)
				return NULL;

			this->target = p;
			this->SetResourceClass(L"DKTextureSampler");
			this->Bind(L"super", target->DKResource::Serializer(), NULL);

			this->Bind(L"minFilter",
				DKFunction([this](DKVariant& v) {v.SetString(MinFilterToString(target->minFilter)); }),
				DKFunction([this](DKVariant& v) {StringToMinFilter(v.String(), &target->minFilter); }),
				DKFunction([this](const DKVariant& v)
			{
				MinificationFilter f;
				return v.ValueType() == DKVariant::TypeString && StringToMinFilter(v.String(), &f);
			}), NULL);

			this->Bind(L"magFilter",
				DKFunction([this](DKVariant& v) {v.SetString(MagFilterToString(target->magFilter)); }),
				DKFunction([this](DKVariant& v) {StringToMagFilter(v.String(), &target->magFilter); }),
				DKFunction([this](const DKVariant& v)
			{
				MagnificationFilter f;
				return v.ValueType() == DKVariant::TypeString && StringToMagFilter(v.String(), &f);
			}), NULL);

			this->Bind(L"wrapS", 
				DKFunction([this](DKVariant& v) {v.SetString(WrapToString(target->wrap[0])); }),
				DKFunction([this](DKVariant& v) {StringToWrap(v.String(), &target->wrap[0]); }),
				DKFunction([this](const DKVariant& v)
			{
				Wrap w;
				return v.ValueType() == DKVariant::TypeString && StringToWrap(v.String(), &w);
			}), NULL);

			this->Bind(L"wrapT",
				DKFunction([this](DKVariant& v) {v.SetString(WrapToString(target->wrap[1])); }),
				DKFunction([this](DKVariant& v) {StringToWrap(v.String(), &target->wrap[1]); }),
				DKFunction([this](const DKVariant& v)
			{
				Wrap w;
				return v.ValueType() == DKVariant::TypeString && StringToWrap(v.String(), &w);
			}), NULL);

			this->Bind(L"wrapR",
				DKFunction([this](DKVariant& v) {v.SetString(WrapToString(target->wrap[2])); }),
				DKFunction([this](DKVariant& v) {StringToWrap(v.String(), &target->wrap[2]); }),
				DKFunction([this](const DKVariant& v)
			{
				Wrap w;
				return v.ValueType() == DKVariant::TypeString && StringToWrap(v.String(), &w);
			}), NULL);

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
