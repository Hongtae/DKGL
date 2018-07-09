//
//  File: DKFileMap.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKSpinLock.h"
#include "DKString.h"
#include "DKData.h"
#include "DKStream.h"

namespace DKFoundation
{
	/// @brief
	/// mapping file and memory to provide random-access of file contents.
	/// good for large file access randomly. cannot be resized already mapped.
	///
	/// To map range (not entire contents) use DKFile::MapContentRange.
	///
	/// Mapping modes:
	///  - read only (entire size or desired length)
	///  - read write (entire size or desired length)
	///  - truncate file and overwrite (desired length)
	///  - read write for anonymous file (desired length)
	///      anonymous file will be deleted automatically.
	///
	/// @note
	///  Do not map a file which located in slow-speed device.
	///   (Network volumes or External usb drives, etc.)
	///  You cannot share this object with other processes.
	class DKGL_API DKFileMap : public DKData
	{
	public:
		DKFileMap(void);
		~DKFileMap(void);

		/// open and map existing file. if size is 0, map entire contents.
		static DKObject<DKFileMap> Open(const DKString& file, size_t size, bool writable);
		/// create new file, fails if overwrite is false and file is exists.
		static DKObject<DKFileMap> Create(const DKString& file, size_t size, bool overwrite);
		/// map temporary file with desired length.
		static DKObject<DKFileMap> Temporary(size_t size);
		/// using virtual memory (page memory)
		static DKObject<DKFileMap> Virtual(size_t size);

		bool IsReadable(void) const override;
		bool IsWritable(void) const override;
		bool IsExcutable(void) const override;
		bool IsTransient(void) const override;
		size_t Length(void) const override;

		const void* LockShared(void) const override;
		bool TryLockShared(const void** ptr) const override;
		void UnlockShared(void) const override;

		void* LockExclusive(void) override;
		bool TryLockExclusive(void** ptr) override;
		void UnlockExclusive(void) override;

	private:
		void* MapContent(void) const;
		void UnmapContent(void) const;
		mutable void* mappedPtr;

		void* mapContext;
		DKSharedLock lock;
		DKSpinLock spinLock;

		DKFileMap(const DKFileMap&) = delete;
		DKFileMap& operator = (const DKFileMap&) = delete;
	};
}
