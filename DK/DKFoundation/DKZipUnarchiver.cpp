//
//  File: DKZipUnarchiver.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../lib/zlib.h"
#include "DKZipUnarchiver.h"
#include "DKString.h"
#include "DKLog.h"

namespace DKFoundation
{
	namespace Private
	{
		// openning zip file as new handle for extract one file.
		// we should use separate handles to extract files from one zip file simultaneously.
		// becouse of each handles calcualte and store hash (crc-32) when reading data,
		// multiple handle is better for multiple file extracting.

		class UnZipFile : public DKStream
		{
		public:
			static DKObject<UnZipFile> Create(const DKString& zipFile, const DKString& file, const char* password)
			{
				if (zipFile.Length() == 0 || file.Length() == 0)
					return NULL;

				DKString filename = zipFile.FilePathString();

				unzFile uf = NULL;
#ifdef _WIN32
				{
					zlib_filefunc64_def ffunc;
					fill_win32_filefunc64W(&ffunc);
					uf = unzOpen2_64((const wchar_t*)filename, &ffunc); // UTF16LE
				}
#else
				{
					DKStringU8 filenameUTF8(filename);
					if (filenameUTF8.Bytes() > 0)
						uf = unzOpen64((const char*)filenameUTF8); // UTF8
				}
#endif
				if (uf)
				{
					unz_file_info64 file_info;
					DKStringU8 fileUTF8(file);
					if (fileUTF8.Bytes() > 0 &&
						unzLocateFile(uf, (const char*)fileUTF8, 0) == UNZ_OK &&
						unzGetCurrentFileInfo64(uf, &file_info, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK &&
						file_info.uncompressed_size > 0)
					{
						if (unzOpenCurrentFilePassword(uf, password) == UNZ_OK)
						{
							DKObject<UnZipFile> p = DKOBJECT_NEW UnZipFile(uf, file_info, password);
							return p;
						}
						else
						{
							DKLog("[%s] failed to open file: %ls.\n", DKGL_FUNCTION_NAME, (const wchar_t*)file);
						}
					}
				}
				return NULL;
			}
			~UnZipFile(void)
			{
				if (unzCloseCurrentFile(handle) != UNZ_OK)
					DKERROR_THROW_DEBUG("unzCloseCurrentFile failed!");
				if (unzClose(handle) != UNZ_OK)
					DKERROR_THROW_DEBUG("unzClose failed!");
			}
		protected:
			UnZipFile(unzFile f, unz_file_info64 info, const char* passwd)
				: handle(f)
				, fileInfo(info)
			{
				DKASSERT_DEBUG(handle != NULL);
				
				if (passwd)
				{
					for (int i = 0; passwd[i] ; i++)
						password.Add(passwd[i]);
					password.Add(0);
				}				
			}
			Position SetPos(Position p)
			{
				Position currentPos = GetPos();
				if (p == currentPos)
					return p;
				if (p <= fileInfo.uncompressed_size)
				{
					Position readBytes = 0;
					if (currentPos > p) // reading from beginning to offset p.
					{
						// to move new offset, re-open file and reading until new offset.
						if (unzOpenCurrentFilePassword(handle, (const char*)password) != UNZ_OK)
							return -1;
						readBytes = p;
					}
					else // need to read bytes of 'p - currentPos'.
					{
						readBytes = p - currentPos;
					}

					if (readBytes > 0)
					{
						void* tmp = DKMemoryDefaultAllocator::Alloc(readBytes);
						unzReadCurrentFile(handle, tmp, readBytes);
						DKMemoryDefaultAllocator::Free(tmp);
					}
					return GetPos();
				}
				return -1;
			}
			Position GetPos(void) const
			{
				return unztell64(handle);
			}
			Position RemainLength(void) const
			{
				return fileInfo.uncompressed_size - GetPos();
			}
			Position TotalLength(void) const
			{
				return fileInfo.uncompressed_size;
			}
			size_t Read(void* p, size_t s)
			{
				if (s == 0)
					return 0;
				if (p == NULL)
					return 0;

				const size_t platformMaxSize = 0x7fffffff;
				char* cp = reinterpret_cast<char*>(p);

				size_t totalRead = 0;
				while (s > 0)
				{
					int toRead = Min(s, platformMaxSize);
					int numRead = unzReadCurrentFile(handle, cp + totalRead, toRead);
					if (numRead <= 0)
						break;
					s -= numRead;
					totalRead += numRead;
				}
				return totalRead;
			}
			size_t Write(const void* p, size_t s)
			{
				return 0;
			}

			bool IsReadable(void) const	{return true;}
			bool IsWritable(void) const	{return false;}
			bool IsSeekable(void) const	{return true;}
		private:
			unzFile					handle;
			const unz_file_info64	fileInfo;
			DKArray<char>			password;
		};
	}
}

using namespace DKFoundation;

DKZipUnarchiver::DKZipUnarchiver(void)
: zipHandle(NULL)
{
}

DKZipUnarchiver::~DKZipUnarchiver(void)
{
	if (zipHandle)
	{
		if (unzClose(zipHandle) != UNZ_OK)
			DKERROR_THROW_DEBUG("unzClose failed!");
	}
}

DKObject<DKZipUnarchiver> DKZipUnarchiver::Create(const DKString& file)
{
	if (file.Length() == 0)
		return NULL;

	DKString filename = file.FilePathString();

	unzFile uf = NULL;
#ifdef _WIN32
	{
		zlib_filefunc64_def ffunc;
		fill_win32_filefunc64W(&ffunc);
		uf = unzOpen2_64((const wchar_t*)filename, &ffunc); // UTF16LE
	}
#else
	{
		DKStringU8 filenameUTF8(filename);
		if (filenameUTF8.Bytes() > 0)
			uf = unzOpen64((const char*)filenameUTF8); // UTF8
	}
#endif
	if (uf)
	{
		DKArray<FileInfo>	filesArray;
		unz_global_info64 gi;
		int err = unzGetGlobalInfo64(uf,&gi);
		if (err == UNZ_OK)
		{
			filesArray.Reserve(gi.number_entry);
			for (int i = 0; i < gi.number_entry; i++)
			{
				DKUniChar8 filename_inzip[1024];
				unz_file_info64 file_info;
				err = unzGetCurrentFileInfo64(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
				if (err == UNZ_OK)
				{
					FileInfo	file;
					size_t len = strlen(filename_inzip);
					if (len > 0 && filename_inzip[len-1] == '/')
					{
						file.directory = true;
						filename_inzip[len-1] = 0; // ignore last path separator
					}
					else
						file.directory = false;
					file.name = DKString(filename_inzip);
					if (len > 0 && file.name.Length() > 0)
					{
						file.uncompressedSize = file_info.uncompressed_size;
						file.compressedSize = file_info.compressed_size;
						file.crypted = file_info.flag & 1;
						file.compressLevel = 0;
						switch (file_info.compression_method)
						{
						case 0:
							file.method = MethodStored;
							break;
						case Z_DEFLATED:
							file.method = MethodDeflated;
							file.compressLevel = (file_info.flag & 0x6) / 2;
							break;
						case Z_BZIP2ED:
							file.method = MethodBZip2ed;
							break;
						default:
							file.method = MethodUnknown;
							break;
						}
						file.crc32 = file_info.crc;
						file.date = DKDateTime(file_info.tmu_date.tm_year, file_info.tmu_date.tm_mon, file_info.tmu_date.tm_mday, file_info.tmu_date.tm_hour, file_info.tmu_date.tm_min, file_info.tmu_date.tm_sec, 0);
						filesArray.Add(file);
					}
				}
				else
				{
					DKLog("zip[%d] error.\n", i);
				}

				err = unzGoToNextFile(uf);
				if (err == UNZ_END_OF_LIST_OF_FILE)
					break;
				if (err != UNZ_OK)
				{
					DKLog("error %d with zipfile in unzGoToNextFile\n",err);
					return NULL;
				}
			}

			DKObject<DKZipUnarchiver> unarchiver = DKObject<DKZipUnarchiver>::New();
			unarchiver->zipHandle = uf;
			unarchiver->filename = filename;
			unarchiver->files = filesArray;

			return unarchiver;
		}
		else
		{
			DKLog("[%s] error %d with file: %ls.\n", DKGL_FUNCTION_NAME, err, (const wchar_t*)file);
		}
	}
	else
	{
		DKLog("[%s] Cannot open file: %ls.\n", DKGL_FUNCTION_NAME, (const wchar_t*)file);
	}

	return NULL;
}

const DKZipUnarchiver::FileInfo* DKZipUnarchiver::GetFileInfo(const DKString& file) const
{
	for (int i = 0; i < files.Count(); i++)
	{
		if (file.CompareNoCase(files.Value(i).name) == 0)
			return &(files.Value(i));
	}
	return NULL;
}

DKObject<DKStream> DKZipUnarchiver::OpenFileStream(const DKString& file, const char* password) const
{
	// check file is exists, open if file exists.
	DKStringU8 fileUTF8(file);

	unz_file_info64 file_info;

	if (fileUTF8.Bytes() > 0 &&
		unzLocateFile(zipHandle, (const char*)fileUTF8, 0) == UNZ_OK &&
		unzGetCurrentFileInfo64(zipHandle, &file_info, NULL, 0, NULL, 0, NULL, 0) == UNZ_OK &&
		file_info.uncompressed_size > 0)
	{
		return Private::UnZipFile::Create(filename, file, password).SafeCast<DKStream>();
	}
	return NULL;
}
