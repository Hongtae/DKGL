//
//  File: DKBufferStream.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include "DKBufferStream.h"

using namespace DKFoundation;

DKBufferStream::DKBufferStream()
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

DKBufferStream::~DKBufferStream()
{
}

DKStream::Position DKBufferStream::SetCurrentPosition(Position p)
{
	if (p < TotalLength())
		this->offset = p;
	else
		return PositionError;
	return this->offset;
}

DKStream::Position DKBufferStream::CurrentPosition() const
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

DKStream::Position DKBufferStream::RemainLength() const
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

DKStream::Position DKBufferStream::TotalLength() const
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
			bytesRead = Min(s, contentSize - this->offset);
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
				if (!this->data->SetLength(contentSize2))
				{
					return PositionError;
				}
			}

			unsigned char* ptr = reinterpret_cast<unsigned char*>(this->data->LockExclusive());
			::memcpy(&ptr[this->offset], p, s);
			this->data->UnlockExclusive();
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

DKData* DKBufferStream::Data()
{
	return Buffer();
}

const DKData* DKBufferStream::Data() const
{
	return Buffer();
}

DKBuffer* DKBufferStream::Buffer()
{
	return this->data;
}

const DKBuffer* DKBufferStream::Buffer() const
{
	return this->data;
}
