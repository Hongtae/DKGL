//
//  File: DKFile.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKStream.h"
#include "DKString.h"
#include "DKBuffer.h"
#include "DKDateTime.h"

namespace DKFoundation
{
	/// @brief a file stream object.
	/// - provide stream interface by default.
	/// - provide data(buffer) interface by file-map (MapContentRange)
	class DKGL_API DKFile : public DKStream
	{
	public:
		struct FileInfo
		{
			uint64_t		size;
			DKDateTime		lastAccessed;
			DKDateTime		lastModified;
			DKDateTime		created;
			bool 			readable;
			bool 			writable;
		};
		enum ModeOpen
		{
			ModeOpenNew = 0,		///< if file exist, truncate zero (create + truncate + read + write)
			ModeOpenCreate,			///< create new, error if file exist (create + read + write)
			ModeOpenExisting,		///< if file not exist error. (read + write)
			ModeOpenReadOnly,		///< open exist file read-only (read)
			ModeOpenAlways,			///< always open, even if not exist. (read + write)
		};
		enum ModeShare
		{
			ModeShareAll = 0,
			ModeShareRead,
			ModeShareExclusive,
		};

		DKFile();
		~DKFile();

		static bool GetInfo(const DKString& file, FileInfo& info);
		static bool Delete(const DKString& file);

		static DKObject<DKFile> Create(const DKString& file, ModeOpen mod, ModeShare share);
		/// create temporary file. delete automatically when object destroyed.
		static DKObject<DKFile> CreateTemporary();

		Position SetCurrentPosition(Position p) override;
		Position CurrentPosition() const override;
		Position RemainLength() const override;
		Position TotalLength() const override;

		/// read file contents and returns DKBuffer object.
		DKObject<DKBuffer> Read(size_t s, DKAllocator& alloc = DKAllocator::DefaultAllocator()) const;
		/// read file contents and copy into p.
		size_t Read(void* p, size_t s) const;
		size_t Read(void* p, size_t s) override;
		/// read file contents and write to other stream
		size_t Read(DKStream* p, size_t s) const;

		size_t Write(const void* p, size_t s) override;
		size_t Write(const DKData *p);
		size_t Write(DKStream* s);

		bool GetInfo(FileInfo& info) const; ///< get file info (for this object)
		FileInfo GetInfo() const;

		bool SetLength(size_t len);

		bool IsReadable() const override;
		bool IsWritable() const override;
		bool IsSeekable() const override;

		const DKString& Path() const;

		/// file-mapped data
		DKObject<DKData> MapContentRange(size_t offset, size_t length);
	
	private:
		DKString	path;
		intptr_t	file;
		ModeOpen	modeOpen;
		ModeShare	modeShare;

		DKFile(const DKFile&) = delete;
		DKFile& operator = (const DKFile&) = delete;
	};
}
