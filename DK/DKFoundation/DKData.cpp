//
//  File: DKData.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKData.h"
#include "DKFile.h"
#include "DKFunction.h"

using namespace DKFoundation;

DKObject<DKData> DKData::StaticData(const void* p, size_t len, bool readonly, DKOperation* cleanup)
{
	struct SData : public DKData
	{
		SData(void) : p(NULL), len(0), readonly(false), cleanup(NULL) {}
		~SData(void)
		{
			if (cleanup)
				cleanup->Perform();
		}
		void* LockContent(void)					{return p;}
		void UnlockContent(void)				{}
		size_t Length(void) const				{return len;}
		virtual bool IsReadable(void) const		{return true;}
		virtual bool IsWritable(void) const		{return !readonly;}
		virtual bool IsExcutable(void) const	{return false;}

		void* p;
		size_t len;
		bool readonly;
		DKObject<DKOperation> cleanup;
	};

	DKObject<SData> sd = DKObject<SData>::New();
	if (p && len > 0)
	{
		sd->p = const_cast<void*>(p);
		sd->len = len;
	}
	else
	{
		sd->p = NULL;
		sd->len = 0;
	}
	sd->readonly = readonly;
	sd->cleanup = cleanup;

	return sd.SafeCast<DKData>();
}

DKData::DKData(void)
	: numShared(0)
	, sharedPtr(NULL)
{
}

DKData::~DKData(void)
{
	DKASSERT_DEBUG(numShared == 0);
	DKASSERT_DEBUG(sharedPtr == NULL);

#ifdef DKGL_DEBUG_ENABLED
	if (sharedLock.TryLock())
	{
		sharedLock.Unlock();
	}
	else
	{
		DKERROR_THROW("Data Locked!");
	}
#endif
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

const void* DKData::LockShared(void) const
{
	sharedLock.LockShared();

	DKCriticalSection<DKSpinLock> guard(this->spinLock);
	if (numShared == 0)
		sharedPtr = const_cast<DKData*>(this)->LockContent();
	numShared++;
	return sharedPtr;
}

bool DKData::TryLockShared(const void** p) const
{
	if (sharedLock.TryLockShared())
	{
		DKCriticalSection<DKSpinLock> guard(this->spinLock);
		if (numShared == 0)
			sharedPtr = const_cast<DKData*>(this)->LockContent();
		numShared++;
		if (p)
			*p = sharedPtr;
	}
	return false;
}

void DKData::UnlockShared(void) const
{
	DKCriticalSection<DKSpinLock> guard(this->spinLock);
	DKASSERT_DEBUG(numShared > 0);

	sharedLock.UnlockShared();
	if (numShared == 1)
	{
		const_cast<DKData*>(this)->UnlockContent();
		sharedPtr = NULL;
	}
	numShared--;
}

void* DKData::LockExclusive(void)
{
	sharedLock.Lock();

	DKCriticalSection<DKSpinLock> guard(this->spinLock);
	DKASSERT_DEBUG(numShared == 0);
	DKASSERT_DEBUG(sharedPtr == NULL);

	return this->LockContent();
}

bool DKData::TryLockExclusive(void** p)
{
	if (sharedLock.TryLock())
	{
		DKCriticalSection<DKSpinLock> guard(this->spinLock);
		DKASSERT_DEBUG(numShared == 0);
		DKASSERT_DEBUG(sharedPtr == NULL);

		void* ptr = this->LockContent();
		if (p)
			*p = ptr;

		return true;
	}
	return false;
}

void DKData::UnlockExclusive(void)
{
	DKCriticalSection<DKSpinLock> guard(this->spinLock);
	DKASSERT_DEBUG(numShared == 0);
	DKASSERT_DEBUG(sharedPtr == NULL);

	sharedLock.Unlock();
	this->UnlockContent();
}
