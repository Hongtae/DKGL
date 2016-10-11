//
//  File: DKStaticMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../lib/Inc_OpenGL.h"
#include "DKOpenGLContext.h"
#include "DKStaticMesh.h"

using namespace DKGL;
using namespace DKGL;

DKStaticMesh::DKStaticMesh(void)
{
}

DKStaticMesh::~DKStaticMesh(void)
{
}

DKObject<DKModel> DKStaticMesh::Clone(UUIDObjectMap& uuids) const
{
	return DKObject<DKStaticMesh>::New()->Copy(uuids, this);
}

DKStaticMesh* DKStaticMesh::Copy(UUIDObjectMap& uuids, const DKStaticMesh* mesh)
{
	if (DKMesh::Copy(uuids, mesh))
	{
		this->vertexBuffers = mesh->vertexBuffers;
		this->indexBuffer = mesh->indexBuffer;
		return this;
	}
	return NULL;
}

bool DKStaticMesh::CanAdoptMaterial(const DKMaterial* m) const
{
	if (m == NULL || m->IsValid() == false)
		return false;

	for (size_t i = 0; i < m->renderingProperties.Count(); ++i)
	{
		const DKArray<DKVertexStream>& streams = m->StreamArray(i);
		for (size_t j = 0; j < streams.Count(); ++j)
		{
			if (this->FindVertexStream(streams.Value(j).id, streams.Value(j).name).decl == NULL)
				return false;
		}
	}
	return true;
}

bool DKStaticMesh::AddVertexBuffer(DKVertexBuffer* buffer)
{
	if (buffer && buffer->NumberOfDeclarations() > 0)
	{
		for (size_t i = 0; i < buffer->NumberOfDeclarations(); ++i)
		{
			if (FindVertexStream(buffer->DeclarationAtIndex(i)->id, buffer->DeclarationAtIndex(i)->name).decl)
				return false;
		}

		this->vertexBuffers.Add(buffer);
		return true;
	}
	return false;
}

size_t DKStaticMesh::NumberOfVertexBuffers(void) const
{
	return this->vertexBuffers.Count();
}

DKVertexBuffer* DKStaticMesh::VertexBufferAtIndex(unsigned int index)
{
	DKASSERT_DEBUG(index < this->vertexBuffers.Count());
	return this->vertexBuffers.Value(index);
}

const DKVertexBuffer* DKStaticMesh::VertexBufferAtIndex(unsigned int index) const
{
	DKASSERT_DEBUG(index < this->vertexBuffers.Count());
	return this->vertexBuffers.Value(index);
}

void DKStaticMesh::RemoveVertexBuffer(const DKVertexBuffer* buffer)
{
	for (size_t i = 0; i < vertexBuffers.Count(); ++i)
	{
		DKVertexBuffer* b = vertexBuffers.Value(i);
		if (b == buffer)
		{
			vertexBuffers.Remove(i);
			return;
		}
	}
}

void DKStaticMesh::RemoveAllVertexBuffers(void)
{
	vertexBuffers.Clear();
}

void DKStaticMesh::SetIndexBuffer(DKIndexBuffer* buffer)
{
	indexBuffer = buffer;
}

DKIndexBuffer* DKStaticMesh::IndexBuffer(void)
{
	return indexBuffer;
}

const DKIndexBuffer* DKStaticMesh::IndexBuffer(void) const
{
	return indexBuffer;
}

DKPrimitive::Type DKStaticMesh::PrimitiveType(void) const
{
	if (this->indexBuffer && this->indexBuffer->NumberOfIndices() > 0)
		return this->indexBuffer->PrimitiveType();

	return DKMesh::PrimitiveType();
}

DKStaticMesh::StreamInfo DKStaticMesh::FindVertexStream(DKVertexStream::Stream stream) const
{
	size_t offset;
	for (const DKVertexBuffer* buffer : vertexBuffers)
	{
		offset = 0;
		for (int i = 0; i < buffer->NumberOfDeclarations(); ++i)
		{
			const DKVertexBuffer::Decl* decl = buffer->DeclarationAtIndex(i);
			if (decl->id == stream)
			{
				return{ decl, buffer, offset };
			}
			offset += DKVertexStream::TypeSize(decl->type);
		}
	}
	return{ NULL, NULL, 0 };
}

DKStaticMesh::StreamInfo DKStaticMesh::FindVertexStream(const DKString& name) const
{
	size_t offset;
	for (const DKVertexBuffer* buffer : vertexBuffers)
	{
		offset = 0;
		for (int i = 0; i < buffer->NumberOfDeclarations(); ++i)
		{
			const DKVertexBuffer::Decl* decl = buffer->DeclarationAtIndex(i);
			if (decl->name == name)
			{
				return{ decl, buffer, offset };
			}
			offset += DKVertexStream::TypeSize(decl->type);
		}
	}
	return{ NULL, NULL, 0 };
}

DKStaticMesh::StreamInfo DKStaticMesh::FindVertexStream(DKVertexStream::Stream stream, const DKString& name) const
{
	size_t offset;
	if (stream < DKVertexStream::StreamUserDefine)
	{
		for (const DKVertexBuffer* buffer : vertexBuffers)
		{
			offset = 0;
			for (int i = 0; i < buffer->NumberOfDeclarations(); ++i)
			{
				const DKVertexBuffer::Decl* decl = buffer->DeclarationAtIndex(i);
				if (decl->id == stream)
				{
					return{ decl, buffer, offset };
				}
				offset += DKVertexStream::TypeSize(decl->type);
			}
		}
	}
	else
	{
		for (const DKVertexBuffer* buffer : vertexBuffers)
		{
			offset = 0;
			for (int i = 0; i < buffer->NumberOfDeclarations(); ++i)
			{
				const DKVertexBuffer::Decl* decl = buffer->DeclarationAtIndex(i);
				if (decl->id == DKVertexStream::StreamUserDefine && decl->name == name)
				{
					return{ decl, buffer, offset };
				}
				offset += DKVertexStream::TypeSize(decl->type);
			}
		}
	}
	return{ NULL, NULL, 0 };
}

bool DKStaticMesh::MakeInterleaved(DKVertexBuffer::MemoryLocation location, DKVertexBuffer::BufferUsage usage)
{
	if (vertexBuffers.IsEmpty())
		return false;

	// calculate new buffer length
	size_t vertexSize = 0;
	size_t vertexCount = 0;

	DKArray<DKVertexBuffer::Decl> decls;
	for (int i = 0; i < vertexBuffers.Count(); i++)
	{
		DKVertexBuffer* buffer = vertexBuffers.Value(i);
		if (!buffer->IsValid())
		{
			DKLog("[%s] buffer is not valid.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		if (buffer->IsLocked(0))		// buffer is locked.
		{
			DKLog("[%s] buffer locked.\n", DKGL_FUNCTION_NAME);
			return false;
		}

		for (int k = 0; k < buffer->NumberOfDeclarations(); k++)
		{
			DKVertexBuffer::Decl decl = *buffer->DeclarationAtIndex(k);
			decls.Add(decl);
		}
		vertexSize += buffer->VertexSize();
		vertexCount = vertexCount > 0 ? Min<size_t>(buffer->NumberOfVertices(), vertexCount) : buffer->NumberOfVertices();
	}
	if (vertexSize == 0 || vertexCount == 0)
		return false;

	// create temporary buffer
	void* buffer = DKMemoryDefaultAllocator::Alloc(vertexSize * vertexCount);
	size_t offset = 0;
	for (int i = 0; i < vertexBuffers.Count(); i++)
	{
		DKVertexBuffer* pVB = vertexBuffers.Value(i);
		unsigned char* pSrc = reinterpret_cast<unsigned char*>(pVB->Lock(DKVertexBuffer::AccessModeReadOnly));
		unsigned char* pDst = reinterpret_cast<unsigned char*>(buffer);
		if (pSrc)
		{
			for (int k = 0; k < vertexCount; k++)
			{
				memcpy(&pDst[k * vertexSize + offset], &pSrc[k * pVB->VertexSize()], pVB->VertexSize());
			}
			pVB->Unlock();
		}
		else
		{
			DKMemoryDefaultAllocator::Free(buffer);
			DKLog("[%s] buffer lock failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		offset += pVB->VertexSize();
	}
	// create vertex buffer (interleaved)
	DKObject<DKVertexBuffer> interleaved = DKVertexBuffer::Create(decls, decls.Count(), buffer, vertexSize, vertexCount, location, usage);

	DKMemoryDefaultAllocator::Free(buffer);

	if (interleaved)
	{
		RemoveAllVertexBuffers();
		if (AddVertexBuffer(interleaved) == false)
		{
			DKERROR_THROW("Unable to recover previous state!");
		}
		return true;
	}
	return false;
}

bool DKStaticMesh::MakeSeparated(DKVertexBuffer::MemoryLocation location, DKVertexBuffer::BufferUsage usage)
{
	if (vertexBuffers.IsEmpty())
		return false;

	DKArray<DKObject<DKVertexBuffer>> separated;
	for (size_t i = 0; i < vertexBuffers.Count(); ++i)
	{
		DKVertexBuffer* pVB = vertexBuffers.Value(i);

		// buffer is locked. (and buffer has more than one declarations.)
		if (pVB->IsLocked(0) && pVB->NumberOfDeclarations() > 1)
		{
			DKLog("[%s] buffer locked.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		for (int k = 0; k < pVB->NumberOfDeclarations(); k++)
		{
			DKVertexBuffer::Decl decl = *pVB->DeclarationAtIndex(k);
			DKObject<DKBuffer> buffer = pVB->CopyStream(decl.id, decl.name);
			if (buffer == NULL || buffer->Length() == 0)
			{
				DKLog("[%s] failed to copy stream.\n", DKGL_FUNCTION_NAME);
				return false;
			}
			DKVertexBuffer::Decl d = { decl.id, decl.name, decl.type, decl.normalize };
			DKObject<DKVertexBuffer> newBuffer = DKVertexBuffer::Create(&d, 1, buffer->LockShared(), DKVertexStream::TypeSize(d.type), pVB->NumberOfVertices(), location, usage);
			buffer->UnlockShared();

			if (newBuffer == NULL)
			{
				DKLog("[%s] failed to create vertex buffer.\n", DKGL_FUNCTION_NAME);
				return false;
			}
			separated.Add(newBuffer);
		}
	}

	// remove all buffers, and add new buffers.
	RemoveAllVertexBuffers();
	for (size_t i = 0; i < separated.Count(); ++i)
	{
		if (AddVertexBuffer(separated.Value(i)) == false)
		{
			DKERROR_THROW("Unable to recover previous state!");
		}
	}
	return true;
}

bool DKStaticMesh::UpdateStream(DKVertexStream::Stream stream, const DKString& name, DKVertexStream::Type type, bool normalize, size_t vertexSize, size_t vertexCount, void* data, DKVertexBuffer::MemoryLocation location, DKVertexBuffer::BufferUsage usage)
{
	if (data == NULL || vertexCount == 0 || vertexSize == 0)
		return RemoveStream(stream, name);

	StreamInfo si = FindVertexStream(stream, name);
	if (si.decl && si.buffer)	 // buffer exists, update buffer
	{
		if (si.buffer->IsLocked(0))
		{
			DKLog("[%s] vertex buffer locked.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		if (si.buffer->NumberOfVertices() != vertexCount)
		{
			DKLog("[%s] vertex count mismatch!\n", DKGL_FUNCTION_NAME);
			return false;
		}
		if (si.decl->type != type || si.buffer->Location() != location || si.buffer->Usage() != usage)
		{
			if (RemoveStream(stream, name))
				return UpdateStream(stream, name, type, normalize, vertexSize, vertexCount, data, location, usage);

			DKLog("[%s] vertex type mismatch! (Failed to remove old data)\n", DKGL_FUNCTION_NAME);
			return false;
		}

		DKVertexBuffer* vb = const_cast<DKVertexBuffer*>(si.buffer);
		uint8_t* p = (uint8_t*)vb->Lock(DKIndexBuffer::AccessModeWriteOnly);
		if (p)
		{
			for (int i = 0; i < vertexCount; i++)
			{
				memcpy(&p[i * si.buffer->VertexSize() + si.offset], &((unsigned char*)data)[i * vertexSize], vertexSize);
			}
			vb->Unlock();
			DKLog("[%s] buffer updated.\n", DKGL_FUNCTION_NAME);
			return true;
		}
		else
		{
			DKLog("[%s] lock failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}
	}
	else	// buffer not exists, create new one
	{
		DKVertexBuffer::Decl d = { stream, name, type, normalize };
		DKObject<DKVertexBuffer> newBuffer = DKVertexBuffer::Create(&d, 1, data, vertexSize, vertexCount, location, usage);
		if (newBuffer)
		{
			if (AddVertexBuffer(newBuffer))
			{
				DKLog("[%s] generated.\n", DKGL_FUNCTION_NAME);
				return true;
			}
			return false;
		}
		else
		{
			DKLog("[%s] vertex buffer creation failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}
	}

	return false;
}

bool DKStaticMesh::RemoveStream(DKVertexStream::Stream stream, const DKString& name)
{
	StreamInfo si = FindVertexStream(stream, name);
	if (si.buffer)
	{
		if (si.buffer->NumberOfDeclarations() == 1)
		{
			RemoveVertexBuffer(si.buffer);
			return true;
		}

		// decls1 : original declarations.
		// decl2 : declarations without specified stream.
		DKArray<DKVertexBuffer::Decl> decls;
		size_t vertexSize = 0;
		for (int i = 0; i < si.buffer->NumberOfDeclarations(); i++)
		{
			DKVertexBuffer::Decl d = *si.buffer->DeclarationAtIndex(i);
			if (d.id != stream)
			{
				decls.Add(d);
				vertexSize += DKVertexStream::TypeSize(d.type);
			}
		}
		size_t vertexCount = si.buffer->NumberOfVertices();

		DKArray<unsigned char> vertexData;	 // temporary buffer
		vertexData.Resize(vertexSize * vertexCount);

		DKVertexBuffer* vb = const_cast<DKVertexBuffer*>(si.buffer);

		unsigned char* pSrc = (unsigned char*)vb->Lock(DKVertexBuffer::AccessModeReadOnly);
		unsigned char* pDst = (unsigned char*)vertexData;
		if (pSrc)
		{
			for (int i = 0; i < vertexCount; i++)
			{
				size_t offset = 0;
				size_t declOffset = 0;
				for (int k = 0; k < vb->NumberOfDeclarations(); k++)
				{
					DKVertexBuffer::Decl d = *vb->DeclarationAtIndex(k);
					size_t s = DKVertexStream::TypeSize(d.type);
					if (d.id != stream)
					{
						memcpy(&pDst[i * vertexSize + offset], &pSrc[i * vb->VertexSize() + declOffset], s);
						offset += s;
					}
					declOffset += s;
				}
			}
			vb->Unlock();

			DKObject<DKVertexBuffer> newBuffer = DKVertexBuffer::Create(decls, decls.Count(), (const unsigned char*)vertexData, vertexSize, vertexCount, vb->Location(), vb->Usage());
			if (newBuffer)
			{
				DKObject<DKVertexBuffer> tmp = vb;	// to prevent automatic delete
				RemoveVertexBuffer(vb);				// si is no longer accessible

				if (AddVertexBuffer(newBuffer))
					return true;

				// buffer insertion failed??, restore!
				DKLog("[%s] AddVertexBuffer failed.\n", DKGL_FUNCTION_NAME);
				if (AddVertexBuffer(tmp))
					DKLog("[%s] buffer restored.\n", DKGL_FUNCTION_NAME);
				else
					DKLog("[%s] critical error: buffer lost.\n", DKGL_FUNCTION_NAME);
				return false;

			}
			else
			{
				DKLog("[%s] vertex buffer creation failed.\n", DKGL_FUNCTION_NAME);
				return false;
			}
		}
		else
		{
			DKLog("[%s] vertex buffer lock failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}
	}
	return false;
}

int DKStaticMesh::BindStream(const DKVertexStream& vs) const
{
	StreamInfo si = FindVertexStream(vs.id, vs.name);
	if (si.buffer && si.buffer->BindStream(vs))
	{
		return si.buffer->NumberOfVertices();
	}
	//state.DisableVertexAttribArray(vs.location); // disable to restore initial value=vec4(0,0,0,1)
	return 0;
}

bool DKStaticMesh::BindPrimitiveIndex(DKPrimitive::Type* primType, int* numIndices, DKIndexBuffer::Type* indexType) const
{
	if (indexBuffer && indexBuffer->NumberOfIndices())
	{
		DKPrimitive::Type p = indexBuffer->PrimitiveType();
		if (indexBuffer->Bind() == false)
		{
			DKLOG_DEBUG("failed to bind index buffer!\n");
			return false;
		}
		if (primType)
			*primType = p;
		if (numIndices)
			*numIndices = indexBuffer->NumberOfIndices();
		if (indexType)
			*indexType = indexBuffer->IndexType();
		return true;
	}
	else
	{
		DKRenderState&	state = DKOpenGLContext::RenderState();
		state.BindIndexBuffer(0);
		if (primType)
			*primType = DefaultPrimitiveType();
		if (numIndices)
			*numIndices = 0;
		return true;
	}
	return false;
}


DKObject<DKSerializer> DKStaticMesh::Serializer(void)
{
	struct LocalSerializer : public DKSerializer
	{
		DKObject<DKStaticMesh> target;

		////////////////////////////////////////////////////////////////////////////////
		// vertexBuffers
		void GetVertexBuffers(ExternalArrayType& v)
		{
			v.Reserve(target->NumberOfVertexBuffers());
			for (size_t i = 0; i < target->NumberOfVertexBuffers(); ++i)
				v.Add(target->VertexBufferAtIndex(i));
		}
		void SetVertexBuffers(ExternalArrayType& v)
		{
			target->RemoveAllVertexBuffers();
			for (size_t i = 0; i < v.Count(); ++i)
			{
				DKVertexBuffer* vb = v.Value(i).SafeCast<DKVertexBuffer>();
				if (vb)
					target->AddVertexBuffer(vb);
			}
		}
		////////////////////////////////////////////////////////////////////////////////
		// indexBuffer
		void GetIndexBuffer(ExternalType& v)
		{
			v = target->IndexBuffer();
		}
		void SetIndexBuffer(ExternalType& v)
		{
			target->SetIndexBuffer(v.SafeCast<DKIndexBuffer>());
		}
		void ResetIndexBuffer(void)
		{
			target->SetIndexBuffer(NULL);
		}
		bool CheckIndexBuffer(const ExternalType& v) const
		{
			return v.SafeCast<DKIndexBuffer>() != NULL;
		}

		DKSerializer* Init(DKStaticMesh* p)
		{
			if (p == NULL)
				return NULL;
			target = p;

			this->SetResourceClass(L"DKStaticMesh");

			this->Bind(L"super", target->DKMesh::Serializer(), NULL);

			this->Bind(L"vertexBuffers",
				DKFunction(this, &LocalSerializer::GetVertexBuffers),
				DKFunction(this, &LocalSerializer::SetVertexBuffers),
				NULL,
				ExternalResourceInclude,
				NULL);

			this->Bind(L"indexBuffer",
				DKFunction(this, &LocalSerializer::GetIndexBuffer),
				DKFunction(this, &LocalSerializer::SetIndexBuffer),
				DKFunction(this, &LocalSerializer::CheckIndexBuffer),
				ExternalResourceInclude,
				DKFunction(this, &LocalSerializer::ResetIndexBuffer)->Invocation());

			return this;
		}
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
