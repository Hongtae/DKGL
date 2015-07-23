//
//  File: DKStaticMesh.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKOpenGLContext.h"
#include "DKStaticMesh.h"

using namespace DKFoundation;
using namespace DKFramework;

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
		this->streamIdMap = mesh->streamIdMap;
		this->streamNameMap = mesh->streamNameMap;
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
			if (this->FindVertexStream(streams.Value(j).id, streams.Value(j).name) == NULL)
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
			if (FindVertexStream(buffer->DeclarationAtIndex(i)->id, buffer->DeclarationAtIndex(i)->name))
				return false;
		}

		for (size_t i = 0; i < buffer->NumberOfDeclarations(); ++i)
		{
			const DKVertexBuffer::Decl* d = buffer->DeclarationAtIndex(i);

			StreamInfo si = {d, buffer};

			if (d->id < DKVertexStream::StreamUserDefine)
			{
				this->streamIdMap.Update(d->id, si);
			}
			else
			{
				this->streamNameMap.Update(d->name, si);
			}
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

void DKStaticMesh::RemoveVertexBuffer(DKVertexBuffer* buffer)
{
	for (size_t i = 0; i < vertexBuffers.Count(); ++i)
	{
		DKVertexBuffer* b = vertexBuffers.Value(i);
		if (b == buffer)
		{
			for (size_t j = 0; j < buffer->NumberOfDeclarations(); ++j)
			{
				const DKVertexBuffer::Decl* d = buffer->DeclarationAtIndex(j);
				if (d->id < DKVertexStream::StreamUserDefine)
					streamIdMap.Remove(d->id);
				else
					streamNameMap.Remove(d->name);
			}
			vertexBuffers.Remove(i);
			return;
		}
	}
}

void DKStaticMesh::RemoveAllVertexBuffers(void)
{
	vertexBuffers.Clear();
	streamIdMap.Clear();
	streamNameMap.Clear();
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

const DKStaticMesh::StreamInfo* DKStaticMesh::FindVertexStream(DKVertexStream::Stream stream) const
{
	const StreamIdMap::Pair* p = streamIdMap.Find(stream);
	if (p)
		return &p->value;
	return NULL;
}

const DKStaticMesh::StreamInfo* DKStaticMesh::FindVertexStream(const DKFoundation::DKString& name) const
{
	const StreamNameMap::Pair* p = streamNameMap.Find(name);
	if (p)
		return &p->value;
	return NULL;
}

const DKStaticMesh::StreamInfo* DKStaticMesh::FindVertexStream(DKVertexStream::Stream stream, const DKFoundation::DKString& name) const
{
	if (stream < DKVertexStream::StreamUserDefine)
		return FindVertexStream(stream);
	return FindVertexStream(name);
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
			decl.offset += vertexSize;
			decls.Add(decl);
		}
		vertexSize += buffer->VertexSize();
		vertexCount = vertexCount > 0 ? Min<size_t>(buffer->NumberOfVertices(), vertexCount) : buffer->NumberOfVertices();
	}
	if (vertexSize == 0 || vertexCount == 0)
		return false;

	// create temporary buffer
	void* buffer = DKMemoryHeapAlloc(vertexSize * vertexCount);
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
			DKMemoryHeapFree(buffer);
			DKLog("[%s] buffer lock failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		offset += pVB->VertexSize();
	}
	// create vertex buffer (interleaved)
	DKObject<DKVertexBuffer> interleaved = DKVertexBuffer::Create(decls, decls.Count(), buffer, vertexSize, vertexCount, location, usage);

	DKMemoryHeapFree(buffer);

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
			DKVertexBuffer::Decl d = {decl.id, decl.name, decl.type, decl.normalize, 0};
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

	const StreamInfo* si = FindVertexStream(stream, name);
	if (si)	 // buffer exists, update buffer
	{
		if (si->buffer->IsLocked(0))
		{
			DKLog("[%s] vertex buffer locked.\n", DKGL_FUNCTION_NAME);
			return false;
		}
		if (si->buffer->NumberOfVertices() != vertexCount)
		{
			DKLog("[%s] vertex count mismatch!\n", DKGL_FUNCTION_NAME);
			return false;
		}
		const DKVertexBuffer::Decl* d = si->decl;
		if (d->type != type || si->buffer->Location() != location || si->buffer->Usage() != usage)
		{
			if (RemoveStream(stream, name))
				return UpdateStream(stream, name, type, normalize, vertexSize, vertexCount, data, location, usage);

			DKLog("[%s] vertex type mismatch! (Failed to remove old data)\n", DKGL_FUNCTION_NAME);
			return false;
		}

		unsigned char* p = (unsigned char*)si->buffer->Lock(DKIndexBuffer::AccessModeWriteOnly);
		if (p)
		{
			for (int i = 0; i < vertexCount; i++)
			{
				memcpy(&p[i * si->buffer->VertexSize() + d->offset], &((unsigned char*)data)[i * vertexSize], vertexSize);
			}
			si->buffer->Unlock();
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
		DKVertexBuffer::Decl d = {stream, name, type, normalize, 0};
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

bool DKStaticMesh::RemoveStream(DKVertexStream::Stream stream, const DKFoundation::DKString& name)
{
	const StreamInfo* si = FindVertexStream(stream, name);
	if (si)
	{
		if (si->buffer->NumberOfDeclarations() == 1)
		{
			RemoveVertexBuffer(si->buffer);
			return true;
		}

		// decls1 : original declarations.
		// decl2 : declarations without specified stream.
		DKArray<DKVertexBuffer::Decl> decls;
		size_t vertexSize = 0;
		for (int i = 0; i < si->buffer->NumberOfDeclarations(); i++)
		{
			DKVertexBuffer::Decl d = *si->buffer->DeclarationAtIndex(i);
			if (d.id != stream)
			{
				d.offset = vertexSize;
				decls.Add(d);
				vertexSize += DKVertexStream::TypeSize(d.type);
			}
		}
		size_t vertexCount = si->buffer->NumberOfVertices();

		DKArray<unsigned char> vertexData;	 // temporary buffer
		vertexData.Resize(vertexSize * vertexCount);

		unsigned char* pSrc = (unsigned char*)si->buffer->Lock(DKVertexBuffer::AccessModeReadOnly);
		unsigned char* pDst = (unsigned char*)vertexData;
		if (pSrc)
		{
			for (int i = 0; i < vertexCount; i++)
			{
				size_t offset = 0;
				for (int k = 0; k < si->buffer->NumberOfDeclarations(); k++)
				{
					DKVertexBuffer::Decl d = *si->buffer->DeclarationAtIndex(k);
					if (d.id != stream)
					{
						size_t s = DKVertexStream::TypeSize(d.type);
						memcpy(&pDst[i * vertexSize + offset], &pSrc[i * si->buffer->VertexSize() + d.offset], s);
						offset += s;
					}
				}
			}
			si->buffer->Unlock();

			DKObject<DKVertexBuffer> newBuffer = DKVertexBuffer::Create(decls, decls.Count(), (const unsigned char*)vertexData, vertexSize, vertexCount, si->buffer->Location(), si->buffer->Usage());
			if (newBuffer)
			{
				DKObject<DKVertexBuffer> tmp = si->buffer;	// to prevent automatic delete
				RemoveVertexBuffer(si->buffer);				// si is no longer accessible

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
	const StreamInfo* si = FindVertexStream(vs.id, vs.name);
	if (si && si->buffer->BindStream(vs))
	{
		return si->buffer->NumberOfVertices();
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
