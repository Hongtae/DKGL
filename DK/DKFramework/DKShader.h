//
//  File: DKShader.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

////////////////////////////////////////////////////////////////////////////////
// DKShader
// GLSL shader code object, can be compiled with OpenGL compiler.
// each shader object can be compiled to one object, multiple objects can
// be linked as shader-program module. (see DKShaderProgram.h)
// a shader program module can be transferred to GPU when bind to context.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKShader
	{
	public:
		enum Type
		{
			TypeUnknown = 0,
			TypeVertexShader,
			TypeFragmentShader,
			TypeGeometryShader, // not supported in OpenGL ES
		};

		DKShader(void);
		~DKShader(void);

		unsigned int GetId(void) const	{return objectId;}

		bool IsValid(void) const;
		Type GetType(void) const;

		// compile and create shader object from source.
		static DKFoundation::DKObject<DKShader> Create(const DKFoundation::DKString& source, Type t);
		static DKFoundation::DKObject<DKShader> Create(const DKFoundation::DKString& source, Type t, DKFoundation::DKString& err);

		DKFoundation::DKString GetSource(void) const;

	private:
		unsigned int objectId;
	};
}
