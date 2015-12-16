//
//  File: DKData.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKData.h"
#include "DKFile.h"
#include "DKFunction.h"

using namespace DKFoundation;


DKObject<DKData> DKData::StaticData(void* p, size_t len, bool readonly, DKOperation* cleanup)
{
	struct ReadOnlyData : public DKData
	{
		const void* p;
		size_t len;
		DKObject<DKOperation> cleanup;

		~ReadOnlyData(void)
		{
			if (cleanup)
				cleanup->Perform();
		}
		size_t Length(void) const					{return len;}
		bool IsReadable(void) const					{return true;}
		bool IsWritable(void) const					{return false;}
		bool IsExcutable(void) const				{return false;}

		const void* LockShared(void) const			{ return p; }
		bool TryLockShared(const void** ptr) const	{ *ptr = p; return true; }
		void UnlockShared(void) const				{}
	};
	struct WritableData : public DKData
	{
		void* p;
		size_t len;
		DKObject<DKOperation> cleanup;
		DKSharedLock lock;

		~WritableData(void)
		{
			if (cleanup)
				cleanup->Perform();
		}

		size_t Length(void) const				{return len;}
		bool IsReadable(void) const				{return true;}
		bool IsWritable(void) const				{return true;}
		bool IsExcutable(void) const			{return false;}

		const void* LockShared(void) const		{ lock.LockShared(); return p; }
		bool TryLockShared(const void** ptr) const
		{
			if (lock.TryLockShared())
			{
				if (ptr)
					*ptr = p;
				return true;
			}
			return false;
		}
		void UnlockShared(void) const			{ lock.UnlockShared(); }

		virtual void* LockExclusive(void)		{ lock.Lock(); return p; }
		virtual bool TryLockExclusive(void** ptr)
		{
			if (lock.TryLock())
			{
				if (ptr)
					*ptr = p;
				return true;
			}
			return false;
		}
		virtual void UnlockExclusive(void)		{ lock.Unlock(); }
	};

	if (p && len > 0)
	{
		DKObject<DKData> output = NULL;
		if (readonly)
		{
			DKObject<ReadOnlyData> data = DKObject<ReadOnlyData>::New();
			data->p = p;
			data->len = len;
			data->cleanup = cleanup;
			output = data.SafeCast<DKData>();
		}
		else
		{
			DKObject<WritableData> data = DKObject<WritableData>::New();
			data->p = p;
			data->len = len;
			data->cleanup = cleanup;
			output = data.SafeCast<DKData>();
		}
		return output;
	}
	return NULL;
}

DKObject<DKData> DKData::StaticData(const void* p, size_t len, DKOperation* cleanup)
{
	return StaticData(const_cast<void*>(p), len, true, cleanup);
}

DKData::DKData(void)
{
}

DKData::~DKData(void)
{
}

bool DKData::WriteToFile(const DKString& file, bool overwrite) const
{
	if (this->Length() == 0)
		return false;

	DKFile::FileInfo info;
	if (DKFile::GetInfo(file, info) && !overwrite)
		return false;		// file is exists already.

	DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenNew, DKFile::ModeShareAll);
	if (f)
	{
		size_t written = f->Write(this);
		if (written < this->Length())
		{
			//f = NULL;
			//DKFile::Delete(file);
			return false;
		}
		return true;
	}
	return false;
}

bool DKData::WriteToStream(DKStream* stream) const
{
	if (stream && stream->IsWritable())
	{
		const void* p = this->LockShared();
		size_t len = this->Length();
		if (len > 0)
			stream->Write(p, len);
		this->UnlockShared();
		return true;
	}
	return false;
}
