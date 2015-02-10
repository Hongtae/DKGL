//
//  File: DKShaderProgram.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKShader.h"
#include "DKVertexStream.h"
#include "DKShaderConstant.h"

////////////////////////////////////////////////////////////////////////////////
// DKShaderProgram
// shader program module class. object can be linked with come compiled shader
// objects. this class manages program object which will be transferred to
// GPU when bind to context.
//
// You can inspect Uniforms (with Samplers), Attributes values.
// If you set value to uniforms or attributes, the value will be retained
// utill program object being destroyed.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKShaderProgram
	{
	public:
		typedef DKFoundation::DKArray<DKShaderConstant>		UniformArray;
		typedef DKFoundation::DKArray<DKVertexStream>		AttributeArray;

		AttributeArray		attributes;
		UniformArray		uniforms;

		DKShaderProgram(void);
		~DKShaderProgram(void);

		void Bind(void) const;

		static DKFoundation::DKObject<DKShaderProgram> Create(const DKFoundation::DKArray<DKFoundation::DKObject<DKShader>>& shaders, DKFoundation::DKString& err);
		static DKFoundation::DKObject<DKShaderProgram> Create(const DKFoundation::DKArray<DKShader*>& shaders, DKFoundation::DKString& err);

		unsigned int GetId(void) const				{return programId;}
		bool IsValid(void) const;

		int	GetUniformComponents(const DKFoundation::DKString& name) const;
		int GetAttribComponents(const DKFoundation::DKString& name) const;

	private:
		unsigned int programId;
	};
}
