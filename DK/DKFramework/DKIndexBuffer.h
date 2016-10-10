//
//  File: DKIndexBuffer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKGeometryBuffer.h"
#include "DKPrimitiveIndex.h"

////////////////////////////////////////////////////////////////////////////////
// DKIndexBuffer
// a index buffer defines primitive type and have indices of vertices
// (see DKPrimitive.h for primitive types)
// (see DKGeometryBuffer.h for accessing content)
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKIndexBuffer : public DKGeometryBuffer
	{
	public:
		enum class Type
		{
			UInt8,
			UInt16,		// fastest
			UInt32,
		};

		DKIndexBuffer(void);
		~DKIndexBuffer(void);

		static DKObject<DKIndexBuffer> Create(const uint8_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u);
		static DKObject<DKIndexBuffer> Create(const uint16_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u);
		static DKObject<DKIndexBuffer> Create(const uint32_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u);
		static DKObject<DKIndexBuffer> Create(const void* buffer, Type indexType, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u);
		
		DKPrimitive::Type	PrimitiveType(void) const;
		size_t				NumberOfIndices(void) const;
		Type				IndexType(void) const;
		bool				CopyIndices(DKArray<uint32_t>& indices) const;

		DKObject<DKSerializer> Serializer(void);

	protected:
		void StructuredLayout(DKArray<DKVariant::StructElem>& layout, size_t& elementSize) const override;

	private:
		DKPrimitive::Type	primitiveType;
		Type				indexType;
		size_t				indexCount;
	};
}
