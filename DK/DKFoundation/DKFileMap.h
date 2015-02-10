//
//  File: DKFileMap.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKObject.h"
#include "DKSpinLock.h"
#include "DKString.h"
#include "DKData.h"
#include "DKStream.h"

////////////////////////////////////////////////////////////////////////////////
// DKFileMap
// mapping file and memory to provide random-access of file contents.
// good for large file access randomly. cannot be resized already mapped.
//
// To map range (not entire contents) use DKFile::MapContentRange.
//
// Mapping modes:
//  - read only (entire size or desired length)
//  - read write (entire size or desired length)
//  - truncate file and overwrite (desired length)
//  - read write for anonymous file (desired length)
//      anonymous file will be deleted automatically.
//
// Note:
//  Do not map a file which located in slow-speed device.
//   (Network volumes or External usb drives, etc.)
//  You cannot share this object with other processes.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKFileMap : public DKData
	{
	public:
		DKFileMap(void);
		~DKFileMap(void);

		// open and map existing file. if size is 0, map entire contents.
		static DKObject<DKFileMap> Open(const DKString& file, size_t size, bool writable);
		// create new file, fails if overwrite is false and file is exists.
		static DKObject<DKFileMap> Create(const DKString& file, size_t size, bool overwrite);
		// map temporary file with desired length.
		static DKObject<DKFileMap> Temporary(size_t size);
		// using virtual memory (page memory)
		static DKObject<DKFileMap> Virtual(size_t size);

		bool IsReadable(void) const;
		bool IsWritable(void) const;
		bool IsExcutable(void) const;
		size_t Length(void) const;

	protected:
		void* LockContent(void);
		void UnlockContent(void);

	private:
		void* mapContext;

		DKFileMap(const DKFileMap&);
		DKFileMap& operator = (const DKFileMap&);
	};
}
