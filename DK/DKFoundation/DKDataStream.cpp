//
//  File: DKDataStream.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <memory.h>
#include "DKDataStream.h"

using namespace DKFoundation;

DKDataStream::DKDataStream(void)
	: offset(0)
{
}

DKDataStream::DKDataStream(DKData* d)
	: offset(0)
	, data(d)
{
}

DKDataStream::DKDataStream(DKData& d)
	: offset(0)
	, data(&d)
{
}

DKDataStream::~DKDataStream(void)
{
}

DKStream::Position DKDataStream::SetPos(Position p)
{
	if (this->data)
	{
		size_t contentSize = this->data->Length();
		this->offset = Clamp(p, 0, contentSize);
	}
	else
	{
		this->offset = 0;
	}

	return this->offset;
}

DKStream::Position DKDataStream::GetPos(void) const
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

DKStream::Position DKDataStream::RemainLength(void) const
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

DKStream::Position DKDataStream::TotalLength(void) const
{
	if (this->data)
	{
		return this->data->Length();
	}
	return 0;
}

size_t DKDataStream::Read(void* p, size_t s)
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

size_t DKDataStream::Write(const void* p, size_t s)
{
	return 0;
}

DKData* DKDataStream::DataSource(void)
{
	return this->data;
}

const DKData* DKDataStream::DataSource(void) const
{
	return this->data;
}
