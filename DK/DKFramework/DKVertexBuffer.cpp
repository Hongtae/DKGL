//
//  File: DKVertexBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKVertexBuffer.h"
#include "DKOpenGLContext.h"

using namespace DKFoundation;
using namespace DKFramework;


DKVertexBuffer::DKVertexBuffer(void)
	: vertexSize(0)
	, vertexCount(0)
{
}

DKVertexBuffer::~DKVertexBuffer(void)
{
}

DKObject<DKVertexBuffer> DKVertexBuffer::Create(const Decl* decls, size_t declCount, const void* vertices, size_t size, size_t count, MemoryLocation m, BufferUsage u)
{
	// count can be 0, but size must not be 0.
	// declarations can not be modified.

	if (decls == NULL || declCount == 0 || size == 0)
		return NULL;
	
	DKSet<DKString> userDefineNames;
	DKSet<DKVertexStream::Stream> streamSet;

	size_t offset = 0;

	for (size_t i = 0; i < declCount; i++)
	{
		size_t typeSize = DKVertexStream::TypeSize(decls[i].type);
		offset += typeSize;

		if (offset > size)
		{
			DKLog("Invalid vertex size.\n");
			return NULL;
		}

		if (decls[i].id < DKVertexStream::StreamUserDefine)
		{
			if (streamSet.Contains(decls[i].id))
			{
				DKLog("Stream:%d duplicated!\n", decls[i].id);
				return NULL;
			}
			streamSet.Insert(decls[i].id);
		}
		else
		{
			if (decls[i].name.Length() == 0 || userDefineNames.Contains(decls[i].name))
			{
				DKLog("Invalid user-defined format name!\n");
				return NULL;
			}
			userDefineNames.Insert(decls[i].name);
		}
	}

	if (size % 4 != 0)
		DKLog("[DKVertexBuffer] Warning: vertex size is not 4bytes alignments.\n");

	DKObject<DKVertexBuffer> buffer = DKObject<DKVertexBuffer>::New();
	buffer->declarations = DKArray<Decl>(decls, declCount);
	buffer->vertexSize = size;
	buffer->vertexCount = 0;
	if (buffer->UpdateContent(vertices, count, m, u))
		return buffer;
	return NULL;
}

const DKVertexBuffer::Decl* DKVertexBuffer::DeclarationAtIndex(long index) const
{
	if (index >= 0 && index < declarations.Count())
		return &declarations[index];
	return NULL;
}

const DKVertexBuffer::Decl* DKVertexBuffer::Declaration(DKVertexStream::Stream stream) const
{
	for (const Decl& decl : declarations)
	{
		if (decl.id == stream)
			return &decl;
	}
	return NULL;
}

const DKVertexBuffer::Decl* DKVertexBuffer::Declaration(const DKFoundation::DKString& name) const
{
	for (const Decl& decl : declarations)
	{
		if (decl.name == name)
			return &decl;
	}
	return NULL;
}

const DKVertexBuffer::Decl* DKVertexBuffer::Declaration(DKVertexStream::Stream stream, const DKFoundation::DKString& name) const
{
	if (stream < DKVertexStream::StreamUserDefine)
	{
		for (const Decl& decl : declarations)
		{
			if (decl.id == stream)
				return &decl;
		}
	}
	else
	{
		for (const Decl& decl : declarations)
		{
			if (decl.id == DKVertexStream::StreamUserDefine && decl.name == name)
				return &decl;
		}
	}
	return NULL;
}

bool DKVertexBuffer::IsValid(void) const
{
	if (DKGeometryBuffer::IsValid())
	{
		if (vertexSize > 0 && declarations.Count() > 0)
			return true;
	}
	return false;
}

DKObject<DKBuffer> DKVertexBuffer::CopyStream(DKVertexStream::Stream stream) const
{
	if (stream < DKVertexStream::StreamUserDefine)
		return CopyStream(stream, L"");
	return NULL;
}

DKObject<DKBuffer> DKVertexBuffer::CopyStream(const DKFoundation::DKString& name) const
{
	if (name.Length() > 0)
		return CopyStream(DKVertexStream::StreamUserDefine, name);
	return NULL;
}

DKObject<DKBuffer> DKVertexBuffer::CopyStream(DKVertexStream::Stream stream, const DKFoundation::DKString& name) const
{
	if (IsValid())
	{
		size_t typeSize = 0;
		size_t offset = 0;
		for (const Decl& decl : declarations)
		{
			typeSize = DKVertexStream::TypeSize(decl.type);
			if (decl.id == stream && decl.name == name)
			{
				break;
			}
			offset += typeSize;
		}
		if (offset + typeSize > this->vertexSize)
		{
			DKLog("[%s] buffer doesn't have a stream (%d).\n", DKGL_FUNCTION_NAME, stream);
			return NULL;
		}

		DKObject<DKBuffer> ret = DKBuffer::Create(NULL, typeSize * vertexCount);
		if (ret == NULL)
		{
			DKLog("[%s] buffer allocation failed.\n", DKGL_FUNCTION_NAME);
			return NULL;
		}
		unsigned char* pSrc = reinterpret_cast<unsigned char*>(const_cast<DKVertexBuffer&>(*this).Lock(AccessModeReadOnly));
		if (pSrc == NULL)
		{
			DKLog("[%s] buffer lock failed.\n", DKGL_FUNCTION_NAME);
			return NULL;
		}
		unsigned char* pDst = (unsigned char*)ret->LockExclusive();

		for (int i = 0; i < vertexCount; i++)
		{
			memcpy(&pDst[i * typeSize], &pSrc[i * vertexSize + offset], typeSize);
		}
		const_cast<DKVertexBuffer&>(*this).Unlock();
		ret->UnlockExclusive();
		return ret;
	}
	return NULL;
}

bool DKVertexBuffer::UpdateContent(const void* data, size_t count, MemoryLocation m, BufferUsage u)
{
	size_t bufferSize = count * vertexSize;

	// 2011-08-23 :
	// UpdateSubContent() is slower. because it waits GL finishes use of buffer.
	// UpdateContent() is faster, it drops previous buffer whether it finished or not.
/*
	if (this->Size() > 0 && bufferSize == this->Size() && vertexCount == count && this->Location() == m && this->Usage() == u)
	{
		if (DKGeometryBuffer::UpdateSubContent(data, 0, bufferSize))
		{
			return true;
		}
	}
*/
	if (DKGeometryBuffer::UpdateContent(BufferTypeVertexArray, m, u, data, bufferSize))
	{
		vertexCount = count;
		return true;
	}
	return false;
}

size_t DKVertexBuffer::NumberOfDeclarations(void) const
{
	return declarations.Count();
}

size_t DKVertexBuffer::VertexSize(void) const
{
	return vertexSize;
}

size_t DKVertexBuffer::NumberOfVertices(void) const
{
	return vertexCount;
}

bool DKVertexBuffer::BindStream(const DKVertexStream& stream) const
{
	if (this->NumberOfVertices() == 0)
		return false;

	const Decl* d = NULL;
	size_t offset = 0;
	if (stream.id < DKVertexStream::StreamUserDefine)
	{
		for (const Decl& decl : declarations)
		{
			size_t typeSize = DKVertexStream::TypeSize(decl.type);
			if (decl.id == stream.id)
			{
				d = &decl;
				break;
			}
			offset += typeSize;
		}
	}
	else
	{
		for (const Decl& decl : declarations)
		{
			size_t typeSize = DKVertexStream::TypeSize(decl.type);
			if (decl.id == stream.id && decl.name == stream.name)
			{
				d = &decl;
				break;
			}
			offset += typeSize;
		}
	}

	if (d && this->Bind())
	{
		DKRenderState& state = DKOpenGLContext::RenderState();

		GLenum type;
		int components;

		// decl->type can be different to value type of shader.
		// value will be converted automatically if necessary.
		switch (d->type)
		{
		case DKVertexStream::TypeFloat1:		type = GL_FLOAT;			components = 1;		break;
		case DKVertexStream::TypeFloat2:		type = GL_FLOAT;			components = 2;		break;
		case DKVertexStream::TypeFloat3:		type = GL_FLOAT;			components = 3;		break;
		case DKVertexStream::TypeFloat4:		type = GL_FLOAT;			components = 4;		break;
		case DKVertexStream::TypeFloat2x2:		type = GL_FLOAT;			components = 4;		break;
		case DKVertexStream::TypeFloat3x3:		type = GL_FLOAT;			components = 9;		break;
		case DKVertexStream::TypeFloat4x4:		type = GL_FLOAT;			components = 16;	break;
		case DKVertexStream::TypeByte1:			type = GL_BYTE;				components = 1;		break;
		case DKVertexStream::TypeByte2:			type = GL_BYTE;				components = 2;		break;
		case DKVertexStream::TypeByte3:			type = GL_BYTE;				components = 3;		break;
		case DKVertexStream::TypeByte4:			type = GL_BYTE;				components = 4;		break;
		case DKVertexStream::TypeUByte1:		type = GL_UNSIGNED_BYTE;	components = 1;		break;
		case DKVertexStream::TypeUByte2:		type = GL_UNSIGNED_BYTE;	components = 2;		break;
		case DKVertexStream::TypeUByte3:		type = GL_UNSIGNED_BYTE;	components = 3;		break;
		case DKVertexStream::TypeUByte4:		type = GL_UNSIGNED_BYTE;	components = 4;		break;
		case DKVertexStream::TypeShort1:		type = GL_SHORT;			components = 1;		break;
		case DKVertexStream::TypeShort2:		type = GL_SHORT;			components = 2;		break;
		case DKVertexStream::TypeShort3:		type = GL_SHORT;			components = 3;		break;
		case DKVertexStream::TypeShort4:		type = GL_SHORT;			components = 4;		break;
		case DKVertexStream::TypeUShort1:		type = GL_UNSIGNED_SHORT;	components = 1;		break;
		case DKVertexStream::TypeUShort2:		type = GL_UNSIGNED_SHORT;	components = 2;		break;
		case DKVertexStream::TypeUShort3:		type = GL_UNSIGNED_SHORT;	components = 3;		break;
		case DKVertexStream::TypeUShort4:		type = GL_UNSIGNED_SHORT;	components = 4;		break;
		default:
			return false;
			break;
		}

		state.EnableVertexAttribArray(stream.location);
		glVertexAttribPointer(stream.location, components, type, (GLboolean)d->normalize, this->VertexSize(), (void*)offset);

		return true;
	}
	return false;
}

void DKVertexBuffer::StructuredLayout(DKFoundation::DKArray<DKVariant::StructElem>& layout, size_t& elementSize) const
{
	elementSize = this->vertexSize;
	layout.Clear();
	layout.Reserve(declarations.Count());

	for (const Decl& decl : declarations)
	{
		size_t baseSize = DKVertexStream::BaseTypeSize(decl.type);
		size_t typeSize = DKVertexStream::TypeSize(decl.type);

		size_t n = 0;

		switch (baseSize)
		{
		case 1:
		case 2:
		case 4:
		case 8:
			while (n + baseSize <= typeSize)
			{
				layout.Add( static_cast<DKVariant::StructElem>( baseSize ) );
				n += baseSize;
			}
			break;
		}

		while ( n < typeSize)
		{
			layout.Add( DKVariant::StructElem::Bypass1 );
			n++;
		}
	}
}

DKObject<DKSerializer> DKVertexBuffer::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKVertexBuffer* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKVertexBuffer");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKGeometryBuffer::Serializer(), NULL);

			this->Bind(L"declarations",
				DKFunction(this, &LocalSerializer::GetDecl),
				DKFunction(this, &LocalSerializer::SetDecl),
				DKFunction(this, &LocalSerializer::CheckDecl),
				NULL);

			this->Bind(L"vertexSize",
				DKFunction(this, &LocalSerializer::GetVertexSize),
				DKFunction(this, &LocalSerializer::SetVertexSize),
				DKFunction(this, &LocalSerializer::CheckVertexSize),
				NULL);

			this->Bind(L"vertexCount",
				DKFunction(this, &LocalSerializer::GetVertexCount),
				DKFunction(this, &LocalSerializer::SetVertexCount),
				DKFunction(this, &LocalSerializer::CheckVertexCount),
				NULL);

			return this;
		}
	private:
		void GetDecl(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeArray);
			for (const Decl& decl : target->declarations)
			{
				DKVariant declMap(DKVariant::TypePairs);
				declMap.Pairs().Insert(L"id", (DKVariant::VString)DKVertexStream::StreamToString(decl.id));
				declMap.Pairs().Insert(L"name", (DKVariant::VString)decl.name);
				declMap.Pairs().Insert(L"type", (DKVariant::VString)DKVertexStream::TypeToString(decl.type));
				declMap.Pairs().Insert(L"normalize", (DKVariant::VInteger)decl.normalize);
				v.Array().Add(declMap);
			}
		}
		void SetDecl(DKVariant& v)
		{
			target->declarations.Clear();
			
			for (const DKVariant& declMap : v.Array())
			{
				if (declMap.ValueType() == DKVariant::TypePairs)
				{
					Decl decl;
					const DKVariant::VPairs::Pair* pid = declMap.Pairs().Find(L"id");
					if (pid == NULL || pid->value.ValueType() != DKVariant::TypeString)
						continue;
					decl.id = DKVertexStream::StringToStream(pid->value.String());
					const DKVariant::VPairs::Pair* pname = declMap.Pairs().Find(L"name");
					if (pname == NULL || pname->value.ValueType() != DKVariant::TypeString)
						continue;
					decl.name = pname->value.String();
					const DKVariant::VPairs::Pair* ptype = declMap.Pairs().Find(L"type");
					if (ptype == NULL || ptype->value.ValueType() != DKVariant::TypeString)
						continue;
					decl.type = DKVertexStream::StringToType(ptype->value.String());
					const DKVariant::VPairs::Pair* pnorm = declMap.Pairs().Find(L"normalize");
					if (pnorm == NULL || pnorm->value.ValueType() != DKVariant::TypeInteger)
						continue;
					decl.normalize = pnorm->value.Integer() != 0;

					target->declarations.Add(decl);
				}
			}
		}
		bool CheckDecl(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeArray)
			{
				for (size_t i = 0; i < v.Array().Count(); ++i)
				{
					const DKVariant& v2 = v.Array().Value(i);
					if (v2.ValueType() != DKVariant::TypePairs)
						return false;
				}
				return true;
			}
			return false;
		}
		void GetVertexSize(DKVariant& v) const
		{
			v = (DKVariant::VInteger)target->vertexSize;
		}
		void SetVertexSize(DKVariant& v)
		{
			target->vertexSize = v.Integer();
		}
		bool CheckVertexSize(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeInteger;
		}
		void GetVertexCount(DKVariant& v) const
		{
			v = (DKVariant::VInteger)target->vertexCount;
		}
		void SetVertexCount(DKVariant& v)
		{
			target->vertexCount = v.Integer();
		}
		bool CheckVertexCount(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeInteger;
		}
		////////////////////////////////////////////////////////////////////////////////
		// callback
		void Callback(State s)
		{
			if (s == StateDeserializeSucceed)
			{
				glFlush();
			}
		}

		DKObject<DKVertexBuffer> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
