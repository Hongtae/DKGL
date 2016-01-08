//
//  File: DKIndexBuffer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKIndexBuffer.h"

using namespace DKFoundation;
using namespace DKFramework;


DKIndexBuffer::DKIndexBuffer(void)
	: primitiveType(DKPrimitive::TypePoints)
	, indexCount(0)
{
}

DKIndexBuffer::~DKIndexBuffer(void)
{
}

DKObject<DKIndexBuffer> DKIndexBuffer::Create(const uint8_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u)
{
	DKObject<DKIndexBuffer> buffer = DKObject<DKIndexBuffer>::New();
	if (buffer->UpdateContent(BufferTypeElementArray, m, u, indices, count))
	{
		buffer->primitiveType = p;
		buffer->indexCount = count;
		buffer->indexType = Type::UInt8;
		return buffer;
	}
	return NULL;
}

DKObject<DKIndexBuffer> DKIndexBuffer::Create(const uint16_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u)
{
	DKObject<DKIndexBuffer> buffer = DKObject<DKIndexBuffer>::New();
	if (buffer->UpdateContent(BufferTypeElementArray, m, u, indices, count * 2))
	{
		buffer->primitiveType = p;
		buffer->indexCount = count;
		buffer->indexType = Type::UInt16;
		return buffer;
	}
	return NULL;
}

DKObject<DKIndexBuffer> DKIndexBuffer::Create(const uint32_t* indices, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u)
{
	DKObject<DKIndexBuffer> buffer = DKObject<DKIndexBuffer>::New();
	if (buffer->UpdateContent(BufferTypeElementArray, m, u, indices, count * 4))
	{
		buffer->primitiveType = p;
		buffer->indexCount = count;
		buffer->indexType = Type::UInt32;
		return buffer;
	}
	return NULL;
}

DKObject<DKIndexBuffer> DKIndexBuffer::Create(const void* buffer, Type indexType, size_t count, DKPrimitive::Type p, MemoryLocation m, BufferUsage u)
{
	switch (indexType)
	{
	case Type::UInt8:
		return Create(reinterpret_cast<const uint8_t*>(buffer), count, p, m, u);
		break;
	case Type::UInt16:
		return Create(reinterpret_cast<const uint16_t*>(buffer), count, p, m, u);
		break;
	case Type::UInt32:
		return Create(reinterpret_cast<const uint32_t*>(buffer), count, p, m, u);
		break;
	}
	return NULL;
}

bool DKIndexBuffer::CopyIndices(DKArray<uint32_t>& indices) const
{
	if (IsValid())
	{
		DKObject<DKBuffer> data = CopyContent();
		if (data == NULL || data->Length() == 0)
		{
			DKLog("[%s] index buffer copy failed.\n", DKGL_FUNCTION_NAME);
			return false;
		}

		indices.Reserve(indices.Count() + NumberOfIndices());

		const void* p = data->LockShared();
		switch (indexType)
		{
		case Type::UInt8:
			for (int i = 0; i < NumberOfIndices(); i++)
				indices.Add(reinterpret_cast<const uint8_t*>(p)[i]);
			break;
		case Type::UInt16:
			for (int i = 0; i < NumberOfIndices(); i++)
				indices.Add(reinterpret_cast<const uint16_t*>(p)[i]);
			break;
		case Type::UInt32:
			for (int i = 0; i < NumberOfIndices(); i++)
				indices.Add(reinterpret_cast<const uint32_t*>(p)[i]);
			break;
		}
		data->UnlockShared();

		return true;
	}
	return false;
}

DKPrimitive::Type DKIndexBuffer::PrimitiveType(void) const
{
	return primitiveType;
}

size_t DKIndexBuffer::NumberOfIndices(void) const
{
	return indexCount;
}

DKIndexBuffer::Type DKIndexBuffer::IndexType(void) const
{
	return indexType;
}

void DKIndexBuffer::StructuredLayout(DKFoundation::DKArray<DKVariant::StructElem>& layout, size_t& elementSize) const
{
	switch (indexType)
	{
	case Type::UInt32:
		elementSize = sizeof(uint32_t);
		break;
	case Type::UInt16:
		elementSize = sizeof(uint16_t);
		break;
	default:
		elementSize = sizeof(uint8_t);
		break;
	}
	layout.Clear();
	layout.Add(static_cast<DKVariant::StructElem>(elementSize));
}

DKObject<DKSerializer> DKIndexBuffer::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKIndexBuffer* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKIndexBuffer");
			this->SetCallback(DKFunction(this, &LocalSerializer::Callback));

			this->Bind(L"super", target->DKGeometryBuffer::Serializer(), NULL);

			this->Bind(L"primitiveType",
				DKFunction(this, &LocalSerializer::GetPrimitiveType),
				DKFunction(this, &LocalSerializer::SetPrimitiveType),
				DKFunction(this, &LocalSerializer::CheckPrimitiveType),
				NULL);

			this->Bind(L"indexType",
				DKFunction(this, &LocalSerializer::GetIndexType),
				DKFunction(this, &LocalSerializer::SetIndexType),
				DKFunction(this, &LocalSerializer::CheckIndexType),
				NULL);

			this->Bind(L"indexCount",
				DKFunction(this, &LocalSerializer::GetIndexCount),
				DKFunction(this, &LocalSerializer::SetIndexCount),
				DKFunction(this, &LocalSerializer::CheckIndexCount),
				NULL);

			return this;
		}
	private:
		void GetPrimitiveType(DKVariant& v) const
		{
			v = (const DKVariant::VString&)DKPrimitive::TypeToString(target->primitiveType);
		}
		void SetPrimitiveType(DKVariant& v)
		{
			target->primitiveType = DKPrimitive::StringToType(v.String());
		}
		bool CheckPrimitiveType(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeString;
		}
		void GetIndexType(DKVariant& v) const
		{
			switch (target->indexType)
			{
			case Type::UInt8:		v = (DKVariant::VInteger)1;		break;
			case Type::UInt16:		v = (DKVariant::VInteger)2;		break;
			case Type::UInt32:		v = (DKVariant::VInteger)4;		break;
			}
		}
		void SetIndexType(DKVariant& v)
		{
			switch (v.Integer())
			{
			case 1:		target->indexType = Type::UInt8;		break;
			case 2:		target->indexType = Type::UInt16;		break;
			case 4:		target->indexType = Type::UInt32;		break;
			}
		}
		bool CheckIndexType(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeInteger && (v.Integer() == 1 || v.Integer() == 2 || v.Integer() == 4))
				return true;
			return false;
		}
		void GetIndexCount(DKVariant& v) const
		{
			v = (DKVariant::VInteger)target->indexCount;
		}
		void SetIndexCount(DKVariant& v)
		{
			target->indexCount = v.Integer();
		}
		bool CheckIndexCount(const DKVariant& v) const
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

		DKObject<DKIndexBuffer> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}
