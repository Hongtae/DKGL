//
//  File: DKDirectory.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#ifdef _WIN32
	#include <windows.h>
	#include <wchar.h>
	#define DIRECTORY_PATH_SEPARATOR		L'\\'
#else
	#include <sys/stat.h>
	#include <dirent.h>
	#include <unistd.h>
	#define DIRECTORY_PATH_SEPARATOR		L'/'
#endif

#include "DKDirectory.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKFile.h"
#include "DKFileMap.h"

using namespace DKFoundation;

DKDirectory::DKDirectory(void)
: currentPath(L"")
, reloadRequired(true)
{
}

DKDirectory::~DKDirectory(void)
{
}

DKObject<DKDirectory> DKDirectory::OpenDir(const DKString& path)
{
	DKString path2 = L"";

#ifdef _WIN32
	// make system absolute path string (begin with drive path, ie 'C:\', 'D:\')
	if (path.Length() >= 2 && (path[0] == L'\\' || path[0] == L'/') && (path[2] == L':'))
		path2 = path.Right(1).FilePathString();
	else
		path2 = path.FilePathString();
#else
	path2 = path.FilePathString();
#endif

	if (IsDirExist(path2))
	{
		DKObject<DKDirectory>	dir = DKObject<DKDirectory>::New();
		dir->currentPath = path2;
		if (dir->IsValid())
			return dir;
	}
	return NULL;
}

bool DKDirectory::IsDirExist(const DKString& path)
{
	if (path.Length() == 0)
		return false;

#ifdef _WIN32
	if (path == L"/" || path == L"\\")			// root directory.
		return true;
	if (path.Length() >= 2 && path[1] == L':')	// regular path.
	{
		DWORD dwAttr = GetFileAttributesW((const wchar_t*)path);
		if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		{
			return true;
		}
	}
#else
	if (path[0] == L'/')
	{
		DKStringU8 pathUTF8(path);
		if (pathUTF8.Bytes() > 0)
		{
			struct stat	st;
			if (stat((const char*)pathUTF8, &st) != -1)
			{
				return (st.st_mode & S_IFDIR) != 0;
			}
		}
	}
#endif

	return false;
}

const DKString& DKDirectory::AbsolutePath(void) const
{
	return currentPath;
}

bool DKDirectory::operator == (const DKDirectory& dir) const
{
	return currentPath.Length() > 0 && dir.currentPath == currentPath;
}

bool DKDirectory::operator != (const DKDirectory& dir) const
{
	return currentPath.Length() == 0 || dir.currentPath != currentPath;
}

bool DKDirectory::IsFileExist(const DKString& name) const
{
	if (IsValid() && name.Length() > 0)
	{
		DKString filename = L"";
		if (currentPath == L"/" || currentPath == L"\\")
			filename = currentPath + name;
		else
			filename = currentPath + L"/" + name;
		
#ifdef _WIN32
		DWORD dwAttr = GetFileAttributesW((const wchar_t*)filename.FilePathString());
		if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			// file exists.
			return true;
		}
#else
		DKStringU8 filenameUTF8(filename.FilePathString());
		if (filenameUTF8.Bytes() > 0)
		{
			struct stat	st;
			return (stat((const char*)filenameUTF8, &st) != -1 && (st.st_mode & S_IFDIR) == 0);
		}
#endif
		return false;
	}
	return false;
}

bool DKDirectory::IsSubdirExist(const DKString& name) const
{
	if (IsValid() && name.Length() > 0)
	{
		DKString strName = L"";
#ifdef _WIN32
		if (currentPath == L"\\")
		{
			strName = name;
		}
		else
		{
			strName = currentPath + "/" + name;
		}
#else
		if (currentPath == L"/")
		{
			strName = currentPath + name;
		}
		else
		{
			strName = currentPath + "/" + name;
		}
#endif
		return IsDirExist(strName);
	}
	return false;
}

DKString DKDirectory::AbsolutePathOfFile(const DKString& name) const
{
	if (IsFileExist(name))
	{
		return (currentPath + L"/" + name).FilePathString();
	}
	return L"";
}

DKString DKDirectory::AbsolutePathOfSubdir(const DKString& name) const
{
	if (IsSubdirExist(name))
	{
		return (currentPath + L"/" + name).FilePathString();
	}
	return L"";
}

DKObject<DKFile> DKDirectory::OpenFile(const DKString& name, DKFile::ModeOpen mode, DKFile::ModeShare share) const
{
	if (IsFileExist(name))
	{
		return DKFile::Create(currentPath + L"/" + name, mode, share);
	}
	return NULL;
}

DKObject<DKFile> DKDirectory::OpenFile(unsigned int index, DKFile::ModeOpen mode, DKFile::ModeShare share) const
{
	if (index < files.Count())
		return OpenFile(FileNameAtIndex(index), mode, share);
	return NULL;
}

DKObject<DKData> DKDirectory::MapFile(const DKString& name, size_t size, bool writable) const
{
	if (IsFileExist(name))
	{
		return DKFileMap::Open(currentPath + L"/" + name, size, writable).SafeCast<DKData>();
	}
	return NULL;
}

DKObject<DKData> DKDirectory::MapFile(unsigned int index, size_t size, bool writable) const
{
	if (index < files.Count())
		return MapFile(FileNameAtIndex(index), size, writable);
	return NULL;
}

DKObject<DKDirectory> DKDirectory::OpenSubdir(const DKString& name) const
{
	if (IsSubdirExist(name))
	{
		DKString newPath = L"";
		if (currentPath == L"/" || currentPath == L"\\")
			newPath = currentPath + name;
		else
			newPath = currentPath + "/" + name;
		return DKDirectory::OpenDir(newPath);
	}
	return NULL;
}

DKObject<DKDirectory> DKDirectory::OpenSubdirAtIndex(unsigned int index) const
{
	if (index < directories.Count())
		return OpenSubdir(SubdirNameAtIndex(index));
	return NULL;
}

DKObject<DKDirectory> DKDirectory::CreateDir(const DKString& name) const
{
	DKString strNewPath = L"";
	if (name.Find(L'/') != -1 || name.Find(L'\\') != -1)
		return NULL;

#ifdef _WIN32
	if (name.Length() > 2 && name[1] == L':')
	{
		strNewPath = name;
	}
	else
	{
		if (currentPath == L"\\")
			strNewPath = name;
		else
			strNewPath = currentPath + "/" + name;
	}
	// create directory.
	if (strNewPath.Length() > 0 && CreateDirectoryW((const wchar_t*)strNewPath, 0))
	{
		return DKDirectory::OpenDir(strNewPath);
	}
#else
	if (name.Length() > 1 && (name[0] == L'/' || name[0] == L'\\')) 
	{
		strNewPath = name;
	}
	else
	{
		strNewPath = currentPath + "/" + name;
	}
	
	DKStringU8 pathUTF8(strNewPath);
	if (pathUTF8.Bytes() > 0)
	{
		if (mkdir((const char*)pathUTF8, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) == 0)
			return DKDirectory::OpenDir(strNewPath);
	}
#endif	

	return NULL;
}

DKObject<DKDirectory> DKDirectory::OpenParent(void) const
{
	if (IsValid())
	{
		size_t len = currentPath.Length();
		for (int i = len - 1; i > 2; i--)
		{
			if (currentPath[i] == DIRECTORY_PATH_SEPARATOR)
			{
				return OpenDir(currentPath.Left(i));
			}
		}
	}
	return NULL;
}

bool DKDirectory::IsValid(void) const
{
	return (this && this->IsDirExist(currentPath));
}

bool DKDirectory::IsReadable(void) const
{
	if (currentPath.Length() == 0)
		return false;
#ifdef _WIN32
	if (currentPath == L"/" || currentPath == L"\\")
		return true;
	return _waccess(currentPath, 04) != -1;
#else
	DKStringU8 pathUTF8(currentPath);
	if (pathUTF8.Bytes() > 0)
	{
		return access((const char*)pathUTF8, R_OK) != -1;
	}
#endif
	return false;
}

bool DKDirectory::IsWritable(void) const
{
	if (currentPath.Length() == 0)
		return false;
#ifdef _WIN32
	if (currentPath == L"/" || currentPath == L"\\")
		return false;
	return _waccess(currentPath, 02) != -1;
#else
	DKStringU8 pathUTF8(currentPath);
	if (pathUTF8.Bytes() > 0)
	{
		return access((const char*)pathUTF8, W_OK) != -1;
	}
#endif
	return false;
}

bool DKDirectory::Reload(void) const
{
	if (currentPath.Length() == 0)
		return false;
	if (!IsValid())
		return false;

	StringList files;
	StringList directories;
	files.Reserve(512);
	directories.Reserve(512);

#ifdef _WIN32
	if (currentPath == L"\\" || currentPath == L"/")
	{
		// retrieve system drive letters.
		const int buffer_len = 2048;
		wchar_t buffer[buffer_len];
		memset(buffer, 0, buffer_len);
		DWORD dwLen = GetLogicalDriveStringsW(buffer_len-1, buffer);
		if (dwLen > buffer_len-1)
		{
			DKLog("Error: GetLogicalDriveStringsW failed.\n");
			return false;
		}

		wchar_t *pDir = buffer;
		do
		{
			size_t len = wcslen(pDir);
			if (len > 2)
			{
				if (pDir[len-1] == L'\\')
					pDir[len-1] = NULL; // remove last '\', (C:\ -> C:)
				directories.Add(pDir);
			}
			pDir += len + 1;
		} while (*pDir);
	}
	else
	{
		DKString str = currentPath + L"\\*";
		WIN32_FIND_DATAW	findData;
		HANDLE hFind = FindFirstFileW((const wchar_t*)str, &findData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			DKLog("Error: FindFirstFileW failed.\n");
			return false;
		}
		else
		{
			do
			{
				if (wcscmp(findData.cFileName, L".") == 0)
					continue;
				if (wcscmp(findData.cFileName, L"..") == 0)
					continue;
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					directories.Add(findData.cFileName);
				}
				else// if (findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL)
				{
					files.Add(findData.cFileName);
				}
			}
			while (FindNextFileW(hFind, &findData));
		}
	}
#else
	DKStringU8 pathUTF8(currentPath);
	if (pathUTF8.Bytes() > 0)
	{
		DIR* dp = opendir((const char*)pathUTF8);
		if (dp)
		{
			struct dirent* ep = NULL;
			while ((ep = readdir(dp)) != NULL)
			{
				if (strcmp(ep->d_name, ".") == 0)
					continue;
				if (strcmp(ep->d_name, "..") == 0)
					continue;
				if (ep->d_type & DT_DIR)
				{
					directories.Add(ep->d_name);
				}
				else if (ep->d_type & DT_REG)
				{
					files.Add(ep->d_name);
				}
			}
			closedir(dp);
		}
		else
		{
			DKLog("Error: opendir failed.\n");
			return false;
		}
	}
	else
	{
		DKLog("Error: invalid path (%ls).\n", (const wchar_t*)currentPath);
		return false;
	}
#endif

	DKDirectory* dir = const_cast<DKDirectory*>(this);
	dir->reloadRequired = false;
	dir->files = files;
	dir->directories = directories;
	return true;
}

size_t DKDirectory::NumberOfFiles(void) const
{
	this->ReloadIfNeeded();
	return files.Count();
}

size_t DKDirectory::NumberOfSubdirs(void) const
{
	this->ReloadIfNeeded();
	return directories.Count();
}

const DKString& DKDirectory::FileNameAtIndex(unsigned int index) const
{
	this->ReloadIfNeeded();
	if (index < files.Count())
	{
		return files.Value(index);
	}
	return DKString::empty;
}

const DKString& DKDirectory::SubdirNameAtIndex(unsigned int index) const
{
	this->ReloadIfNeeded();
	if (index < directories.Count())
	{
		return directories.Value(index);
	}
	return DKString::empty;
}

bool DKDirectory::DeleteSubDir(const DKString& name) const
{
	if (!IsValid())
		return false;

	DKString path = AbsolutePathOfSubdir(name);
	
	if (IsDirExist(path))
	{		
#ifdef _WIN32
		return RemoveDirectoryW((const wchar_t*)path) != 0;
#else
		DKStringU8 pathUTF8(currentPath);
		if (pathUTF8.Bytes() > 0)
		{
			return rmdir((const char*)pathUTF8) == 0;
		}
#endif
	}
	return false;
}

void DKDirectory::ReloadIfNeeded(void) const
{
	if (this->reloadRequired)
		this->Reload();
}
