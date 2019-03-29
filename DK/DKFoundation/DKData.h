//
//  File: DKData.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKString.h"
#include "DKSharedLock.h"
#include "DKSpinLock.h"

namespace DKFoundation
{
	class DKOperation;
	class DKStream;

	/// @brief Interface for describing raw-data
	///
	/// Offers a memory access interface with shared or exclusive locks
	/// @see DKBuffer
	class DKGL_API DKData
	{
	public:
		DKData();
		virtual ~DKData();

		virtual size_t Length() const = 0;
		virtual bool IsReadable() const = 0;
		virtual bool IsWritable() const = 0;
		virtual bool IsExcutable() const = 0;		///< program code image
		virtual bool IsTransient() const = 0;

		/// Cast an existing buffer to DKData (writable).
		/// You can provide cleanup operation which will be invoked on finished.
		/// @param p existing buffer address. It should be writable.
		/// @param len length of buffer
		/// @param cleanup Custom cleanup task called when 'DKData' is released
		static DKObject<DKData> StaticData(void* p, size_t len, bool readonly, DKOperation* cleanup = NULL);
		/// Cast an existing buffer to DKData (readonly).
		/// You can provide cleanup operation which will be invoked on finished.
		/// @param p existing buffer address
		/// @param len length of buffer
		/// @param cleanup Custom cleanup task called when 'DKData' is released
		static DKObject<DKData> StaticData(const void* p, size_t len, DKOperation* cleanup = NULL);

		/// Write content to file
		bool WriteToFile(const DKString& file, bool overwrite) const;
		/// Write content to a stream object
		bool WriteToStream(DKStream* stream) const;

		/// shared lock. (read-only)
		/// multiple-threads can be locked with this method simultaneously.
		virtual const void* LockShared() const = 0;
		virtual bool TryLockShared(const void**) const = 0;
		virtual void UnlockShared() const = 0;

		/// exclusive lock. (read-write)
		/// only one thread can be locked.
		virtual void* LockExclusive()		{ return NULL; }
		virtual bool TryLockExclusive(void**)	{ return false; }
		virtual void UnlockExclusive()		{}

		/// Clone immutable data object.
		virtual DKObject<DKData> ImmutableData() const;


		DKData(DKData&&) = delete;
		DKData(const DKData&) = delete;
		DKData& operator = (DKData&&) = delete;
		DKData& operator = (const DKData&) = delete;
	};

	/// @brief scoped read accessor for DKData
	///
	/// Data objects must implement shared locks.
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
		~DKDataReader()
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
		size_t Length() const
		{
			if (source)
				return source->Length();
			return 0;
		}
		const void* Bytes() const		{return data;}
		operator const void* () const	{return data;}
	private:
		DKObject<DKData> source;
		const void* data;
	};
	/// @brief scoped write accessor for DKData
	/// 
	/// Data objects must implement exclusive locks.
	class DKDataWriter
	{
	public:
		DKDataWriter(DKData* p) : source(p), data(NULL)
		{
			if (source)
				data = source->LockExclusive();
		}
		~DKDataWriter()
		{
			if (source)
				source->UnlockExclusive();
		}
		size_t Length() const
		{
			if (source)
				return source->Length();
			return 0;
		}
		void* Bytes() const		{return data;}
		operator void* ()		{return data;}
	private:
		DKObject<DKData> source;
		void* data;
		DKDataWriter(const DKDataWriter&) = delete;
		DKDataWriter& operator = (const DKDataWriter&) = delete;
	};
}
