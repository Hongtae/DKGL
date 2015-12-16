//
//  File: DKShaderProgram.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKShaderProgram.h"
#include "DKOpenGLContext.h"
#include "DKResourceLoader.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			inline DKShaderConstant::Type GetShaderConstantType(GLenum t)
			{
				switch (t)
				{
				case GL_FLOAT:				return DKShaderConstant::TypeFloat1;
				case GL_FLOAT_VEC2:			return DKShaderConstant::TypeFloat2;
				case GL_FLOAT_VEC3:			return DKShaderConstant::TypeFloat3;
				case GL_FLOAT_VEC4:			return DKShaderConstant::TypeFloat4;
				case GL_INT:				return DKShaderConstant::TypeInt1;
				case GL_INT_VEC2:			return DKShaderConstant::TypeInt2;
				case GL_INT_VEC3:			return DKShaderConstant::TypeInt3;
				case GL_INT_VEC4:			return DKShaderConstant::TypeInt4;
				case GL_BOOL:				return DKShaderConstant::TypeBool1;
				case GL_BOOL_VEC2:			return DKShaderConstant::TypeBool2;
				case GL_BOOL_VEC3:			return DKShaderConstant::TypeBool3;
				case GL_BOOL_VEC4:			return DKShaderConstant::TypeBool4;
				case GL_FLOAT_MAT2:			return DKShaderConstant::TypeFloat2x2;
				case GL_FLOAT_MAT3:			return DKShaderConstant::TypeFloat3x3;
				case GL_FLOAT_MAT4:			return DKShaderConstant::TypeFloat4x4;
				case GL_SAMPLER_2D:			return DKShaderConstant::TypeSampler2D;
				case GL_SAMPLER_CUBE:		return DKShaderConstant::TypeSamplerCube;
				}
				DKLog("Warning: DKShaderConstant type (%x) invalid or not supported.\n", t);
				return DKShaderConstant::TypeUnknown;
			}

			inline DKVertexStream::Type GetVertexStreamType(GLenum t)
			{
				switch (t)
				{
				case GL_FLOAT:				return DKVertexStream::TypeFloat1;
				case GL_FLOAT_VEC2:			return DKVertexStream::TypeFloat2;
				case GL_FLOAT_VEC3:			return DKVertexStream::TypeFloat3;
				case GL_FLOAT_VEC4:			return DKVertexStream::TypeFloat4;
				case GL_FLOAT_MAT2:			return DKVertexStream::TypeFloat2x2;
				case GL_FLOAT_MAT3:			return DKVertexStream::TypeFloat3x3;
				case GL_FLOAT_MAT4:			return DKVertexStream::TypeFloat4x4;
				}
				DKLog("Warning: Unknown DKVertexStream type conversion (%x) invalid or not supported.\n", t);
				return DKVertexStream::TypeUnknown;
			}

			struct ProgramInfo
			{
				GLuint programId;
				DKShaderProgram::AttributeArray		attributes;
				DKShaderProgram::UniformArray		uniforms;
				DKArray<DKObject<DKShader>>			shaders;
			};

			bool CreateProgram(const DKArray<DKShader*>& shaders, DKString& err, ProgramInfo& pi)
			{
				DKOpenGLContext::RenderState().UseProgram(0);

				if (shaders.IsEmpty())
				{
					err = L"No shader";
					return false;
				}

				// link all shader objects in lists.
				GLuint id = glCreateProgram();

				DKArray<DKObject<DKShader>> attachedShaders;
				attachedShaders.Reserve(shaders.Count());

				for (DKShader* shader : shaders)
				{
					if (shader && shader->IsValid())
					{
						glAttachShader(id, shader->GetId());
						attachedShaders.Add(shader);
					}
				}
				glLinkProgram(id);

				// get linker log
				int logLen = 0;
				glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLen);
				if (logLen > 1)
				{
					char *log = (char*)DKMemoryDefaultAllocator::Alloc(logLen+4);
					int charsWritten = 0;
					glGetProgramInfoLog(id, logLen, &charsWritten, log);
					err = log;
					DKMemoryDefaultAllocator::Free(log);
				}

				// get link result
				GLint ret = 0;
				glGetProgramiv(id, GL_LINK_STATUS, &ret);
				if (ret)
				{
					glValidateProgram(id);
					GLint validate = 0;
					glGetProgramiv(id, GL_VALIDATE_STATUS, &validate);
					if (!validate)
					{
						DKLog("Warning: ShaderProgram(%x) validation failed.\n", id);
						logLen = 0;
						glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLen);
						if (logLen > 1)
						{
							char *log = (char*)DKMemoryDefaultAllocator::Alloc(logLen+4);
							int charsWritten = 0;
							glGetProgramInfoLog(id, logLen, &charsWritten, log);
							DKLog("Validation log: %ls\n", (const wchar_t*)DKString(log));
							DKMemoryDefaultAllocator::Free(log);
						}
					}

					DKShaderProgram::AttributeArray	attribs;
					DKShaderProgram::UniformArray	uniforms;

					GLint numAttribs = 0;				// number of attributes
					GLint maxAttribNameLength = 0;		// maximum length of attribute name
					glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &numAttribs);
					glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribNameLength);
					GLint numUniforms = 0;				// number of uniforms
					GLint maxUniformNameLength = 0;		// maximul length of uniform name
					glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numUniforms);
					glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

					attribs.Reserve(numAttribs);
					uniforms.Reserve(numUniforms);

					size_t buffSize = Max(maxAttribNameLength, maxUniformNameLength) + 2;
					char* name = (char*)DKMemoryDefaultAllocator::Alloc(buffSize+1);
					for (int i = 0; i < numAttribs; i++)
					{
						GLsizei	len = 0;
						GLint	size = 0;
						GLenum	type;
						glGetActiveAttrib(id, i, buffSize, &len, &size, &type, name);
						if (len > 0 && name[0])
						{
							int loc = glGetAttribLocation(id, name);
							if (loc >= 0)
							{
								DKVertexStream	att = {name, DKVertexStream::StreamUnknown, GetVertexStreamType(type), static_cast<size_t>(size), loc};
								attribs.Add(att);
							}
							// DKLog("Shader Program: 0x%x Attribute[%d] = %ls (size:%d, type:%ls, location:%d)\n",
							//	id, i, (const wchar_t*)DKString(name), size, (const wchar_t*)DKVertexStream::TypeToString((DKVertexStream::Type)type), loc);
						}
						else
						{
							DKLog("Error: Shader Program: 0x%x Attribute[%d] is unknown.\n", id, i);
						}
					}
					for (int i = 0; i < numUniforms; i++)
					{
						GLsizei	len = 0;
						GLint	size = 0;
						GLenum	type;
						glGetActiveUniform(id, i, buffSize, &len, &size, &type, name);
						if (len > 0 && name[0])
						{
							if (len > 3 && name[len-3] == '[' && name[len-2] == '0' && name[len-1] == ']')
							{
								// name is array type as 'name[0]', remove '[0]' suffix.
								name[len-3] = NULL;					
							}
							int loc = glGetUniformLocation(id, name);
							if (loc >= 0)
							{
								DKShaderConstant	uni = {name, DKShaderConstant::UniformUnknown, GetShaderConstantType(type), static_cast<size_t>(size), loc};
								uniforms.Add(uni);
							}
							// DKLog("Shader Program: 0x%x Uniform[%d] = %ls (size:%d, type:%ls, location:%d)\n",
							//	id, i, (const wchar_t*)DKString(name), size, (const wchar_t*)DKShaderConstant::TypeToString((DKShaderConstant::Type)type), loc);
						}
						else
						{
							DKLog("Error: Shader Program: 0x%x Uniform[%d] = %ls (%d)\n", id, i, (const wchar_t*)DKString(name), size);
						}
					}
					DKMemoryDefaultAllocator::Free(name);

					pi.programId = id;
					pi.shaders = attachedShaders;
					pi.attributes = attribs;
					pi.uniforms = uniforms;

					glFlush();
					return true;
				}

				// link error
				if (err.Length() == 0)
					err = L"Failed to link shaders.";
				glDeleteProgram(id);
				return false;
			}
		}
	}
}

using namespace DKFramework;


DKShaderProgram::DKShaderProgram(void)
	: programId(0)
{
}

DKShaderProgram::~DKShaderProgram(void)
{
	DKOpenGLContext::RenderState().UseProgram(0);

	if (glIsProgram(programId))
	{
		// attached shaders will be deleted later.
		glDeleteProgram(programId);
	}
}

void DKShaderProgram::Bind(void) const
{
	DKOpenGLContext::RenderState().UseProgram(programId);
}

DKObject<DKShaderProgram> DKShaderProgram::Create(const DKArray<DKShader*>& shaders, DKString& err)
{
	Private::ProgramInfo pi;
	if (Private::CreateProgram(shaders, err, pi))
	{
		DKObject<DKShaderProgram> program = DKObject<DKShaderProgram>::New();
		program->programId = pi.programId;
		program->attributes = pi.attributes;
		program->uniforms = pi.uniforms;
		return program;
	}
	return NULL;
}

DKObject<DKShaderProgram> DKShaderProgram::Create(const DKArray<DKObject<DKShader>>& shaders, DKString& err)
{
	DKArray<DKShader*> shaders2;
	shaders2.Reserve(shaders.Count());
	for (DKObject<DKShader> s : shaders)
		shaders2.Add(s);

	return Create(shaders2, err);
}

bool DKShaderProgram::IsValid(void) const
{
	if (glIsProgram(programId))
	{
		GLint	link = 0;
		GLint	validate = 0;
		glGetProgramiv(programId, GL_LINK_STATUS, &link);
		glGetProgramiv(programId, GL_VALIDATE_STATUS, &validate);
		return link == GL_TRUE && validate == GL_TRUE;
	}
	return false;
}

int	DKShaderProgram::GetUniformComponents(const DKFoundation::DKString& name) const
{
	for (int i = 0; i < uniforms.Count(); i++)
	{
		if (uniforms.Value(i).name == name)
			return uniforms.Value(i).components;
	}
	return 0;
}

int DKShaderProgram::GetAttribComponents(const DKFoundation::DKString& name) const
{
	for (int i = 0; i < attributes.Count(); i++)
	{
		if (attributes.Value(i).name == name)
			return attributes.Value(i).components;
	}
	return 0;
}
