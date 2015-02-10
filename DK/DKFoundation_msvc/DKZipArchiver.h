//
//  File: DKZipArchiver.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKString.h"
#include "DKBuffer.h"
#include "DKDateTime.h"
#include "DKArray.h"
#include "DKStream.h"
#include "DKLock.h"

////////////////////////////////////////////////////////////////////////////////
// DKZipArchiver
// a zip file writer.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKZipArchiver
	{
	public:
		DKZipArchiver(void);
		~DKZipArchiver(void);

		static DKObject<DKZipArchiver> Create(const DKString& file, bool append);

		// add file into zip-archive.
		// compressionLevel is integer value from 0 to 9. (default is 6)
		// (0: no-compression, 9: maximum compression)
		bool Write(const DKString& file, DKStream* stream, int compressionLevel, const char* password = NULL);
		bool Write(const DKString& file, const void* data, size_t len, int compressionLevel, const char* password = NULL);

		const DKString& GetArchiveName(void) const		{return filename;}
	private:
		DKString	filename;
		void*		zipHandle;
		DKLock		lock;
	};
}
