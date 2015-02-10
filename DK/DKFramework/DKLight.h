//
//  File: DKLight.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKColor.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKLight
// a light object.
//
// Note:
//   This class has been deprecated. Use custom shader instead.
//   (No need for deferred rendering system)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLight
	{
	public:
		enum LightType
		{
			LightTypeUnknown = 0,
			LightTypeDirectional,
			LightTypePoint,
		};
		DKLight(void)
			: type(LightTypeUnknown)
		{
		}
		DKLight(const DKVector3& dir, const DKColor& c)
			: type(LightTypeDirectional)
			, position(DKVector3(-dir).Normalize())
			, color(c)
			, constAttenuation(1.0f)
			, linearAttenuation(0.0f)
			, quadraticAttenuation(0.0f)
		{
		}
		DKLight(const DKVector3& pos, const DKColor& c, float constAtten, float linearAtten, float quadAtten)
			: type(LightTypePoint)
			, position(pos)
			, color(c)
			, constAttenuation(constAtten)
			, linearAttenuation(linearAtten)
			, quadraticAttenuation(quadAtten)
		{
		}
		LightType Type(void) const
		{
			return type;
		}
		void SetType(LightType t)
		{
			type = t;
		}
		DKVector3 Direction(void) const
		{
			return DKVector3(-position).Normalize();
		}
		void SetDirection(const DKVector3& dir)
		{
			position = DKVector3(-dir).Normalize();
		}
		float Attenuation(const DKVector3& pos) const
		{
			if (type == LightTypeDirectional)
				return 1.0f;
			float distance = (position - pos).Length();
			return 1.0f / (constAttenuation + linearAttenuation * distance + quadraticAttenuation * distance * distance);
		}

		DKVector3	position;
		DKColor		color;
		float constAttenuation;
		float linearAttenuation;
		float quadraticAttenuation;

	private:
		LightType type;
	};
}
