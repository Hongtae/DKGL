//
//  File: DKMemory.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../DKInclude.h"
#include <stdlib.h>
#include <memory.h>
#include <new>

#include "DKMap.h"
#include "DKMemory.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKUtils.h"
#include "DKUUID.h"

#ifdef _WIN32
#include <windows.h>
#else
#if defined(__APPLE__) && defined(__MACH__)
#include <mach/vm_statistics.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif


namespace DKFoundation
{
	namespace Private
	{


		static void* MemAlloc(size_t s) // works in spite of s = 0
		{
			return ::malloc(s);
		}

		static void MemFree(void* p)
		{
			return ::free(p);
		}

		static void* MemRealloc(void* p, size_t s)
		{
			if (p == NULL)
			{
				return MemAlloc(s);
			}
			else if (s == 0)
			{
				MemFree(p);
				return NULL;
			}


			return ::realloc(p, s);
		}


#ifdef _WIN32
		static inline DKString GetErrorString(DWORD dwError)
		{
			DKString ret = L"";
			// error!
			LPVOID lpMsgBuf;
			::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
							 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &lpMsgBuf, 0, NULL );

			ret = (const wchar_t*)lpMsgBuf;
			::LocalFree(lpMsgBuf);
			return ret;
		}
#endif

		static DKSpinLock initLock; // used one time when initialize.
		template <typename T, unsigned int length> struct AtomicInitializedStaticArray
		{
			enum {ArrayLength = length};
			static T* Value(void)
			{
				static T* v = NULL;
				if (v == NULL)
				{
					DKCriticalSection<DKSpinLock> guard(initLock);
					if (v == NULL)
					{
						static T values[ArrayLength];
						v = values;
					}
				}
				return v;
			}
			static T& ValueAtIndex(unsigned int index)
			{
				return Value()[index];
			}
		};

#ifndef _WIN32
		enum {VirtualMemoryTableLength = 19};
		struct VirtualMemoryTable
		{
			typedef DKSpinLock Lock;
			typedef DKCriticalSection<Lock> CriticalSection;
			typedef DKMap<void*, size_t> Container;

			Lock lock;
			Container container;
		};
		static VirtualMemoryTable& GetVirtualMemoryTable(void* ptr)
		{
			return AtomicInitializedStaticArray<VirtualMemoryTable, VirtualMemoryTableLength>::ValueAtIndex(reinterpret_cast<uintptr_t>(ptr) % VirtualMemoryTableLength);
		}
#endif
		enum {FileMemoryTableLength = 19};
		struct FileMemoryTable
		{
			struct Node
			{
#ifdef _WIN32
				HANDLE file;
				HANDLE map;
#else
				int fd;
#endif
				size_t length;
			};
			typedef DKSpinLock Lock;
			typedef DKCriticalSection<Lock> CriticalSection;
			typedef DKMap<void*, Node> Container;

			Lock lock;
			Container container;
		};

		static FileMemoryTable& GetFileMemoryTable(void* ptr)
		{
			return AtomicInitializedStaticArray<FileMemoryTable, FileMemoryTableLength>::ValueAtIndex(reinterpret_cast<uintptr_t>(ptr) % FileMemoryTableLength);
		}

		static bool PrintFileMemoryStat(const FileMemoryTable::Node& fnode)
		{
#ifdef _WIN32
			if (fnode.file != INVALID_HANDLE_VALUE)
			{
				BY_HANDLE_FILE_INFORMATION info;
				if (::GetFileInformationByHandle(fnode.file, &info))
				{
					DKLog("PrintFileMemoryStat for handle: %p\n", fnode.file);
					DKLog("info.fileAttr = %x\n", info.dwFileAttributes);
					DKLog("info.volumeSerial = %x\n", info.dwVolumeSerialNumber);
					DKLog("info.fileSizeH = %d\n", info.nFileSizeHigh);
					DKLog("info.fileSizeL = %d\n", info.nFileSizeLow);
					DKLog("info.fileLink = %d\n", info.nNumberOfLinks);
					DKLog("info.fileIndexH = %d\n", info.nFileIndexHigh);
					DKLog("info.fileIndexL = %d\n", info.nFileIndexLow);
					DKLog("PrintFileMemoryStat end ------------\n");

					return true;

				}
				else
				{
					DKLog("GetFileInformationByHandle failed:%ls\n", (const wchar_t*)GetErrorString(::GetLastError()));
				}
			}
#else
			if (fnode.fd != -1)
			{
				struct stat st;
				if (::fstat(fnode.fd, &st) == 0)
				{
					DKLog("PrintFileMemoryStat for fd: %d\n", fnode.fd);
					DKLog("stat.st_dev: %d\n", st.st_dev);
					DKLog("stat.st_ino: %llu\n", st.st_ino);
					DKLog("stat.st_mode: %d\n", st.st_mode);
					DKLog("stat.st_nlink: %d\n", st.st_nlink);
					DKLog("stat.st_uid: %d\n", st.st_uid);
					DKLog("stat.st_gid: %d\n", st.st_gid);
					DKLog("stat.st_rdev: %d\n", st.st_rdev);
					DKLog("stat.st_size: %llu\n", st.st_size);
					DKLog("stat.st_blocks: %lld\n", st.st_blocks);
					DKLog("stat.st_blksize: %d\n", st.st_blksize);
					//DKLog("stat.st_flags: %d\n", st.st_flags);
					//DKLog("stat.st_gen: %d\n", st.st_gen);
					DKLog("PrintFileMemoryStat end ------------\n");

					return true;
				}
				else
				{
					DKLog("fstat(%d) failed: %s\n", fnode.fd, strerror(errno));
				}
			}
#endif
			else
			{
				DKLog("PrintFileInfo failed (fd == -1)\n");
			}
			return false;
		}

		enum {PageTableLength = 19};
		struct PageTable
		{
			typedef DKSpinLock Lock;
			typedef DKCriticalSection<Lock> CriticalSection;
			typedef DKMap<void*, size_t> Container;

			Lock lock;
			Container container;
		};

		static PageTable& GetPageTable(void* ptr)
		{
			return AtomicInitializedStaticArray<PageTable, FileMemoryTableLength>::ValueAtIndex(reinterpret_cast<uintptr_t>(ptr) % FileMemoryTableLength);
		}
	}
}


namespace DKFoundation
{
	DKLIB_API void* DKMemoryHeapAlloc(size_t s)
	{
		return ::malloc(s);
	}

	DKLIB_API void* DKMemoryHeapRealloc(void* p, size_t s)
	{
		return ::realloc(p, s);
	}

	DKLIB_API void DKMemoryHeapFree(void* p)
	{
		return ::free(p);
	}

	DKLIB_API void* DKMemoryVirtualAlloc(size_t s)
	{
		void* ptr = NULL;

		size_t pageSize = DKMemoryPageSize();
		DKASSERT_DEBUG(pageSize != 0);

		if (s == 0)
			s = pageSize;
		else if (s % pageSize)
			s += pageSize - (s % pageSize);

		DKASSERT_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
		ptr = ::VirtualAlloc(0, s, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
#else
		ptr = ::mmap(0, s, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
		if (ptr == MAP_FAILED)
		{
			perror("mmap failed");
		}
		else
		{
			Private::VirtualMemoryTable& vt = Private::GetVirtualMemoryTable(ptr);
			Private::VirtualMemoryTable::CriticalSection guard(vt.lock);

			if (vt.container.Insert(ptr, s))
			{
			}
			else
			{
				DKERROR_THROW_DEBUG("VT Map insertion failed.");
				// unmap.
				if (::munmap(ptr, s) != 0)
				{
					DKERROR_THROW_DEBUG("munmap failed");
					perror("munmap failed");
				}
				ptr = NULL;
			}
		}
#endif
		DKASSERT_DEBUG(ptr);
		return ptr;
	}

	DKLIB_API void* DKMemoryVirtualRealloc(void* p, size_t s)
	{
		if (p && s > 0) // realloc
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_DEBUG(pageSize != 0);
			if (s % pageSize > 0)
				s += pageSize - (s % pageSize);
#ifdef _WIN32
			SIZE_T bytesOld = 0;
			LPVOID ptr = NULL;
			MEMORY_BASIC_INFORMATION mbiStatus;

			//Check size of current array in bytes, quit if no bytes available.
			if(!VirtualQuery(p, &mbiStatus, sizeof(mbiStatus)))
				return NULL;
			bytesOld = mbiStatus.RegionSize;
			if(bytesOld == 0)
				return NULL;

			if (bytesOld == s)
				return p;

			DKASSERT_DEBUG((bytesOld % pageSize) == 0);
			DKASSERT_DEBUG((s % pageSize) == 0);

			if (bytesOld == s)
				return p;

			//Create a new array, quit if failed to create.
			ptr = VirtualAlloc(NULL, s, MEM_COMMIT, PAGE_READWRITE);
			if(ptr == NULL)
				return NULL;

			VirtualQuery(ptr, &mbiStatus, sizeof(mbiStatus));

			//Copy old array onto new array, making sure not to exceed any boundries.
			if(bytesOld > mbiStatus.RegionSize)
				bytesOld = mbiStatus.RegionSize;

			CopyMemory(ptr, p, bytesOld);

			//Free old array and return address of new array.
			VirtualFree(p, 0, MEM_RELEASE);
			return ptr;
#else
			Private::VirtualMemoryTable& vt = Private::GetVirtualMemoryTable(p);
			Private::VirtualMemoryTable::CriticalSection guard(vt.lock);
			Private::VirtualMemoryTable::Container::Pair* pair = vt.container.Find(p);
			DKASSERT_DEBUG(pair);

			if (pair)
			{
				size_t size1 = pair->value;
				size_t size2 = s;

				DKASSERT_DEBUG((size1 % pageSize) == 0);
				DKASSERT_DEBUG((size2 % pageSize) == 0);

				if (size1 == size2)
				{
					DKLog("realloc %lu -> %lu bytes ptr:%p (no change)\n", size1, size2, p);
					return p;
				}

				DKLog("realloc %lu -> %lu bytes ptr:%p \n", size1, size2, p);

				if (size1 > size2) // shrink
				{
					unsigned char* p2 = reinterpret_cast<unsigned char*>(p) + size2;
					size_t len = size1 - size2;

					DKLog("munmap(%p, %lu)\n", p2, len);
					if (::munmap(p2, len) != 0)
					{
						perror("unmap error");
						// get a new chunk and copy.
					}
					else
					{
						pair->value = size2;
						return p;
					}
				}
				else
				{
					unsigned char* p2 = reinterpret_cast<unsigned char*>(p) + size1;
					size_t len = size2 - size1;
					// mmap on p2 with len length.
					// recall mmap for entire region and copy if failed.
					void* p3 = ::mmap(p2, len, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
					if (p3 == MAP_FAILED)
					{
						perror("mmap failed");
						// failed, mmap for entire region and copy!
					}
					else
					{
						if (p2 == reinterpret_cast<unsigned char*>(p3))
						{
							DKLog("mmap returns good position.\n");
							pair->value = size2;
							return p;
						}
						else
						{
							printf("mmap returns unwanted location(%p != %p). unmap and realloc\n", p2, p3);
							if (::munmap(p3, len) != 0)
							{
								perror("unmap error");
							}
						}
					}
				}
				// mmap with specified range and copy.
				void* p2 = ::mmap(0, size2, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
				if (p2 == MAP_FAILED)
				{
					perror("mmap failed");
					p2 = NULL;
				}
				else
				{
					vt.container.Insert(p2, size2);

					size_t len = 0;
					if (size1 > size2)
						len = size2 - sizeof(size_t);
					else
						len = size1 - sizeof(size_t);

					DKLog("copying %lu bytes\n", len);
					::memcpy(p2, p, len);
				}
				if (::munmap(p, size1) != 0)
				{
					perror("unmap error");
				}
				vt.container.Remove(p);
				return p2;
			}
#endif
		}
		else if (p) // release
		{
			DKMemoryVirtualFree(p);
		}
		else if (s > 0) // new alloc
		{
			return DKMemoryVirtualAlloc(s);
		}
		return NULL;
	}

	DKLIB_API void DKMemoryVirtualFree(void* p)
	{
		if (p)
		{
#ifdef _WIN32
			BOOL ret = ::VirtualFree(p, 0, MEM_RELEASE);
			DKASSERT_DEBUG(ret != 0);
#else
			Private::VirtualMemoryTable& vt = Private::GetVirtualMemoryTable(p);
			Private::VirtualMemoryTable::CriticalSection guard(vt.lock);
			Private::VirtualMemoryTable::Container::Pair* pair = vt.container.Find(p);
			if (pair)
			{
				size_t pageSize = getpagesize();

				DKASSERT_DEBUG(pair->value > 0);
				DKASSERT_DEBUG((pair->value % pageSize) == 0);
				if (::munmap(p, pair->value) != 0)
				{
					perror("munmap failed");
				}
				vt.container.Remove(p);
			}
			else
			{
				DKERROR_THROW_DEBUG("unallocated address");
			}
#endif
		}
	}

	DKLIB_API void* DKMemoryFileAlloc(size_t s)
	{
		DKString tmpPath = DKTemporaryDirectory();

#ifdef _WIN32
		HANDLE hFile = INVALID_HANDLE_VALUE;
		while (hFile == INVALID_HANDLE_VALUE)
		{
			DKString filePath(tmpPath.FilePathStringByAppendingPath(DKUUID::Create().String()));
			if (filePath.Length() == 0)
			{
				DKLog("cannot locate temp file!\n");
				break;
			}

			hFile = ::CreateFileW((const wchar_t*)filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_DELETE_ON_CLOSE, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD err = ::GetLastError();
				// stop if not in case of file exists.
				if (err != ERROR_FILE_EXISTS)
				{
					DKLog("CreateFile(%ls) failed:%s\n", (const wchar_t*)filePath, (const wchar_t*)Private::GetErrorString(err));
					break;
				}
			}
			else
			{
				LARGE_INTEGER size;
				size.QuadPart = s;

				if (::SetFilePointerEx(hFile, size, NULL, FILE_BEGIN) && ::SetEndOfFile(hFile))
				{
					HANDLE hMap = ::CreateFileMappingW(hFile, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, NULL);
					if (hMap != NULL)
					{
						LPVOID ptr = ::MapViewOfFileEx(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);
						if (ptr)
						{
							Private::FileMemoryTable::Node fnode;

							fnode.file = hFile;
							fnode.map = hMap;
							fnode.length = s;

							DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));

							Private::FileMemoryTable& ftable = Private::GetFileMemoryTable(ptr);
							DKCriticalSection<DKSpinLock> guard(ftable.lock);
							if (ftable.container.Insert(ptr, fnode) == false)
							{
								DKERROR_THROW_DEBUG("cannot insert container?");
							}

							DKLog("Memory File Created: %ls (%llu bytes)\n", (const wchar_t*)filePath, s);

							return ptr;
						}
						if (::CloseHandle(hMap) == 0)
						{
							DKLog("CloseHandle(%p) failed:%ls\n", hMap, (const wchar_t*)Private::GetErrorString(::GetLastError()));
						}
					}
					else
					{
						DKLog("CreateFileMapping failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					}
				}
				else
				{
					DKLog("File truncate failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}

				if (::CloseHandle(hFile) == 0)
				{
					DKLog("CloseHandle(%p) failed:%ls\n", hFile, (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
				break;
			}
		}
#else
		int fd = -1;
		while (fd == -1)
		{
			DKStringU8 filePath(tmpPath.FilePathStringByAppendingPath(DKUUID::Create().String()));
			if (filePath.Length() == 0)
			{
				DKLog("cannot locate temp file!\n");
				break;
			}
#ifdef O_EXLOCK
			fd = ::open((const char*)filePath, O_RDWR|O_CREAT|O_EXCL|O_EXLOCK);
#else
			fd = ::open((const char*)filePath, O_RDWR|O_CREAT|O_EXCL);
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
				if (::unlink((const char*)filePath) == 0)
				{
#if defined(__APPLE__) && defined(__MACH__)
					if (fcntl(fd, F_NOCACHE, 1) == -1)
						DKLog("fcntl failed: %s\n", strerror(errno));
					if (fcntl(fd, F_FULLFSYNC) == -1)
						DKLog("fcntl failed: %s\n", strerror(errno));
#endif
					if (::ftruncate(fd, s) == 0)
					{
#if defined(__APPLE__) && defined(__MACH__)
						void* p = ::mmap(0, s, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED|MAP_NOCACHE, fd, 0);
#else
						void* p = ::mmap(0, s, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fd, 0);
#endif
						if (p != MAP_FAILED)
						{
							Private::FileMemoryTable::Node fnode;
							fnode.fd = fd;
							fnode.length = s;

							DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));

							Private::FileMemoryTable& ftable = Private::GetFileMemoryTable(p);
							DKCriticalSection<DKSpinLock> guard(ftable.lock);
							if (ftable.container.Insert(p, fnode) == false)
							{
								DKERROR_THROW_DEBUG("cannot insert container?");
							}
							return p;
						}
						else
						{
							DKLog("mmap failed:%s\n", ::strerror(errno));
						}
					}
					else
					{
						DKLog("ftruncate(%s, %lu) failed: %s\n", (const char*)filePath, s, strerror(errno));
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
		return NULL;
	}

	DKLIB_API void* DKMemoryFileRealloc(void* p, size_t s)
	{
		if (p && s > 0)
		{
			Private::FileMemoryTable& ftable = Private::GetFileMemoryTable(p);
			DKCriticalSection<DKSpinLock> guard(ftable.lock);
			Private::FileMemoryTable::Container::Pair* pair = ftable.container.Find(p);
			if (pair)
			{
				Private::FileMemoryTable::Node& fnode = pair->value;

				DKASSERT_DEBUG(fnode.length > 0);
				DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));
#ifdef _WIN32
				if (::UnmapViewOfFile(p))
				{
					if (::CloseHandle(fnode.map))
					{
						fnode.map = NULL;

						LARGE_INTEGER size;
						size.QuadPart = s;
						if (::SetFilePointerEx(fnode.file, size, NULL, FILE_BEGIN) && ::SetEndOfFile(fnode.file))
						{
							fnode.length = s;

							HANDLE hMap = ::CreateFileMappingW(fnode.file, NULL, PAGE_READWRITE, size.HighPart, size.LowPart, NULL);
							if (hMap)
							{
								fnode.map = hMap;

								LPVOID p2 = ::MapViewOfFileEx(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0, p);
								if (p2 == NULL)
									p2 = ::MapViewOfFileEx(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0, NULL);

								if (p2)
								{
									DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));

									if (p2 != p)
									{
										Private::FileMemoryTable& ftable2 = Private::GetFileMemoryTable(p2);
										if (&ftable == &ftable2)
										{
										}
										else
										{
											DKCriticalSection<DKSpinLock> guard2(ftable2.lock);
											if (ftable2.container.Insert(p2, fnode) == false)
											{
												DKERROR_THROW_DEBUG("cannot insert container?");
											}
											ftable.container.Remove(p);
										}
										p = p2;
									}
									return p;
								}
							}
							else
							{
								DKLog("CreateFileMapping failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
							}
						}
						else
						{
							DKLog("File truncate failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
						}
					}
					else
					{
						DKLog("ClsoeHandle failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
						DKERROR_THROW_DEBUG("CloseHandle failed!");
					}
				}
				else
				{
					DKLog("UnmapViewOfFile failed:l%s\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					DKERROR_THROW_DEBUG("munmap failed!");
				}
				if (fnode.map)
				{
					if (::CloseHandle(fnode.map) == 0)
						DKLog("CloseHandle(%p) failed: %ls\n", fnode.map, (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
				if (fnode.file != INVALID_HANDLE_VALUE)
				{
					if (::CloseHandle(fnode.file) == 0)
						DKLog("CloseHandle(%p) failed: %ls\n", fnode.file, (const wchar_t*)Private::GetErrorString(::GetLastError()));
				}
#else
				if (::munmap(p, fnode.length) == 0)
				{
					if (::ftruncate(fnode.fd, s) == 0)
					{
#if defined(__APPLE__) && defined(__MACH__)
						void* p2 = ::mmap(p, s, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED|MAP_NOCACHE, fnode.fd, 0);
#else
						void* p2 = ::mmap(p, s, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, fnode.fd, 0);
#endif
						if (p2 != MAP_FAILED)
						{
							fnode.length = s;

							DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));

							if (p2 != p)
							{
								Private::FileMemoryTable& ftable2 = Private::GetFileMemoryTable(p2);
								if (&ftable == &ftable2)
								{
								}
								else
								{
									DKCriticalSection<DKSpinLock> guard2(ftable2.lock);
									if (ftable2.container.Insert(p2, fnode) == false)
									{
										DKERROR_THROW_DEBUG("cannot insert container?");
									}
									ftable.container.Remove(p);
								}
								p = p2;
							}
							return p;
						}
						else
						{
							DKLog("mmap failed: %s\n", strerror(errno));
						}
					}
					else
					{
						DKLog("ftruncate failed: %s\n", strerror(errno));
					}
				}
				else
				{
					DKLog("munmap failed: %s\n", strerror(errno));
					DKERROR_THROW_DEBUG("munmap failed!");
				}
				if (::close(fnode.fd) != 0)
				{
					DKLog("close failed: %s\n", strerror(errno));
				}
#endif
				ftable.container.Remove(p);
			}
			else
			{
				DKERROR_THROW_DEBUG("Cannot find alloc-info with this address.");
			}
		}
		else if (p)
		{
			DKMemoryFileFree(p);
		}
		else if (s > 0)
		{
			return DKMemoryFileAlloc(s);
		}
		return NULL;
	}

	DKLIB_API void DKMemoryFileFree(void* p)
	{
		if (p)
		{
			Private::FileMemoryTable& ftable = Private::GetFileMemoryTable(p);
			DKCriticalSection<DKSpinLock> guard(ftable.lock);
			Private::FileMemoryTable::Container::Pair* pair = ftable.container.Find(p);
			if (pair)
			{
				Private::FileMemoryTable::Node& fnode = pair->value;

				DKASSERT_DEBUG(fnode.length > 0);
				DKASSERT_DEBUG(Private::PrintFileMemoryStat(fnode));
#ifdef _WIN32
				if (::UnmapViewOfFile(p) == 0)
				{
					DKLog("UnmapViewOfFile failed:l%s\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					DKERROR_THROW_DEBUG("munmap failed!");
				}
				if (::CloseHandle(fnode.map) == 0)
				{
					DKLog("ClsoeHandle failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					DKERROR_THROW_DEBUG("CloseHandle failed!");
				}
				if (::CloseHandle(fnode.file) == 0)
				{
					DKLog("ClsoeHandle failed:%ls\n", (const wchar_t*)Private::GetErrorString(::GetLastError()));
					DKERROR_THROW_DEBUG("CloseHandle failed!");
				}
#else
				if (::munmap(p, fnode.length) != 0)
				{
					DKLog("munmap failed: %s\n", strerror(errno));
					DKERROR_THROW_DEBUG("munmap failed!");
				}
				if (::close(fnode.fd) != 0)
				{
					DKLog("close failed: %s\n", strerror(errno));
				}
#endif
				ftable.container.Remove(p);
			}
			else
			{
				DKERROR_THROW_DEBUG("Cannot find alloc-info with this address.");
			}
		}
	}

	DKLIB_API size_t DKMemoryPageSize(void)
	{
#ifdef _WIN32
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwPageSize;
#else
		return getpagesize();
#endif
		return 1024;
	}

	DKLIB_API void* DKMemoryPageReserve(void* p, size_t s)
	{
		void* ptr = NULL;

		size_t pageSize = DKMemoryPageSize();
		DKASSERT_DEBUG(pageSize != 0);

		if (s == 0)
			s = pageSize;
		else if (s % pageSize)
			s += pageSize - (s % pageSize);

		DKASSERT_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
		ptr = ::VirtualAlloc(p, s, MEM_RESERVE, PAGE_READWRITE);
		if (ptr == NULL)
			ptr = ::VirtualAlloc(0, s, MEM_RESERVE, PAGE_READWRITE);
#else
		ptr = ::mmap(0, s, PROT_NONE, MAP_ANON|MAP_PRIVATE, -1, 0);
		if (ptr == MAP_FAILED)
		{
			DKLog("mmap failed: %s\n", strerror(errno));
			ptr = NULL;
		}
#endif
		if (ptr)
		{
			Private::PageTable& pt = Private::GetPageTable(ptr);
			Private::PageTable::CriticalSection guard(pt.lock);
			if (pt.container.Insert(ptr, s))
			{
			}
			else
			{
				DKERROR_THROW_DEBUG("VT Map insertion failed.");
#ifdef _WIN32
				if (::VirtualFree(ptr, 0, MEM_RELEASE) == 0)
				{
					DKERROR_THROW_DEBUG("VirtualFree failed");
					DKLog("VirtualFree failed:%ls\n", Private::GetErrorString(::GetLastError()));
				}
#else
				// unmap.
				if (::munmap(ptr, s) != 0)
				{
					DKERROR_THROW_DEBUG("munmap failed");
					DKLog("munmap failed:%s\n", strerror(errno));
				}
#endif
				ptr = NULL;
			}
		}
		return ptr;
	}

	DKLIB_API void DKMemoryPageRelease(void* p)
	{
		if (p)
		{
			Private::PageTable& pt = Private::GetPageTable(p);
			Private::PageTable::CriticalSection guard(pt.lock);
			Private::PageTable::Container::Pair* pair = pt.container.Find(p);
			if (pair)
			{
				size_t pageSize = DKMemoryPageSize();
				DKASSERT_DEBUG(pair->value > 0);
				DKASSERT_DEBUG((pair->value % pageSize) == 0);
#ifdef _WIN32
#ifdef DKLIB_DEBUG_ENABLED
				MEMORY_BASIC_INFORMATION mbi;
				if (::VirtualQuery(p, &mbi, sizeof(mbi)))
				{
					DKASSERT_DEBUG(mbi.BaseAddress == p);
					DKASSERT_DEBUG(mbi.RegionSize == pair->value);
				}
#endif
				if (::VirtualFree(p, 0, MEM_RELEASE) == 0)
				{
					DKERROR_THROW_DEBUG("VirtualFree failed");
					DKLog("VirtualFree failed:%ls\n", Private::GetErrorString(::GetLastError()));
				}
#else
				if (::munmap(p, pair->value) != 0)
				{
					DKERROR_THROW_DEBUG("munmap failed");
					DKLog("munmap failed:%s\n", strerror(errno));
				}
#endif
				pt.container.Remove(p);
			}
			else
			{
				DKERROR_THROW_DEBUG("unallocated address");
			}
		}
	}

	DKLIB_API void DKMemoryPageCommit(void* p, size_t s)
	{
		if (p && s > 0)
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_DEBUG(pageSize != 0);

			s = Max(s, pageSize);

			if (s % pageSize)
				s += pageSize - (s % pageSize);

			DKASSERT_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
#ifdef DKLIB_DEBUG_ENABLED
			MEMORY_BASIC_INFORMATION mbi;
			if (::VirtualQuery(p, &mbi, sizeof(mbi)))
			{
				size_t beginAddr = reinterpret_cast<size_t>(mbi.BaseAddress);
				size_t endAddr = reinterpret_cast<size_t>(mbi.BaseAddress) + mbi.RegionSize;

				DKASSERT_DEBUG(reinterpret_cast<size_t>(p) >= beginAddr);
				DKASSERT_DEBUG(reinterpret_cast<size_t>(p) <= endAddr);
				DKASSERT_DEBUG(reinterpret_cast<size_t>(p) + s >= beginAddr);
				DKASSERT_DEBUG(reinterpret_cast<size_t>(p) + s <= endAddr);
			}
#endif
			void* baseAddr = ::VirtualAlloc(p, s, MEM_COMMIT, PAGE_READWRITE);
			if (baseAddr == NULL)
			{
				DKERROR_THROW_DEBUG("VirtualAlloc failed");
				DKLog("VirtualAlloc failed:%ls\n", Private::GetErrorString(::GetLastError()));
			}
#else
			while (mprotect(p, s, PROT_READ|PROT_WRITE) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
			while (madvise(p, s, MADV_WILLNEED) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
#endif
		}
	}

	DKLIB_API void DKMemoryPageDecommit(void* p, size_t s)
	{
		if (p && s > 0)
		{
			size_t pageSize = DKMemoryPageSize();
			DKASSERT_DEBUG(pageSize != 0);

			s = Max(s, pageSize);

			if (s % pageSize)
				s += pageSize - (s % pageSize);

			DKASSERT_DEBUG((s % pageSize) == 0);

#ifdef _WIN32
			if (::VirtualFree(p, s, MEM_DECOMMIT) == 0)
			{
				DKERROR_THROW_DEBUG("VirtualFree failed");
				DKLog("VirtualFree failed:%ls\n", Private::GetErrorString(::GetLastError()));
			}
#else
			while (madvise(p, s, MADV_DONTNEED) == -1)
			{
				DKLog("madvice failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
			while (mprotect(p, s, PROT_NONE) == -1)
			{
				DKLog("mprotect failed:%s\n", strerror(errno));
				if (errno != EAGAIN)
					break;
			}
#endif
		}
	}
	
	DKLIB_API void* DKMemoryPoolAlloc(size_t s)
	{
		return Private::MemAlloc(s);
	}
	
	DKLIB_API void* DKMemoryPoolRealloc(void* p, size_t s)
	{
		return Private::MemRealloc(p, s);
	}
	
	DKLIB_API void DKMemoryPoolFree(void* p)
	{
		return Private::MemFree(p);
	}
}
