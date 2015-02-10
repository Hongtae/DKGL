//
//  File: DKVertexBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKGeometryBuffer.h"
#include "DKVertexStream.h"

////////////////////////////////////////////////////////////////////////////////
// DKVertexBuffer
// vertex buffer object.
// You need to declare vertex stream type before use. (DKVertexBuffer::Decl)
// Vertex stream type is defined in DKVertexStream. (see DKVertexStream.h)
// Once you declare your vertex stream types, declarations will not be modified.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKLIB_API DKVertexBuffer : public DKGeometryBuffer
	{
	public:
		struct Decl  // Vertex stream declaration.
		{
			DKVertexStream::Stream	id;
			DKFoundation::DKString	name;  // used if id is DKVertexStream::StreamUserDefine.
			DKVertexStream::Type	type;
			bool					normalize;  // normalize integer to float. (0.0~1.0 for unsigned, -1.0~1.0 for signed)
			size_t					offset;
		};

		DKVertexBuffer(void);
		~DKVertexBuffer(void);
		
		static DKFoundation::DKObject<DKVertexBuffer> Create(const Decl* decls, size_t declCount, const void* vertices, size_t size, size_t count, MemoryLocation m, BufferUsage u);
		template <typename T, typename U>
		static DKFoundation::DKObject<DKVertexBuffer> Create(const Decl* decls, size_t declCount, const DKFoundation::DKArray<T,U>& vertices, MemoryLocation m, BufferUsage u)
		{
			return Create(decls, declCount, (const T*)vertices, sizeof(T), vertices.Count(), m, u);
		}

		const Decl* DeclarationAtIndex(long index) const;
		const Decl* Declaration(DKVertexStream::Stream stream) const;       // find by id, not for DKVertexStream::StreamUserDefine
		const Decl* Declaration(const DKFoundation::DKString& name) const;	// find by name, for DKVertexStream::StreamUserDefine only.
		const Decl* Declaration(DKVertexStream::Stream stream, const DKFoundation::DKString& name) const; // find by id or name

		size_t NumberOfDeclarations(void) const;
		size_t VertexSize(void) const;
		size_t NumberOfVertices(void) const;

		DKFoundation::DKObject<DKFoundation::DKBuffer> CopyStream(DKVertexStream::Stream stream) const;
		DKFoundation::DKObject<DKFoundation::DKBuffer> CopyStream(const DKFoundation::DKString& name) const;
		DKFoundation::DKObject<DKFoundation::DKBuffer> CopyStream(DKVertexStream::Stream stream, const DKFoundation::DKString& name) const;

		bool IsValid(void) const;
		bool UpdateContent(const void* vertexData, size_t vertexCount, MemoryLocation m, BufferUsage u);

		DKFoundation::DKObject<DKSerializer> Serializer(void);

		bool BindStream(const DKVertexStream& stream) const;
	private:
		void UpdateDeclMap(void);
		typedef DKFoundation::DKMap<DKVertexStream::Stream, const Decl*> DeclMapById;
		typedef DKFoundation::DKMap<DKFoundation::DKString, const Decl*> DeclMapByName;

		DeclMapById declMapByStreamId;
		DeclMapByName declMapByStreamName;

		DKFoundation::DKArray<Decl>			declarations;
		size_t								vertexCount;   // number of vertices
		size_t								vertexSize;    // size of one vertex
	};
}
