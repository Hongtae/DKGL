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

        virtual const void* Contents() const = 0;
        virtual void* MutableContents() { return nullptr; }

		/// Clone immutable data object.
		virtual DKObject<DKData> ImmutableData() const;

		DKData(DKData&&) = delete;
		DKData(const DKData&) = delete;
		DKData& operator = (DKData&&) = delete;
		DKData& operator = (const DKData&) = delete;
	};
}
