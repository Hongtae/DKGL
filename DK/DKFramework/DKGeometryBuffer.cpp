//
//  File: DKGeometryBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKGeometryBuffer.h"
#include "DKOpenGLContext.h"

using namespace DKFoundation;
using namespace DKFramework;


DKGeometryBuffer::DKGeometryBuffer(void)
: resourceId(0)
, resourceSize(0)
{
}

DKGeometryBuffer::~DKGeometryBuffer(void)
{
	Unlock();

	// unbind
	if (resourceType == BufferTypeVertexArray)
		DKOpenGLContext::RenderState().BindVertexBuffer(0);
	else
		DKOpenGLContext::RenderState().BindIndexBuffer(0);

	if (resourceId)
		glDeleteBuffers(1, &resourceId);

	resourceId = 0;
	resourceSize = 0;
}

bool DKGeometryBuffer::IsValid(void) const
{
	if (resourceId == 0)
		return false;

#ifdef DKGL_DEBUG_ENABLED
		return glIsBuffer(resourceId) == GL_TRUE;
#endif
	return true;
}

DKObject<DKBuffer> DKGeometryBuffer::CopyContent(void) const
{
	if (!IsValid())
		return NULL;
	AccessMode access;
	if (IsLocked(&access))
	{
		if (access == AccessModeReadOnly || access == AccessModeReadWrite)
		{
			void *p = AcquireLockedBuffer();
			if (p)
			{
				return DKBuffer::Create(p, resourceSize);
			}
		}
		DKLog("Buffer Object locked!");
		return NULL;
	}

	DKObject<DKBuffer> ret = NULL;
	if (Bind())
	{
		GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
#ifdef DKGL_OPENGL_ES
		void* p = glMapBufferRange(target, 0, resourceSize, GL_MAP_READ_BIT);
		ret = DKBuffer::Create(p, resourceSize);
		glUnmapBuffer(target);
#else
		ret = DKBuffer::Create(NULL, resourceSize);
		if (ret)
		{
			glGetBufferSubData(target, 0, resourceSize, ret->LockExclusive());
			ret->UnlockExclusive();
		}
#endif
	}
	return ret;
}

bool DKGeometryBuffer::IsLocked(AccessMode* lock) const
{
	if (!IsValid())
		return false;

	if (Bind())
	{
		GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
		GLint mapped = 0;
		glGetBufferParameteriv(target, GL_BUFFER_MAPPED, &mapped);
		if (mapped && lock)
		{
			GLint access = 0;
#ifdef DKGL_OPENGL_ES
			glGetBufferParameteriv(target, GL_BUFFER_ACCESS_FLAGS, &access);

			if (access & GL_MAP_READ_BIT && access & GL_MAP_WRITE_BIT)
				*lock = AccessModeReadWrite;
			else if (access & GL_MAP_READ_BIT)
				*lock = AccessModeReadOnly;
			else if (access & GL_MAP_WRITE_BIT)
				*lock = AccessModeWriteOnly;
			else
				DKLog("[%s] Unsupported buffer access.\n", DKGL_FUNCTION_NAME);
#else
			glGetBufferParameteriv(target, GL_BUFFER_ACCESS, &access);

			switch (access)
			{
				case GL_READ_ONLY:	*lock = AccessModeReadOnly;		break;
				case GL_WRITE_ONLY:	*lock = AccessModeWriteOnly;	break;
				case GL_READ_WRITE:	*lock = AccessModeReadWrite;	break;
				default:
					DKLog("[%s] Unsupported buffer access.\n", DKGL_FUNCTION_NAME);
					break;
			}
#endif
		}
		return mapped == GL_TRUE;
	}
	return false;
}

void* DKGeometryBuffer::AcquireLockedBuffer(void) const
{
	if (!IsValid() || !IsLocked(0))
		return NULL;

	if (Bind())
	{
		GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
		GLvoid* ptr = 0;
		glGetBufferPointerv(target, GL_BUFFER_MAP_POINTER, &ptr);
		return ptr;
	}
	return NULL;
}

void* DKGeometryBuffer::Lock(AccessMode lock)
{
	if (!IsValid())
		return NULL;

	if (IsLocked(0))
	{
		DKLog("[%s] Buffer locked already.", DKGL_FUNCTION_NAME);
		return NULL;
	}

	GLenum access = 0;
	switch (lock)
	{
#ifdef DKGL_OPENGL_ES
		case AccessModeReadOnly:	access = GL_MAP_READ_BIT;					break;
		case AccessModeWriteOnly:	access = GL_MAP_WRITE_BIT;					break;
		case AccessModeReadWrite:	access = GL_MAP_READ_BIT|GL_MAP_WRITE_BIT;	break;
#else
		case AccessModeReadOnly:	access = GL_READ_ONLY;	break;
		case AccessModeWriteOnly:	access = GL_WRITE_ONLY;	break;
		case AccessModeReadWrite:	access = GL_READ_WRITE;	break;
#endif
	default:
		DKLog("[%s] Unsupported buffer access.\n", DKGL_FUNCTION_NAME);
		return NULL;
	}

	if (Bind())
	{
		GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
		return glMapBufferRange(target, 0, resourceSize, access);
	}
	return NULL;
}

void DKGeometryBuffer::Unlock(void)
{
	if (!IsLocked(0))
		return;

	if (Bind())
	{
		GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
		glUnmapBuffer(target);
	}
}

bool DKGeometryBuffer::UpdateContent(BufferType t, MemoryLocation m, BufferUsage u, const DKData* data)
{
	if (data)
	{
		bool ret = UpdateContent(t, m, u, data->LockShared(), data->Length());
		data->UnlockShared();
		return ret;
	}
	return UpdateContent(t, m, u, NULL, 0);
}

bool DKGeometryBuffer::UpdateContent(BufferType t, MemoryLocation m, BufferUsage u, const void* data, size_t size)
{
	// check parameters
	switch (t)
	{
	case BufferTypeVertexArray:
	case BufferTypeElementArray:
		break;
	default:	return false;		// Wrong type.
	}
	switch (m)
	{
	case MemoryLocationStatic:
	case MemoryLocationDynamic:
	case MemoryLocationStream:
		break;
	default:	return false;		// Wrong memory location.
	}
	switch (u)
	{
	case BufferUsageDraw:
	case BufferUsageRead:
	case BufferUsageCopy:
		break;
	default:	return false;		// Wrong usage.
	}

	GLenum usage = 0;
	switch (u)
	{
	case BufferUsageDraw:
		usage = m == MemoryLocationStatic ? GL_STATIC_DRAW : (m == MemoryLocationDynamic ? GL_DYNAMIC_DRAW : GL_STREAM_DRAW);
		break;
	case BufferUsageRead:
		usage = m == MemoryLocationStatic ? GL_STATIC_READ : (m == MemoryLocationDynamic ? GL_DYNAMIC_READ : GL_STREAM_READ);
		break;
	case BufferUsageCopy:
		usage = m == MemoryLocationStatic ? GL_STATIC_COPY : (m == MemoryLocationDynamic ? GL_DYNAMIC_COPY : GL_STREAM_COPY);
		break;
	default:
		DKLog("[%s] Unsupported buffer usage.\n", DKGL_FUNCTION_NAME);
		return false;
	}

	if (resourceId == 0)
		glGenBuffers(1, &resourceId);

	if (t == BufferTypeVertexArray)
		DKOpenGLContext::RenderState().BindVertexBuffer(resourceId);
	else
		DKOpenGLContext::RenderState().BindIndexBuffer(resourceId);

	GLenum target = t == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
	glBindBuffer(target, resourceId);
	glBufferData(target, size, NULL, usage);		// invalidate current data
	if (data && size > 0)
		glBufferData(target, size, data, usage);	// upload new data
	
	resourceType = t;
	resourceLocation = m;
	resourceUsage = u;
	resourceSize = size;

	return true;
}

bool DKGeometryBuffer::UpdateSubContent(const void* data, size_t offset, size_t size)
{
	if (data == NULL || size == 0 || resourceSize < offset + size)
		return false;
	if (!IsValid())
		return false;

	if (resourceType == BufferTypeVertexArray)
		DKOpenGLContext::RenderState().BindVertexBuffer(resourceId);
	else
		DKOpenGLContext::RenderState().BindIndexBuffer(resourceId);

	GLenum target = resourceType == BufferTypeVertexArray ? GL_ARRAY_BUFFER : GL_ELEMENT_ARRAY_BUFFER;
	glBufferSubData(target, offset, size, data);
	
	return true;
}

bool DKGeometryBuffer::Bind(void) const
{
	if (!IsValid())
		return false;

	if (resourceType == BufferTypeVertexArray)
		DKOpenGLContext::RenderState().BindVertexBuffer(resourceId);
	else
		DKOpenGLContext::RenderState().BindIndexBuffer(resourceId);

	return true;
}

DKObject<DKSerializer> DKGeometryBuffer::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKGeometryBuffer* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKGeometryBuffer");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKResource::Serializer(), NULL);

			this->Bind(L"resourceType",
				DKFunction(this, &LocalSerializer::GetBufferType),
				DKFunction(this, &LocalSerializer::SetBufferType),
				DKFunction(this, &LocalSerializer::CheckBufferType),
				NULL);

			this->Bind(L"resourceLocation",
				DKFunction(this, &LocalSerializer::GetMemoryLocation),
				DKFunction(this, &LocalSerializer::SetMemoryLocation),
				DKFunction(this, &LocalSerializer::CheckMemoryLocation),
				NULL);

			this->Bind(L"resourceUsage",
				DKFunction(this, &LocalSerializer::GetBufferUsage),
				DKFunction(this, &LocalSerializer::SetBufferUsage),
				DKFunction(this, &LocalSerializer::CheckBufferUsage),
				NULL);

			this->Bind(L"contents",
				DKFunction(this, &LocalSerializer::GetContent),
				DKFunction(this, &LocalSerializer::SetContent),
				DKFunction(this, &LocalSerializer::CheckContent), NULL);

			return this;
		}
	private:
		void SetBufferType(DKVariant& v)
		{
			if (v.String().CompareNoCase(L"elementArray") == 0)
				type = BufferTypeElementArray;
			else
				type = BufferTypeVertexArray;
		}
		void SetMemoryLocation(DKVariant& v)
		{
			if (v.String().CompareNoCase(L"static") == 0)
				location = MemoryLocationStatic;
			else if (v.String().CompareNoCase(L"stream") == 0)
				location = MemoryLocationStream;
			else
				location = MemoryLocationDynamic;
		}
		void SetBufferUsage(DKVariant& v)
		{
			if (v.String().CompareNoCase(L"read") == 0)
				usage = BufferUsageRead;
			else if (v.String().CompareNoCase(L"copy") == 0)
				usage = BufferUsageCopy;
			else
				usage = BufferUsageDraw;
		}
		void SetContent(DKVariant& v)
		{
			if (v.ValueType() == DKVariant::TypeData)
				content = DKBuffer::Create(&v.Data());
			else if (v.ValueType() == DKVariant::TypeStructData)
				content = DKBuffer::Create(&v.StructuredData().data);
		}
		void GetBufferType(DKVariant& v) const
		{
			switch (target->Type())
			{
			case BufferTypeVertexArray:
				v = (const DKVariant::VString&)L"vertexArray";
				break;
			case BufferTypeElementArray:
				v = (const DKVariant::VString&)L"elementArray";
				break;
			}
		}
		void GetMemoryLocation(DKVariant& v) const
		{
			switch (target->Location())
			{
			case MemoryLocationStatic:
				v = (const DKVariant::VString&)L"static";
				break;
			case MemoryLocationDynamic:
				v = (const DKVariant::VString&)L"dynamic";
				break;
			case MemoryLocationStream:
				v = (const DKVariant::VString&)L"stream";
				break;
			}
		}
		void GetBufferUsage(DKVariant& v) const
		{
			switch (target->Usage())
			{
			case BufferUsageDraw:
				v = (const DKVariant::VString&)L"draw";
				break;
			case BufferUsageRead:
				v = (const DKVariant::VString&)L"read";
				break;
			case BufferUsageCopy:
				v = (const DKVariant::VString&)L"copy";
				break;
			}
		}
		void GetContent(DKVariant& v) const
		{
			DKObject<DKBuffer> data = target->CopyContent();
			if (data)
			{
				DKVariant::VStructuredData stData;
				stData.elementSize = 0;
				stData.layout.Clear();

				target->StructuredLayout(stData.layout, stData.elementSize);
				//v.SetData(*data);
				v.StructuredData() = std::move(stData);
			}
		}
		bool CheckBufferType(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeString)
			{
				if (v.String().CompareNoCase(L"vertexArray") == 0 ||
					v.String().CompareNoCase(L"elementArray") == 0)
					return true;
			}
			return false;
		}
		bool CheckMemoryLocation(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeString)
			{
				if (v.String().CompareNoCase(L"static") == 0 ||
					v.String().CompareNoCase(L"dynamic") == 0 ||
					v.String().CompareNoCase(L"stream") == 0)
					return true;
			}
			return false;
		}
		bool CheckBufferUsage(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeString)
			{
				if (v.String().CompareNoCase(L"draw") == 0 ||
					v.String().CompareNoCase(L"read") == 0 ||
					v.String().CompareNoCase(L"copy") == 0)
					return true;
			}
			return false;
		}
		bool CheckContent(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeData || v.ValueType() == DKVariant::TypeStructData;
		}
		void Callback(State s)
		{
			if (s == StateDeserializeSucceed)
			{
				target->UpdateContent(type, location, usage, content);
			}
		}
		BufferType					type;
		MemoryLocation				location;
		BufferUsage					usage;
		DKObject<DKBuffer>			content;
		DKObject<DKGeometryBuffer>	target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
