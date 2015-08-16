//
//  File: DKShader.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2006-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKShader.h"
#include "DKScreen.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		GLuint CompileShader(const DKString& implement, DKShader::Type t, DKString& err)
		{
			if (implement.Length() == 0)
			{
				err = L"No implementation.";
				return 0;
			}
			if (t != DKShader::TypeFragmentShader && t != DKShader::TypeGeometryShader && t != DKShader::TypeVertexShader)
			{
				err = L"Unknown shader type";
				return 0;
			}
			DKStringU8 source(implement);
			if (source.Bytes() == 0)
			{
				err = L"Failed to convert source encoding (UTF-8)";
				return 0;
			}

			GLuint hShader = 0;

			if (t == DKShader::TypeVertexShader)		// vertex shader
			{
				hShader = glCreateShader(GL_VERTEX_SHADER);
			}
			else if (t == DKShader::TypeFragmentShader)	// fragment shader (pixel shader)
			{
				hShader = glCreateShader(GL_FRAGMENT_SHADER);
			}
			else
			{
#ifdef GL_GEOMETRY_SHADER
				hShader = glCreateShader(GL_GEOMETRY_SHADER);
#else
				err = L"Geometry shader not supported in this implementation.";
				DKLog("Error: Geometry shader not supported in this implementation.\n");
				return 0;
#endif
			}

			if (hShader == 0)
			{
				err = L"Failed to create shader object";
				return 0;
			}

			const char* src = (const char*)source;
			glShaderSource(hShader, 1, &src, NULL);
			glCompileShader(hShader);

			// retrieve compiler log
			int nLogLen = 0;
			glGetShaderiv(hShader, GL_INFO_LOG_LENGTH, &nLogLen);
			if (nLogLen > 1)
			{
				char *pLog = (char*)DKMemoryDefaultAllocator::Alloc(nLogLen);
				int nCharsWritten = 0;
				glGetShaderInfoLog(hShader, nLogLen, &nCharsWritten, pLog);
				err = pLog;
				DKMemoryDefaultAllocator::Free(pLog);
			}

			////////////////////////////////////////////////////////////////////////////////
			// get compilation result.
			int result = 0;
			glGetShaderiv(hShader, GL_COMPILE_STATUS, &result);
			glFlush();
			if (result)
			{
				return hShader;
			}

			// compile error.
			if (err.Length() == 0)
				err = L"Failed to compile shader.";
			glDeleteShader(hShader);
			return 0;
		}
	}
}

using namespace DKFramework;


DKShader::DKShader(void)
	: objectId(0)
{
}

DKShader::~DKShader(void)
{
	if (objectId)
	{
		glDeleteShader(objectId);
	}
}

DKObject<DKShader> DKShader::Create(const DKFoundation::DKString& implement, Type t)
{
	DKString err = L"";
	return Create(implement, t, err);
}

DKObject<DKShader> DKShader::Create(const DKFoundation::DKString& implement, Type t, DKFoundation::DKString& err)
{
	GLuint hShader = Private::CompileShader(implement, t, err);
	if (hShader)
	{
		DKObject<DKShader> s = DKObject<DKShader>::New();
		s->objectId = hShader;
		return s;
	}
	return NULL;
}

bool DKShader::IsValid(void) const
{
	if (glIsShader(objectId))
	{
		GLint status = 0;
		glGetShaderiv(objectId, GL_COMPILE_STATUS, &status);
		return status == GL_TRUE;
	}
	return false;
}

DKShader::Type DKShader::GetType(void) const
{
	if (glIsShader(objectId))
	{
		GLint type = 0;
		glGetShaderiv(objectId, GL_SHADER_TYPE, &type);

		if (type == GL_VERTEX_SHADER)
			return TypeVertexShader;
		else if (type == GL_FRAGMENT_SHADER)
			return TypeFragmentShader;
#ifdef GL_GEOMETRY_SHADER
		else if (type == GL_GEOMETRY_SHADER)
			return TypeGeometryShader;
#endif
	}
	return TypeUnknown;
}

DKString DKShader::GetSource(void) const
{
	DKString src = L"";

	if (glIsShader(objectId))
	{
		GLint srcLen = 0;
		glGetShaderiv(objectId, GL_SHADER_SOURCE_LENGTH, &srcLen);
		if (srcLen > 0)
		{
			char* buff = (char*)DKMemoryDefaultAllocator::Alloc(srcLen+4);
			GLsizei output = 0;
			glGetShaderSource(objectId, srcLen+4, &output, buff);
			if (output > 0)
			{
				src = DKString((const DKUniChar8*)buff, output);
			}
			DKMemoryDefaultAllocator::Free(buff);
		}
	}
	return src;
}
