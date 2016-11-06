//
//  File: DKZipArchiver.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "../Libs/Inc_zlib.h"
#include "DKZipArchiver.h"
#include "DKCriticalSection.h"
#include "DKString.h"
#include "DKLog.h"

using namespace DKGL;

DKZipArchiver::DKZipArchiver(void)
: zipHandle(NULL)
{
}

DKZipArchiver::~DKZipArchiver(void)
{
	if (zipHandle)
	{
		if (zipClose(zipHandle, NULL) != ZIP_OK)
			DKERROR_THROW_DEBUG("zipClose failed!");
	}
}

DKObject<DKZipArchiver> DKZipArchiver::Create(const DKString& file, bool append)
{
	if (file.Length() == 0)
		return NULL;

	DKString filename = file.FilePathString();

	zipFile zf = NULL;
	int openMode = append ? 2 : 0;
#ifdef _WIN32
	{
		zlib_filefunc64_def ffunc;
		fill_win32_filefunc64W(&ffunc);
		zf = zipOpen2_64((const wchar_t*)filename, openMode, NULL, &ffunc);			// UTF16LE
	}
#else
	{
		DKStringU8 filenameUTF8(filename);
		if (filenameUTF8.Bytes() > 0)
			zf = zipOpen64((const char*)filenameUTF8, openMode);		// UTF8
	}
#endif
	if (zf)
	{
		DKObject<DKZipArchiver> archiver = DKObject<DKZipArchiver>::New();
		archiver->zipHandle = zf;
		archiver->filename = file;
		return archiver;
	}
	return NULL;
}

bool DKZipArchiver::Write(const DKString& file, DKStream* stream, int compressionLevel, const char* password)
{
	if (zipHandle && stream && stream->IsReadable())
	{
		if (password && stream->IsSeekable() == false)
		{
			DKLog("DKZipArchiver Error: Cannot generate password CRC (stream is not seekable!)\n");
			return false;
		}

		DKStringU8 filenameUTF8(file);
		if (filenameUTF8.Bytes() == 0)
			return false;

		DKStream::Position streamOffset = stream->GetPos();
		uint64_t streamLength = stream->RemainLength();

		char* buffer[0x4000];
		const size_t bufferSize = 0x4000;		// 16384

		DKCriticalSection<DKLock>	section(lock);

		uLong crcForCrypting = 0;
		if (password)
		{
			stream->SetPos(streamOffset);

			streamLength = 0;

			while ( true )
			{
				int read = stream->Read(buffer, bufferSize);
				if (read <= 0)
					break;
				crcForCrypting = crc32(crcForCrypting, (const Bytef *)buffer, read);
				streamLength += read;
			}
			stream->SetPos(streamOffset);
		}

		int zip64 = (streamLength >= 0xffffffff) ? 1 : 0;
		compressionLevel = Clamp(compressionLevel, 0, 9);

		zip_fileinfo	zinfo;
		memset(&zinfo, 0, sizeof(zip_fileinfo));

		if (zipOpenNewFileInZip3_64(zipHandle, (const char*)filenameUTF8, &zinfo, 
			NULL,0,NULL,0,NULL,
			(compressionLevel != 0) ? Z_DEFLATED : 0, compressionLevel, 0,
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			password, crcForCrypting, zip64) == ZIP_OK)
		{
			while ( streamLength > 0 )
			{
				int toWrite = stream->Read(buffer, bufferSize);
				if (zipWriteInFileInZip(zipHandle, buffer, toWrite) < 0)
				{
					DKLog("[%s] zipWriteInFileInZip error!\n", DKGL_FUNCTION_NAME); 
					return false;
				}
				streamLength -= toWrite;
			}

			if (zipCloseFileInZip(zipHandle) != ZIP_OK)
			{
				DKLog("[%s] zipCloseFileInZip error!\n", DKGL_FUNCTION_NAME); 
				return false;
			}
			return true;
		}

	}
	return false;
}

bool DKZipArchiver::Write(const DKString& file, const void* data, size_t len, int compressionLevel, const char* password)
{
	if (zipHandle)
	{
		DKStringU8 filenameUTF8(file);
		if (filenameUTF8.Bytes() == 0)
			return false;
	
		uLong crcForCrypting = 0;
		if (password)
			crcForCrypting = crc32(crcForCrypting, (const Bytef *)data, len);

		int zip64 = (len >= 0xffffffff) ? 1 : 0;
		compressionLevel = Clamp(compressionLevel, 0, 9);

		zip_fileinfo	zinfo;
		memset(&zinfo, 0, sizeof(zip_fileinfo));

		DKCriticalSection<DKLock>	section(lock);

		if (zipOpenNewFileInZip3_64(zipHandle, (const char*)filenameUTF8, &zinfo, 
			NULL,0,NULL,0,NULL,
			(compressionLevel != 0) ? Z_DEFLATED : 0, compressionLevel, 0,
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
			password, crcForCrypting, zip64) == ZIP_OK)
		{
			size_t totalWritten = 0;
			const char* cdata = reinterpret_cast<const char*>(data);
			while (len > 0)
			{
				int toWrite = Min(len, 0x4000);
				if (zipWriteInFileInZip(zipHandle, cdata + totalWritten, toWrite) < 0)
				{
					DKLog("[%s] zipWriteInFileInZip error!\n", DKGL_FUNCTION_NAME); 
					return false;
				}
				totalWritten += toWrite;
				len -= toWrite;
			}

			if (zipCloseFileInZip(zipHandle) != ZIP_OK)
			{
				DKLog("[%s] zipCloseFileInZip error!\n", DKGL_FUNCTION_NAME); 
				return false;
			}
			return true;
		}
	}
	return false;
}
