//
//  File: DKBufferStream.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include "DKBufferStream.h"

using namespace DKFoundation;

DKBufferStream::DKBufferStream(void)
	: offset(0)
{
}

DKBufferStream::DKBufferStream(DKBuffer& b)
	: offset(0)
	, data(&b)
{
}

DKBufferStream::DKBufferStream(DKBuffer* b)
	: offset(0)
	, data(b)
{
}

DKBufferStream::~DKBufferStream(void)
{
}

DKStream::Position DKBufferStream::SetPos(Position p)
{
	if (this->data)
	{
		size_t contentSize = this->data->Length();
		this->offset = Clamp<Position>(p, 0, contentSize);
	}
	else
	{
		this->offset = 0;
	}

	return this->offset;
}

DKStream::Position DKBufferStream::GetPos(void) const
{
	if (this->data)
	{
		size_t contentSize = this->data->Length();
		if (this->offset > contentSize)
			return contentSize;
		return this->offset;
	}
	return 0;
}

DKStream::Position DKBufferStream::RemainLength(void) const
{
	if (this->data)
	{
		size_t contentSize = this->data->Length();
		if (this->offset > contentSize)
			return 0;
		return contentSize - this->offset;
	}
	return 0;
}

DKStream::Position DKBufferStream::TotalLength(void) const
{
	if (this->data)
	{
		return this->data->Length();
	}
	return 0;
}

size_t DKBufferStream::Read(void* p, size_t s)
{
	if (this->data)
	{
		size_t bytesRead = 0;
		const char* ptr = reinterpret_cast<const char*>(this->data->LockShared());
		size_t contentSize = this->data->Length();
		if (this->offset > contentSize)
		{
			this->offset = contentSize; // adjust position.
		}
		else
		{
			bytesRead = Min<size_t>(s, contentSize - this->offset);
			memcpy(p, &ptr[this->offset], bytesRead);
			this->offset = this->offset + bytesRead;
		}
		this->data->UnlockShared();
		return bytesRead;
	}
	else
	{
		this->offset = 0;
	}
	return 0;
}

size_t DKBufferStream::Write(const void* p, size_t s)
{
	if (p && s)
	{
		if (this->data)
		{
			size_t contentSize1 = this->data->Length();
			size_t contentSize2 = this->offset + s;
			if (contentSize2 > contentSize1) // extend
			{
				DKObject<DKBuffer> data2 = DKBuffer::Create(NULL, contentSize2);

				const unsigned char* ptr1 = reinterpret_cast<const unsigned char*>(this->data->LockShared());
				unsigned char* ptr2 = reinterpret_cast<unsigned char*>(data2->LockExclusive());

				::memcpy(ptr2, ptr1, this->offset);
				::memcpy(&ptr2[this->offset], p, s);

				data2->UnlockExclusive();
				this->data->UnlockShared();

				this->data->SetContent(data2);
			}
			else
			{
				unsigned char* ptr = reinterpret_cast<unsigned char*>(this->data->LockExclusive());
				::memcpy(&ptr[this->offset], p, s);
				this->data->UnlockExclusive();
			}
			this->offset = contentSize2;
		}
		else
		{
			this->data = DKBuffer::Create(p, s);
			this->offset = s;
		}
		return s;
	}
	return 0;
}

void DKBufferStream::ResetStream(const void* p, size_t s)
{
	if (p && s)
	{
		this->data = DKBuffer::Create(p, s);
		offset = 0;
	}
	else
	{
		this->data = NULL;
		offset = 0;
	}
}

void DKBufferStream::ResetStream(DKBuffer* d)
{
	this->data = d;
	this->offset = 0;
}

DKData* DKBufferStream::DataSource(void)
{
	return BufferObject();
}

const DKData* DKBufferStream::DataSource(void) const
{
	return BufferObject();
}

DKBuffer* DKBufferStream::BufferObject(void)
{
	return this->data;
}

const DKBuffer* DKBufferStream::BufferObject(void) const
{
	return this->data;
}
