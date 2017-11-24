//
//  File: DKFile.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#else
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#endif

#include "DKFile.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKUtils.h"
#include "DKUuid.h"

namespace DKFoundation
{
	namespace Private
	{
#ifdef _WIN32
		inline DKString GetErrorString(DWORD dwError)
		{
			DKString ret = L"";
			// error!
			LPVOID lpMsgBuf;
			::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

			ret = (const wchar_t*)lpMsgBuf;
			::LocalFree(lpMsgBuf);
			return ret;
		}
#endif
	}
}

#define DKFILE_INVALID_FILE_HANDLE		(-1)

using namespace DKFoundation;

DKFile::DKFile(void)
	: file(DKFILE_INVALID_FILE_HANDLE)
{
}

DKFile::~DKFile(void)
{
	if (this->file != DKFILE_INVALID_FILE_HANDLE)
	{
#ifdef _WIN32
		if (::CloseHandle((HANDLE)this->file) == 0)
		{
			DKLog("CloseHandle failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
		}
#else
		if (::close((int)this->file) != 0)
		{
			DKLog("close failed: %s\n", strerror(errno));
		}
#endif
	}
}

DKObject<DKFile> DKFile::Create(const DKString& file, ModeOpen mod, ModeShare share)
{
	if (file.Length() == 0)
		return NULL;

	DKString filePath = file.FilePathString();

#ifdef _WIN32
	DWORD fileAccess = 0;
	DWORD fileShareMode = 0;
	DWORD fileCreationDisposition = 0;

	switch (mod)
	{
	case ModeOpenNew:		// if file exist, truncate zero 
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileCreationDisposition = CREATE_ALWAYS;
		break;
	case ModeOpenCreate:
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileCreationDisposition = CREATE_NEW;
		break;
	case ModeOpenExisting:		// if file not exist error.
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileCreationDisposition = OPEN_EXISTING;
		break;
	case ModeOpenReadOnly:		// open exist file read-only
		fileAccess = GENERIC_READ;
		fileCreationDisposition = OPEN_EXISTING;
		break;
	case ModeOpenAlways:		// always open, even if not exist. (MODE_WRITE must specified if file not exist)
		fileAccess = GENERIC_READ | GENERIC_WRITE;
		fileCreationDisposition = OPEN_ALWAYS;
		break;
	default:
		DKLog("[DKFile] Incorrect mode parameter!\n");
		return NULL;
		break;
	}
	switch (share)
	{
	case ModeShareRead:
		fileShareMode = FILE_SHARE_READ;
		break;
	case ModeShareExclusive:
		fileShareMode = 0;
		break;
	case ModeShareAll:
		fileShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		break;
	default:
		DKLog("[DKFile] Incorrect mode parameter!\n");
		return NULL;
		break;
	}

	HANDLE hFile = ::CreateFileW((const wchar_t*)filePath, fileAccess, fileShareMode, NULL, fileCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DKObject<DKFile> f = DKObject<DKFile>::New();
		f->path = filePath;
		f->file = (intptr_t)hFile;
		f->modeOpen = mod;
		f->modeShare = share;
		DKASSERT_DEBUG(hFile == (HANDLE)f->file);
		return f;
	}
	else
	{
		DKLog("CreateFile(%ls) failed:%ls\n", (const wchar_t*)filePath, (const wchar_t*)Private::GetErrorString(::GetLastError()));
	}
#else
	int nOpenMode = 0;
	switch (mod)
	{
	case ModeOpenNew:		// if file exist, truncate zero 
		nOpenMode = O_CREAT | O_RDWR | O_TRUNC;
		break;
	case ModeOpenCreate:	// create new file, error if exist.
		nOpenMode = O_CREAT | O_RDWR | O_EXCL;
		break;
	case ModeOpenExisting:		// if file not exist error.
		nOpenMode = O_RDWR;
		break;
	case ModeOpenReadOnly:		// open exist file read-only
		nOpenMode = O_RDONLY;
		break;
	case ModeOpenAlways:		// always open, even if not exist. (MODE_WRITE must specified if file not exist)
		nOpenMode = O_CREAT | O_RDWR;
		break;
	default:
		DKLog("[DKFile] Incorrect mode parameter!\n");
		return NULL;
		break;
	}

	DKStringU8 filenameUTF8(filePath);
	if (filenameUTF8.Bytes() > 0)
	{
		int fd = ::open((const char*)filenameUTF8, nOpenMode, 00755);
		if (fd != -1)
		{
			if (::lseek(fd, 0, SEEK_SET) == -1)
				DKLog("lseek failed:%s\n", strerror(errno));

			if (mod != ModeOpenReadOnly)
			{
				struct  flock lock;
				lock.l_start = 0;
				lock.l_len = 0;
				lock.l_whence = SEEK_SET;
				lock.l_type = F_UNLCK;

				switch (share)
				{
				case ModeShareAll:
					lock.l_type = F_UNLCK;
					break;
				case ModeShareRead:
					lock.l_type = F_RDLCK;
					break;
				case ModeShareExclusive:
					lock.l_type = F_WRLCK;
					break;
				default:
					// lock - ??
					break;
				}
				if (::fcntl(fd, F_SETLK, &lock) == -1) // lock failed.
				{
					DKLog("fcntl failed:%s\n", strerror(errno));
					// ERROR!
					if (::close(fd) != 0)
						DKLog("close failed:%s\n", strerror(errno));

					fd = -1;
				}
			}
		}
		else
		{
			DKLog("open(%s) failed:%s\n", (const char*)filenameUTF8, strerror(errno));
		}

		if (fd != -1)
		{
			DKObject<DKFile> f = DKObject<DKFile>::New();
			f->path = file;
			f->file = fd;
			f->modeOpen = mod;
			f->modeShare = share;
			DKASSERT_DEBUG(fd == (ptrdiff_t)f->file);
			return f;
		}
	}
#endif
	return NULL;
}

DKObject<DKFile> DKFile::CreateTemporary(void)
{
#ifdef _WIN32
	DKString tmpPath = DKTemporaryDirectory();
	HANDLE hFile = INVALID_HANDLE_VALUE;
	while (hFile == INVALID_HANDLE_VALUE)
	{
		DKString filePath = tmpPath.FilePathStringByAppendingPath(DKUuid::Create().String());

		hFile = ::CreateFileW((const wchar_t*)filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW,
			FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_FLAG_DELETE_ON_CLOSE,
			NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD err = ::GetLastError();
			if (err != ERROR_FILE_EXISTS)
			{
				DKLog("CreateFile(%ls) failed:%s\n", (const wchar_t*)filePath, (const wchar_t*)Private::GetErrorString(err));
				break;
			}
		}
		else
		{
			DKLog("DKFile::CreateTemporary file:%ls created.\n", (const wchar_t*)filePath);
			DKObject<DKFile> f = DKObject<DKFile>::New();
			f->path = filePath;
			f->file = (intptr_t)hFile;
			f->modeOpen = ModeOpenCreate;
			f->modeShare = ModeShareExclusive;
			DKASSERT_DEBUG(hFile == (HANDLE)f->file);
			return f;
		}
	}
#else
	DKString tmpPath = DKTemporaryDirectory();
	DKStringU8 filePath = "";
	int fd = -1;
	while (fd == -1)
	{
		filePath = DKStringU8(tmpPath.FilePathStringByAppendingPath(DKUuid::Create().String()));
		if (filePath.Length() == 0)
		{
			DKLog("cannot create temp file!\n");
			break;
		}

#ifdef O_EXLOCK
		fd = ::open((const char*)filePath, O_RDWR | O_CREAT | O_EXCL | O_EXLOCK);
#else
		fd = ::open((const char*)filePath, O_RDWR | O_CREAT | O_EXCL);
		if (fd != -1)
			flock(fd, LOCK_EX);
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
			DKLog("DKFile::CreateTemporary file:%s created.\n", (const char*)filePath);
			if (::unlink((const char*)filePath) == 0)
			{
				DKObject<DKFile> f = DKObject<DKFile>::New();
				f->file = fd;
				f->path = L"";
				f->modeOpen = ModeOpenCreate;
				f->modeShare = ModeShareExclusive;
				DKASSERT_DEBUG(fd == (ptrdiff_t)f->file);
				return f;
			}
			else
			{
				DKLog("unlink(%s) failed: %s\n", (const char*)filePath, strerror(errno));
			}
			break;
		}
	}
#endif
	return NULL;
}

DKFile::Position DKFile::SetCurrentPosition(Position p)
{
	if (this->file != DKFILE_INVALID_FILE_HANDLE)
	{
#ifdef _WIN32
		LARGE_INTEGER pos, out;
		pos.QuadPart = p;
		out.QuadPart = -1;
		if (::SetFilePointerEx((HANDLE)this->file, pos, &out, FILE_BEGIN))
		{
			return out.QuadPart;
		}
		else
		{
			pos.QuadPart = 0;
			DKLog("SetFilePointer failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
		}

		pos.QuadPart = 0;
		if (::SetFilePointerEx((HANDLE)this->file, pos, &out, FILE_CURRENT))
			return out.QuadPart;
#else
		return ::lseek((int)this->file, p, SEEK_SET);
#endif
	}
	return -1;
}

DKFile::Position DKFile::CurrentPosition(void) const
{
	if (this->file != DKFILE_INVALID_FILE_HANDLE)
	{
#ifdef _WIN32
		LARGE_INTEGER pos, out;
		pos.QuadPart = 0;
		if (::SetFilePointerEx((HANDLE)this->file, pos, &out, FILE_CURRENT))
		{
			return out.QuadPart;
		}
		else
		{
			DKLog("SetFilePointer failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
		}
#else
		return ::lseek((int)this->file, 0, SEEK_CUR);
#endif
	}
	return -1;
}

DKFile::Position DKFile::RemainLength(void) const
{
	FileInfo info;
	if (GetInfo(info))
		return info.size - CurrentPosition();
	return 0;
}

DKFile::Position DKFile::TotalLength(void) const
{
	FileInfo info;
	if (GetInfo(info))
		return info.size;
	return -1;
}

DKObject<DKBuffer> DKFile::Read(size_t s, DKAllocator& alloc) const
{
	if (this->file == DKFILE_INVALID_FILE_HANDLE)
		return NULL;

	DKObject<DKBuffer> data = DKBuffer::Create(NULL, s, alloc);

	if (s > 0)
	{
		char* tmp = reinterpret_cast<char*>(data->LockExclusive());
		size_t bytesRead = 0;
		while (bytesRead < s)
		{
			int bytesToRead = 0x10000;
			if (bytesRead + bytesToRead > s)
				bytesToRead = s - bytesRead;
#ifdef _WIN32
			DWORD numRead = 0;
			if (::ReadFile((HANDLE)this->file, &tmp[bytesRead], bytesToRead, &numRead, 0) == 0)
				break;
			if (numRead == 0)
				break;
#else
			int numRead = ::read((int)this->file, &tmp[bytesRead], bytesToRead);
			if (numRead <= 0)
				break;
#endif
			bytesRead += numRead;
		}
		data->UnlockExclusive();

		if (bytesRead > 0)
		{
			if (bytesRead < s)
			{
				void* tmp2 = data->LockExclusive();
				DKObject<DKBuffer> data2 = DKBuffer::Create(tmp2, bytesRead, alloc);
				data->UnlockExclusive();
				data = data2;
			}
		}
		else
		{
			data->SetContent(0, 0);
		}
	}
	return data;
}

size_t DKFile::Read(void* p, size_t s) const
{
	if (this->file == DKFILE_INVALID_FILE_HANDLE)
		return (size_t)-1;

	if (s == 0)
		return 0;
	if (p == NULL)
		return 0;

	char* cp = reinterpret_cast<char*>(p);

	size_t bytesRead = 0;
	while (bytesRead < s)
	{
		int bytesToRead = 0x10000;
		if (bytesRead + bytesToRead > s)
			bytesToRead = s - bytesRead;

#ifdef _WIN32
		DWORD numRead = 0;
		if (::ReadFile((HANDLE)this->file, &cp[bytesRead], bytesToRead, &numRead, 0) == 0)
			break;
		if (numRead == 0)
			break;
#else
		int numRead = ::read((int)this->file, &cp[bytesRead], bytesToRead);
		if (numRead <= 0)
			break;
#endif
		bytesRead += numRead;
	}
	return bytesRead;
}

size_t DKFile::Read(void* p, size_t s)
{
	return static_cast<const DKFile*>(this)->Read(p, s);
}

size_t DKFile::Read(DKStream* p, size_t s) const
{
	if (this->file == DKFILE_INVALID_FILE_HANDLE)
		return (size_t)-1;

	if (p == NULL || p->IsWritable() == false)
		return 0;

	const size_t buffSize = 4096;
	char buff[buffSize];

	size_t bytesRead = 0;
	while (bytesRead < s)
	{
		int bytesToRead = buffSize;
		if (bytesRead + bytesToRead > s)
			bytesToRead = s - bytesRead;

#ifdef _WIN32
		DWORD numRead = 0;
		if (::ReadFile((HANDLE)this->file, buff, bytesToRead, &numRead, 0) == 0)
			break;
		if (numRead == 0)
			break;
#else
		int numRead = ::read((int)this->file, buff, bytesToRead);
		if (numRead <= 0)
			break;
#endif
		if (p->Write(buff, numRead) != numRead)
			break;
		bytesToRead += numRead;
	}
	return bytesRead;
}

size_t DKFile::Write(const void* p, size_t s)
{
	if (this->file == DKFILE_INVALID_FILE_HANDLE)
		return (size_t)-1;

	if (s == 0)
		return 0;
	if (p == NULL)
		return 0;

	const size_t platformMaxSize = 0x7fffffff;
	const char* cp = reinterpret_cast<const char*>(p);

	size_t totalWritten = 0;
	while (totalWritten < s)
	{
		size_t remains = s - totalWritten;
		int toWrite = (int)Min(remains, platformMaxSize);
#ifdef _WIN32
		DWORD numWrote = 0;
		if (::WriteFile((HANDLE)this->file, cp + totalWritten, toWrite, &numWrote, 0) == 0)
			break;
		if (numWrote == 0)
			break;
#else
		int numWrote = write((int)this->file, cp + totalWritten, toWrite);
		if (numWrote <= 0)
			break;
#endif
		totalWritten += numWrote;
	}
	return totalWritten;
}

size_t DKFile::Write(const DKData *p)
{
	if (p == NULL)
		return 0;

	const void* ptr = p->LockShared();
	size_t numWritten = Write(ptr, p->Length());
	p->UnlockShared();
	return numWritten;
}

size_t DKFile::Write(DKStream* s)
{
	if (s)
	{
		size_t len = s->RemainLength();
		if (len > 0)
		{
			void* p = DKMalloc(len);
			size_t numRead = s->Read(p, len);
			size_t numWritten = this->Write(p, numRead);
			DKFree(p);

			return numWritten;
		}
	}
	return 0;
}

bool DKFile::GetInfo(const DKString& file, FileInfo& info)
{
	if (file.Length() == 0)
		return false;

#ifdef _WIN32
	struct _stat64	st;
	if (_wstat64(file, &st) == -1)
		return false;
	if ((st.st_mode & _S_IFREG) == 0)
		return false;
	info.size = st.st_size;
	info.lastAccessed = DKDateTime(st.st_atime);
	info.lastModified = DKDateTime(st.st_mtime);
	info.created = DKDateTime(st.st_ctime);
	info.readable = _waccess(file, 04) != -1;
	info.writable = _waccess(file, 02) != -1;
#else
	// convert UTF-32 to UTF-8.
	DKStringU8 filenameUTF8(file);
	if (filenameUTF8.Bytes() > 0)
	{
		struct stat st;
		if (stat((const char*)filenameUTF8, &st) == -1)
			return false;
		if ((st.st_mode & S_IFREG) == 0)
			return false;

		info.size = st.st_size;
#if defined(__APPLE__) && defined(__MACH__)
		info.lastAccessed = DKDateTime(static_cast<double>(st.st_atimespec.tv_sec) + (static_cast<double>(st.st_atimespec.tv_nsec) / 1000000000.0f));
		info.lastModified = DKDateTime(static_cast<double>(st.st_mtimespec.tv_sec) + (static_cast<double>(st.st_mtimespec.tv_nsec) / 1000000000.0f));
		info.created = DKDateTime(static_cast<double>(st.st_ctimespec.tv_sec) + (static_cast<double>(st.st_ctimespec.tv_nsec) / 1000000000.0f));
#else
		info.lastAccessed = DKDateTime(st.st_atime, 0);
		info.lastModified = DKDateTime(st.st_mtime, 0);
		info.created = DKDateTime(st.st_ctime, 0);
#endif
		info.readable = access((const char*)filenameUTF8, R_OK) != -1;
		info.writable = access((const char*)filenameUTF8, W_OK) != -1;
	}
#endif

	return true;
}

bool DKFile::GetInfo(DKFile::FileInfo& info) const
{
	if (this->file == DKFILE_INVALID_FILE_HANDLE)
		return false;

#ifdef _WIN32
	BY_HANDLE_FILE_INFORMATION fileInfo;
	if (::GetFileInformationByHandle((HANDLE)this->file, &fileInfo))
	{
		union TimeUTC
		{
			int64_t ns100; // time since 1 Jan 1601 in 100ns units
			FILETIME ft;
		};

		LARGE_INTEGER fileSize;
		fileSize.HighPart = fileInfo.nFileSizeHigh;
		fileSize.LowPart = fileInfo.nFileSizeLow;

		TimeUTC ct, at, wt;
		ct.ft = fileInfo.ftCreationTime;
		at.ft = fileInfo.ftLastAccessTime;
		wt.ft = fileInfo.ftLastWriteTime;

		info.size = fileSize.QuadPart;
		info.created = DKDateTime((ct.ns100 / 10) & 1000000LL, (ct.ns100 - 116444736000000000LL) / 10000000LL);
		info.lastAccessed = DKDateTime((at.ns100 / 10) & 1000000LL, (at.ns100 - 116444736000000000LL) / 10000000LL);
		info.lastModified = DKDateTime((wt.ns100 / 10) & 1000000LL, (wt.ns100 - 116444736000000000LL) / 10000000LL);
	}
	else
		return false;
#else
	struct stat st;
	if (::fstat((int)this->file, &st) == -1)
		return false;
	if ((st.st_mode & S_IFREG) == 0)
		return false;

	info.size = st.st_size;
#if defined(__APPLE__) && defined(__MACH__)
	info.lastAccessed = DKDateTime(static_cast<double>(st.st_atimespec.tv_sec) + (static_cast<double>(st.st_atimespec.tv_nsec) / 1000000000.0f));
	info.lastModified = DKDateTime(static_cast<double>(st.st_mtimespec.tv_sec) + (static_cast<double>(st.st_mtimespec.tv_nsec) / 1000000000.0f));
	info.created = DKDateTime(static_cast<double>(st.st_ctimespec.tv_sec) + (static_cast<double>(st.st_ctimespec.tv_nsec) / 1000000000.0f));
#else
	info.lastAccessed = DKDateTime(st.st_atime, 0);
	info.lastModified = DKDateTime(st.st_mtime, 0);
	info.created = DKDateTime(st.st_ctime, 0);
#endif
#endif

	return true;
}

DKFile::FileInfo DKFile::GetInfo(void) const
{
	FileInfo info;
	if (GetInfo(info) == false)
	{
		DKERROR_THROW("GetInfo failed!");
	}
	return info;
}

bool DKFile::Delete(const DKString& file)
{
	if (file.Length() > 0)
	{
#ifdef _WIN32
		return ::DeleteFileW((const wchar_t*)file) != 0;
#else
		// convert UTF-32 to UTF-8.
		DKStringU8 filenameUTF8(file);
		if (filenameUTF8.Bytes() > 0)
		{
			return unlink((const char*)filenameUTF8) == 0;
		}
#endif
	}
	return false;
}

bool DKFile::SetLength(size_t len)
{
	if (this->file != DKFILE_INVALID_FILE_HANDLE)
	{
#ifdef _WIN32
		LARGE_INTEGER pos, out;
		pos.QuadPart = 0;
		out.QuadPart = 0;
		// get current position.
		if (::SetFilePointerEx((HANDLE)this->file, pos, &out, FILE_CURRENT))
		{
			// move to offset (desired length)
			pos.QuadPart = len;
			if (::SetFilePointerEx((HANDLE)this->file, pos, NULL, FILE_BEGIN))
			{
				// set end of file.
				if (::SetEndOfFile((HANDLE)this->file))
				{
					// back to previous position.
					if (::SetFilePointerEx((HANDLE)this->file, out, NULL, FILE_BEGIN) == 0)
					{
						DKLog("SetFilePointerEx failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					}
					return true;
				}
				else
				{
					DKLog("SetEndOfFile failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
			}
			else
			{
				DKLog("SetFilePointerEx failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
			}
		}
		else
		{
			DKLog("SetFilePointerEx failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
		}
#else
		if (::ftruncate((int)this->file, len) == 0)
		{
			return true;
		}
		else
		{
			DKLog("ftruncate failed:%s\n", ::strerror(errno));
		}
#endif
	}
	return false;
}

bool DKFile::IsReadable(void) const
{
	return (this->file != DKFILE_INVALID_FILE_HANDLE);
}

bool DKFile::IsWritable(void) const
{
	return (this->file != DKFILE_INVALID_FILE_HANDLE) && (this->modeOpen != ModeOpenReadOnly);
}

bool DKFile::IsSeekable(void) const
{
	return IsReadable();
}

const DKString& DKFile::Path(void) const
{
	return this->path;
}

DKObject<DKData> DKFile::MapContentRange(size_t offset, size_t length)
{
	if (this->file != DKFILE_INVALID_FILE_HANDLE)
	{
#ifdef _WIN32
		struct MappedContent : public DKData
		{
			~MappedContent(void)
			{
				if (::UnmapViewOfFile(ptr) == 0)
				{
					DKLog("UnmapViewOfFile failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
				if (::CloseHandle(hMap) == 0)
				{
					DKLog("CloseHandle failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
			}

			size_t Length(void) const { return length; }
			virtual bool IsReadable(void) const { return readable; }
			virtual bool IsWritable(void) const { return writable; }
			virtual bool IsExcutable(void) const { return false; }

			const void* LockShared(void) const { lock.LockShared(); return &ptr[offset]; }
			bool TryLockShared(const void** ptr) const
			{
				if (lock.TryLockShared())
				{
					if (ptr)
						*ptr = &ptr[offset];
					return true;
				}
				return false;
			}
			void UnlockShared(void) const { lock.UnlockShared(); }

			void* LockExclusive(void) { lock.Lock(); return &ptr[offset]; }
			bool TryLockExclusive(void** ptr)
			{
				if (lock.TryLock())
				{
					if (ptr)
						*ptr = &ptr[offset];
					return true;
				}
				return false;
			}
			void UnlockExclusive(void) { lock.Unlock(); }

			size_t length;
			size_t offset;
			bool readable;
			bool writable;
			HANDLE hMap;
			unsigned char* ptr;
			DKSharedLock lock;
		};

		bool readable = false;
		bool writable = false;
		if (this->IsReadable())
			readable = true;
		if (this->IsWritable())
			writable = true;

		DWORD protect = 0;
		if (readable && writable)
			protect = PAGE_READWRITE;
		else
			protect = PAGE_READONLY;

		HANDLE hMap = ::CreateFileMappingW((HANDLE)this->file, NULL, protect, 0, 0, NULL);
		if (hMap)
		{
			DWORD access = writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ;
			LARGE_INTEGER mapOffset;
			mapOffset.QuadPart = offset;

			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			DKASSERT_DEBUG(sysInfo.dwAllocationGranularity != 0);

			if (mapOffset.QuadPart % sysInfo.dwAllocationGranularity)		// alignment 맞춤.
				mapOffset.QuadPart -= (mapOffset.QuadPart % sysInfo.dwAllocationGranularity);

			size_t mapLength = length + (offset - mapOffset.QuadPart);

			LPVOID ptr = ::MapViewOfFile(hMap, access, mapOffset.HighPart, mapOffset.LowPart, mapLength);
			if (ptr)
			{
				DKObject<MappedContent> data = DKObject<MappedContent>::New();
				data->length = length;
				data->offset = offset - mapOffset.QuadPart;
				data->readable = readable;
				data->writable = writable;
				data->hMap = hMap;
				data->ptr = reinterpret_cast<unsigned char*>(ptr);
				return data.SafeCast<DKData>();
			}
			else
			{
				DKLog("MapViewOfFile failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
			}
			::CloseHandle(hMap);
		}
		else
		{
			DKLog("CreateFileMapping failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
		}
#else
		struct MappedContent : public DKData
		{
			~MappedContent(void)
			{
				if (::munmap(ptr, length + offset) != 0)
				{
					DKLog("munmap failed:%s\n", strerror(errno));
				}
			}

			size_t Length(void) const { return length; }
			bool IsReadable(void) const { return (prot & PROT_READ) != 0; }
			bool IsWritable(void) const { return (prot & PROT_WRITE) != 0; }
			bool IsExcutable(void) const { return false; }

			const void* LockShared(void) const { lock.LockShared(); return &ptr[offset]; }
			bool TryLockShared(const void** ptr) const
			{
				if (lock.TryLockShared())
				{
					if (ptr)
						*ptr = &ptr[offset];
					return true;
				}
				return false;
			}
			void UnlockShared(void) const { lock.UnlockShared(); }

			void* LockExclusive(void) { lock.Lock(); return &ptr[offset]; }
			bool TryLockExclusive(void** ptr)
			{
				if (lock.TryLock())
				{
					if (ptr)
						*ptr = &ptr[offset];
					return true;
				}
				return false;
			}
			void UnlockExclusive(void) { lock.Unlock(); }

			int prot;
			size_t length;
			size_t offset;
			unsigned char* ptr;
			DKSharedLock lock;
		};

		int prot = PROT_NONE;
		if (this->IsReadable())
			prot |= PROT_READ;
		if (this->IsWritable())
			prot |= PROT_WRITE;

		long pageSize = DKMemoryPageSize();
		DKASSERT_DEBUG(pageSize > 0);
		size_t mapOffset = offset & ~(pageSize - 1);
		DKASSERT_DEBUG(offset >= mapOffset);

		void* p = ::mmap(0, length + (offset - mapOffset), prot, MAP_FILE | MAP_SHARED, (int)this->file, mapOffset);
		if (p == MAP_FAILED)
		{
			DKLog("mmap failed:%s\n", strerror(errno));
		}
		else
		{
			DKObject<MappedContent> data = DKObject<MappedContent>::New();
			data->prot = prot;
			data->offset = (offset - mapOffset);
			data->length = length;
			data->ptr = reinterpret_cast<unsigned char*>(p);
			return data.SafeCast<DKData>();
		}
#endif
	}
	return NULL;
}

