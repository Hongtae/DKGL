//
//  File: DKShaderProgram.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKGL_API DKShaderProgram
	{
	public:
		typedef DKArray<DKShaderConstant>		UniformArray;
		typedef DKArray<DKVertexStream>		AttributeArray;

		AttributeArray		attributes;
		UniformArray		uniforms;

		DKShaderProgram(void);
		~DKShaderProgram(void);

		void Bind(void) const;

		static DKObject<DKShaderProgram> Create(const DKArray<DKObject<DKShader>>& shaders, DKString& err);
		static DKObject<DKShaderProgram> Create(const DKArray<DKShader*>& shaders, DKString& err);

		unsigned int GetId(void) const				{return programId;}
		bool IsValid(void) const;

		int	GetUniformComponents(const DKString& name) const;
		int GetAttribComponents(const DKString& name) const;

	private:
		unsigned int programId;
	};
}
