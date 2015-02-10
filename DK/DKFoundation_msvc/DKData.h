//
//  File: DKData.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKObject.h"
#include "DKString.h"
#include "DKSharedLock.h"
#include "DKSpinLock.h"

////////////////////////////////////////////////////////////////////////////////
// DKData
// abstract class.
//
// provide memory accessing interface.
// this class supports locking with shared, exclusive access.
//
// full thread-safe.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKOperation;
	class DKStream;
	class DKLIB_API DKData
	{
	public:
		DKData(void);
		virtual ~DKData(void);

		virtual size_t Length(void) const = 0;
		virtual bool IsReadable(void) const = 0;
		virtual bool IsWritable(void) const = 0;
		virtual bool IsExcutable(void) const = 0;		// program code image

		// StaticData: using DKData with existing buffer,
		// you can provide cleanup operation which will be invoked on finished.
		static DKObject<DKData> StaticData(const void* p, size_t len, bool readonly = true, DKOperation* cleanup = NULL);

		bool WriteToFile(const DKString& file, bool overwrite) const;
		bool WriteToStream(DKStream* stream) const;

		// shared lock. (read-only)
		// multiple-threads can be locked with this method simultaneously.
		const void* LockShared(void) const;
		bool TryLockShared(const void**) const;
		void UnlockShared(void) const;

		// exclusive lock. (read-write)
		// only one thread can be locked.
		void* LockExclusive(void);
		bool TryLockExclusive(void**);
		void UnlockExclusive(void);

	protected:
		virtual void* LockContent(void) = 0;
		virtual void UnlockContent(void) = 0;
		DKSharedLock sharedLock;
	private:
		DKSpinLock	spinLock;
		mutable size_t numShared;
		mutable const void* sharedPtr;
	};

	// scoped read, write accessor
	class DKDataReader
	{
	public:
		DKDataReader(DKData* p) : source(p), data(NULL)
		{
			if (source)
				data = source->LockShared();
		}
		DKDataReader(const DKDataReader& r) : source(r.source), data(NULL)
		{
			if (source)
				data = source->LockShared();
		}
		~DKDataReader(void)
		{
			if (source)
				source->UnlockShared();
		}
		DKDataReader& operator = (const DKDataReader& r)
		{
			if (this->source != r.source)
			{
				if (source)
					source->UnlockShared();
				data = NULL;
				this->source = r.source;
				if (source)
					data = source->LockShared();
			}
			return *this;
		}
		size_t Length(void) const
		{
			if (source)
				return source->Length();
			return 0;
		}
		operator const void* (void)	{return data;}
	private:
		DKObject<DKData> source;
		const void* data;
	};

	class DKDataWriter
	{
	public:
		DKDataWriter(DKData* p) : source(p), data(NULL)
		{
			if (source)
				data = source->LockExclusive();
		}
		~DKDataWriter(void)
		{
			if (source)
				source->UnlockExclusive();
		}
		size_t Length(void) const
		{
			if (source)
				return source->Length();
			return 0;
		}
		operator void* (void)		{return data;}
	private:
		DKObject<DKData> source;
		void* data;
		DKDataWriter(const DKDataWriter&);
		DKDataWriter& operator = (const DKDataWriter&);
	};
}
