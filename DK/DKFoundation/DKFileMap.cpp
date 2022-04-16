//
//  File: DKFileMap.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#endif

#include "DKFile.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKFileMap.h"
#include "DKUtils.h"
#include "DKUuid.h"

namespace DKFoundation::Private
{
#ifdef _WIN32
    DKString GetWin32ErrorString(DWORD dwError);
#endif
    struct FileMapContext
    {
#ifdef _WIN32
        HANDLE file;
        HANDLE map;
        DWORD access;
#else
        int file;
        int prot;
        int flags;
#endif
        void* baseAddress;
        size_t length;
    };
}

using namespace DKFoundation;
using namespace DKFoundation::Private;

DKFileMap::DKFileMap()
	: mapContext(nullptr)
{
}

DKFileMap::~DKFileMap()
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
#ifdef _WIN32
		if (ctxt->baseAddress)
		{
			if (::UnmapViewOfFile(ctxt->baseAddress) == FALSE)
			{
				DKLog("UnmapViewOfFile failed:%ls\n", (const wchar_t*)GetWin32ErrorString(::GetLastError()));
			}
		}
		DKASSERT_DEBUG(ctxt->map != nullptr);
		if (::CloseHandle(ctxt->map) == FALSE)
		{
			DKLog("CloseHandle failed:%ls\n", (const wchar_t*)GetWin32ErrorString(::GetLastError()));
		}
		if (ctxt->file != INVALID_HANDLE_VALUE)
		{
			if (::CloseHandle(ctxt->file) == FALSE)
			{
				DKLog("CloseHandle failed:%ls\n", (const wchar_t*)GetWin32ErrorString(::GetLastError()));
			}
		}
#else
		if (ctxt->baseAddress)
		{
			if (::munmap(ctxt->baseAddress, ctxt->length) != 0)
			{
				DKLog("munmap failed:%s\n", strerror(errno));
			}
		}
		if (ctxt->file != -1)
		{
			if (::close(ctxt->file) != 0)
			{
				DKLog("close failed:%s\n", strerror(errno));
			}
		}
#endif
		delete ctxt;
	}
}

const void* DKFileMap::Contents() const
{
    return MapContents();
}

void* DKFileMap::MutableContents()
{
    if (IsWritable())
        return MapContents();
    return nullptr;
}

void* DKFileMap::MapContents() const
{
	// map, commit
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
        DKCriticalSection<DKSpinLock> guard(spinLock);
        if (ctxt->baseAddress == nullptr)
        {
            DKASSERT_DEBUG(ctxt->length != 0);

#ifdef _WIN32
            DKASSERT_DEBUG(ctxt->map != nullptr);
            ctxt->baseAddress = ::MapViewOfFile(ctxt->map, ctxt->access, 0, 0, ctxt->length);
            if (ctxt->baseAddress == nullptr)
            {
                DKLog("MapViewOfFile failed:%ls\n", (const wchar_t*)GetWin32ErrorString(::GetLastError()));
            }
#else
            ctxt->baseAddress = ::mmap(0, ctxt->length, ctxt->prot, ctxt->flags, ctxt->file, 0);
            if (ctxt->baseAddress == MAP_FAILED)
            {
                ctxt->baseAddress = nullptr;
                DKLog("mmap failed:%s\n", strerror(errno));
            }
#endif
        }
		return ctxt->baseAddress;
	}
	return nullptr;
}

void DKFileMap::UnmapContents() const
{
	// unmap, decommit
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	DKASSERT_DEBUG(ctxt != nullptr);
	if (ctxt)
	{
        DKCriticalSection<DKSpinLock> guard(spinLock);
        if (ctxt->baseAddress != nullptr)
        {
#ifdef _WIN32
            if (::UnmapViewOfFile(ctxt->baseAddress) == FALSE)
            {
                DKLog("UnmapViewOfFile failed:%ls\n", (const wchar_t*)GetWin32ErrorString(::GetLastError()));
                DKERROR_THROW_DEBUG("UnmapViewOfFile failed");
            }
#else
            if (::munmap(ctxt->baseAddress, ctxt->length) != 0)
            {
                DKLog("munmap failed:%s\n", strerror(errno));
                DKERROR_THROW_DEBUG("munmap failed");
            }
#endif
            ctxt->baseAddress = nullptr;
        }
	}
}

size_t DKFileMap::Length() const
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
		return ctxt->length;
	}
	return 0;
}

bool DKFileMap::IsReadable() const
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
#ifdef _WIN32
		return true;
#else
		return (ctxt->prot & PROT_READ) != 0;
#endif
	}
	return false;
}

bool DKFileMap::IsWritable() const
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
#ifdef _WIN32
		return (ctxt->access & FILE_MAP_WRITE) != 0;
#else
		return (ctxt->prot & PROT_WRITE) != 0;
#endif
	}
	return false;
}

bool DKFileMap::IsExcutable() const
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
#ifdef _WIN32
		return (ctxt->access & FILE_MAP_EXECUTE) != 0;
#else
		return (ctxt->prot & PROT_EXEC) != 0;
#endif
	}
	return false;
}

bool DKFileMap::IsTransient() const
{
	FileMapContext* ctxt = reinterpret_cast<FileMapContext*>(this->mapContext);
	if (ctxt)
	{
		return false;
	}
	return true;
}

DKObject<DKFileMap> DKFileMap::Open(const DKString &file, size_t size, bool writable)
{
	if (file.Length() == 0)
		return nullptr;

#ifdef _WIN32
	DKString path(file.FilePathString());
	DWORD dwAccess = writable ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	DWORD dwShare = writable ? 0 : FILE_SHARE_READ;
	HANDLE hFile = ::CreateFileW((const wchar_t*)path, dwAccess, dwShare, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER fs;
		if (::GetFileSizeEx(hFile, &fs))
		{
			if (size == 0)
				size = fs.QuadPart;

			if (size > 0)
			{
				LARGE_INTEGER mapLength;
				mapLength.QuadPart = size;
				DWORD flProtect = writable ? PAGE_READWRITE : PAGE_READONLY;
				HANDLE hMap = ::CreateFileMappingW(hFile, nullptr, flProtect, mapLength.HighPart, mapLength.LowPart, nullptr);
				if (hMap)
				{
					FileMapContext* ctxt = new FileMapContext();
					ctxt->file = hFile;
					ctxt->map = hMap;
					ctxt->access = writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
					ctxt->baseAddress = nullptr;
					ctxt->length = size;

					DKLog("DKFileMap created. (file:%ls, length:%lu, file-size:%lld, %s)\n", (const wchar_t*)path, size, fs.QuadPart, (writable ? "writable" : "readonly"));
					DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
					fileMap->mapContext = reinterpret_cast<void*>(ctxt);
					return fileMap;
				}
				else
				{
					DKLog("CreateFileMapping(%ls, %lu) failed: %s\n", (const wchar_t*)path, size, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
				}
			}
			else
			{
				DKLog("file size and requested size is zero.\n");
			}
		}
		else
		{
			DKLog("GetFileSize(%ls) failed:%ls\n", (const wchar_t*)path, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
		}
		if (::CloseHandle(hFile) == 0)
			DKLog("CloseHandle(%p) failed: %ls\n", hFile, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
	}
	else
	{
		DKLog("CreateFile(%ls) failed:%ls\n", (const wchar_t*)path, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
	}
#else
	DKStringU8 path(file.FilePathString());

#if defined(__APPLE__) && defined(__MACH__)
	int flags = writable ? (O_RDWR | O_EXLOCK) : (O_RDONLY | O_SHLOCK);
#else
	int flags = writable ? O_RDWR : O_RDONLY;
#endif
	int fd = ::open((const char*)path, flags, 0);
	if (fd != -1)
	{
#if defined(__APPLE__) && defined(__MACH__)
		int mmapFlags = MAP_FILE | MAP_SHARED;
#else
		if (fcntl(fd, F_SETLK, writable ? F_WRLCK : F_RDLCK) == -1)
			DKLog("fcntl failed: %s\n", strerror(errno));
		int mmapFlags = MAP_FILE | MAP_SHARED;
#endif

		struct stat st;
		if (::fstat( fd, &st ) == 0 )
		{
			if (size == 0)
				size = st.st_size;

			if (size > 0)
			{
				if (st.st_size >= size)
				{
					FileMapContext* ctxt = new FileMapContext();
					ctxt->file = fd;
					ctxt->prot = writable ? (PROT_READ|PROT_WRITE) : PROT_READ;
					ctxt->flags = mmapFlags;
					ctxt->baseAddress = nullptr;
					ctxt->length = size;

					DKLog("DKFileMap created. (file:%s, length:%lu, file-size:%lld, %s)\n", (const char*)path, size, st.st_size, (writable ? "writable" : "readonly"));
					DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
					fileMap->mapContext = reinterpret_cast<void*>(ctxt);
					return fileMap;
				}
				else if (writable)
				{
					if (::ftruncate(fd, size) == 0)
					{
						FileMapContext* ctxt = new FileMapContext();
						ctxt->file = fd;
						ctxt->prot = PROT_READ|PROT_WRITE;
						ctxt->flags = mmapFlags;
						ctxt->baseAddress = nullptr;
						ctxt->length = size;

						DKLog("DKFileMap created. (file:%s, length:%lu, file-size:%lld (before expand), writable)\n", (const char*)path, size, st.st_size);
						DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
						fileMap->mapContext = reinterpret_cast<void*>(ctxt);
						return fileMap;
					}
					else
					{
						DKLog("ftruncate(%s, %lu) failed: %s\n", (const char*)path, size, strerror(errno));
					}
				}
				else
				{
					DKLog("file map size invalid: (file:%ld, request:%ld)\n", st.st_size, size);
				}
			}
			else
			{
				DKLog("file size and requested size is zero.\n");
			}
		}
		else
		{
			DKLog("fstat(%s) failed: %s\n", (const char*)path, strerror(errno));
		}
		if (::close(fd) != 0)
			DKLog("close(%d) failed: %s\n", fd, strerror(errno));
	}
	else
	{
		DKLog("open(%s) failed: %s\n", (const char*)path, strerror(errno));
	}
#endif
	return nullptr;
}

DKObject<DKFileMap> DKFileMap::Create(const DKString& file, size_t size, bool overwrite)
{
	if (size == 0)
		return nullptr;

#ifdef _WIN32
	DKString path(file.FilePathString());
	DWORD dwCreationDisposition = overwrite ? CREATE_ALWAYS : CREATE_NEW;
	HANDLE hFile = ::CreateFileW((const wchar_t*)path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER mapLength;
		mapLength.QuadPart = size;
		HANDLE hMap = ::CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, mapLength.HighPart, mapLength.LowPart, nullptr);
		if (hMap)
		{
			FileMapContext* ctxt = new FileMapContext();
			ctxt->file = hFile;
			ctxt->map = hMap;
			ctxt->access = FILE_MAP_ALL_ACCESS;
			ctxt->baseAddress = nullptr;
			ctxt->length = size;

			DKLog("DKFileMap created. (file:%ls, length:%lu (file created), writable)\n", (const wchar_t*)path, size);
			DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
			fileMap->mapContext = reinterpret_cast<void*>(ctxt);
			return fileMap;
		}
		else
		{
			DKLog("CreateFileMapping(%ls, %lu) failed: %s\n", (const wchar_t*)path, size, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
		}
		if (::CloseHandle(hFile) == 0)
			DKLog("CloseHandle(%p) failed: %ls\n", hFile, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
	}
	else
	{
		DKLog("CreateFile(%ls) failed:%ls\n", (const wchar_t*)path, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
	}
#else
	DKStringU8 path(file.FilePathString());
#if defined(__APPLE__) && defined(__MACH__)
	int flags = O_RDWR | O_CREAT | O_EXLOCK;
#else
	int flags = O_RDWR | O_CREAT;
#endif
	if (!overwrite)
		flags |= O_EXCL;

	int fd = ::open((const char*)path, flags, 0);
	if (fd != -1)
	{
#if defined(__APPLE__) && defined(__MACH__)
		int mmapFlags = MAP_FILE | MAP_SHARED;
#else
		flock(fd, LOCK_EX);
		int mmapFlags = MAP_FILE | MAP_SHARED;
#endif

		if (::ftruncate(fd, size) == 0)
		{
			FileMapContext* ctxt = new FileMapContext();
			ctxt->file = fd;
			ctxt->prot = PROT_READ|PROT_WRITE;
			ctxt->flags = mmapFlags;
			ctxt->baseAddress = nullptr;
			ctxt->length = size;

			DKLog("DKFileMap created. (file:%s, length:%lu (file created), writable)\n", (const char*)path, size);
			DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
			fileMap->mapContext = reinterpret_cast<void*>(ctxt);
			return fileMap;
		}
		else
		{
			DKLog("ftruncate(%s, %lu) failed: %s\n", (const char*)path, size, strerror(errno));
		}
		if (::close(fd) != 0)
			DKLog("close(%d) failed: %s\n", fd, strerror(errno));
	}
	else
	{
		DKLog("open(%s) failed: %s\n", (const char*)path, strerror(errno));
	}
#endif
	return nullptr;
}

DKObject<DKFileMap> DKFileMap::Temporary(size_t size)
{
	if (size == 0)
		return nullptr;

	DKString tmpPath = DKTemporaryDirectory();
#ifdef _WIN32
	HANDLE hFile = INVALID_HANDLE_VALUE;
	while (hFile == INVALID_HANDLE_VALUE)
	{
		DKString filePath(tmpPath.FilePathStringByAppendingPath(DKUuid::Create().String()));
		if (filePath.Length() == 0)
		{
			DKLog("Cannot locate temp file!\n");
			break;
		}

		hFile = ::CreateFileW((const wchar_t*)filePath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW,
			FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD err = ::GetLastError();
			if (err != ERROR_FILE_EXISTS) // stop if not in case of file exists.
			{
				DKLog("CreateFile(%ls) failed:%s\n", (const wchar_t*)filePath, (const wchar_t*)GetWin32ErrorString(err));
				break;
			}
		}
		else
		{
			LARGE_INTEGER mapLength;
			mapLength.QuadPart = size;
			HANDLE hMap = ::CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, mapLength.HighPart, mapLength.LowPart, nullptr);
			if (hMap)
			{
				FileMapContext* ctxt = new FileMapContext();
				ctxt->file = hFile;
				ctxt->map = hMap;
				ctxt->access = FILE_MAP_ALL_ACCESS;
				ctxt->baseAddress = nullptr;
				ctxt->length = size;

				DKLog("DKFileMap created. (file:%ls, length:%lu (file created), writable)\n", (const wchar_t*)filePath, size);
				DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
				fileMap->mapContext = reinterpret_cast<void*>(ctxt);
				return fileMap;
			}
			else
			{
				DKLog("CreateFileMapping(%ls, %lu) failed: %s\n", (const wchar_t*)filePath, size, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
			}
			if (::CloseHandle(hFile) == 0)
				DKLog("CloseHandle(%p) failed: %ls\n", hFile, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
			break;
		}
	}
#else
	int fd = -1;
	while (fd == -1)
	{
		DKStringU8 filePath(tmpPath.FilePathStringByAppendingPath(DKUuid::Create().String()));
		if (filePath.Length() == 0)
		{
			DKLog("Cannot locate temp file!\n");
			break;
		}

#if defined(__APPLE__) && defined(__MACH__)
		fd = ::open((const char*)filePath, O_RDWR | O_CREAT | O_EXCL | O_EXLOCK);
		int mmapFlags = MAP_FILE | MAP_SHARED;
#else
		fd = ::open((const char*)filePath, O_RDWR | O_CREAT | O_EXCL);
		if (fd != -1)
			flock(fd, LOCK_EX);
		int mmapFlags = MAP_FILE | MAP_SHARED;
#endif
		if (fd == -1)
		{
			int err = errno;
			DKLog("file open error: %s\n", ::strerror(err));

			if (err != EEXIST) // stop if not in case of file exists.
			{
				break;
			}
		}
		else
		{
			if (::unlink((const char*)filePath) == 0)
			{
				if (::ftruncate(fd, size) == 0)
				{
					FileMapContext* ctxt = new FileMapContext();
					ctxt->file = fd;
					ctxt->prot = PROT_READ|PROT_WRITE;
					ctxt->flags = mmapFlags;
					ctxt->baseAddress = nullptr;
					ctxt->length = size;
					
					DKLog("DKFileMap created. (file:%s, length:%lu (file created), writable)\n", (const char*)filePath, size);
					DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
					fileMap->mapContext = reinterpret_cast<void*>(ctxt);
					return fileMap;
				}
				else
				{
					DKLog("ftruncate(%s, %lu) failed: %s\n", (const char*)filePath, size, strerror(errno));
				}
			}
			else
			{
				DKLog("unlink(%s) failed: %s\n", (const char*)filePath, strerror(errno));
			}

			if (::close(fd) != 0)
				DKLog("close(%d) failed: %s\n", fd, strerror(errno));			
			
			break;
		}
	}
#endif
	return nullptr;
}

DKObject<DKFileMap> DKFileMap::Virtual(size_t size)
{
	if (size > 0)
	{
#ifdef _WIN32
		LARGE_INTEGER mapLength;
		mapLength.QuadPart = size;
		HANDLE hMap = ::CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, mapLength.HighPart, mapLength.LowPart, nullptr);
		if (hMap)
		{
			FileMapContext* ctxt = new FileMapContext();
			ctxt->file = INVALID_HANDLE_VALUE;
			ctxt->map = hMap;
			ctxt->access = FILE_MAP_ALL_ACCESS;
			ctxt->baseAddress = nullptr;
			ctxt->length = size;

			DKLog("DKFileMap created. (length:%lu, writable, virtual)\n", size);
			DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
			fileMap->mapContext = reinterpret_cast<void*>(ctxt);
			return fileMap;
		}
		else
		{
			DKLog("CreateFileMapping(%lu) failed: %s\n", size, (const wchar_t*)GetWin32ErrorString(::GetLastError()));
		}
#else
		FileMapContext* ctxt = new FileMapContext();
		ctxt->file = -1;
		ctxt->prot = PROT_READ|PROT_WRITE;
		ctxt->flags = MAP_ANON|MAP_PRIVATE;
		ctxt->baseAddress = nullptr;
		ctxt->length = size;

		DKLog("DKFileMap created. (length:%lu, writable, virtual)\n", size);
		DKObject<DKFileMap> fileMap = DKObject<DKFileMap>::New();
		fileMap->mapContext = reinterpret_cast<void*>(ctxt);
		return fileMap;
#endif
	}
	return nullptr;
}
