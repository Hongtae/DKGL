//
//  File: DKResourcePool.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKResourcePool.h"
#include "DKResource.h"

using namespace DKFramework;

DKResourcePool::DKResourcePool(void)
	: allocator(NULL)
{
}

DKResourcePool::~DKResourcePool(void)
{
}

bool DKResourcePool::AddLocator(Locator* loc, const DKString& name)
{
	if (loc && name.Length() > 0)
	{
		DKCriticalSection<DKSpinLock> guard(this->lock);

		for (NamedLocator& nl : locators)
		{
			if (nl.name == name)
				return nl.locator == loc;
		}

		NamedLocator nl = {name, loc};
		locators.Add(nl);
		return true;
	}
	return false;
}

DKResourcePool::Locator* DKResourcePool::AddLocatorForPath(const DKString& path)
{
	if (path.Length() > 0)
	{
		DKObject<Locator> locator = NULL;
		DKObject<DKDirectory> dir = DKDirectory::OpenDir(path);
		if (dir)		// regular directory
		{
			struct DirLocator : public Locator
			{
				DKObject<DKDirectory> dir;

				DirLocator(DKDirectory* d) : dir(d) {}
				DKString FindSystemPath(const DKString& name) const
				{
					if (dir->IsFileExist(name))
						return dir->AbsolutePathOfFile(name);
					return "";
				}
				DKObject<DKStream> OpenStream(const DKString& name) const
				{
					if (dir->IsFileExist(name))
					{
						DKObject<DKFile> f = dir->OpenFile(name, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll);
						return f.SafeCast<DKStream>();
					}
					return NULL;
				}
			};
			locator = DKOBJECT_NEW DirLocator(dir);
		}
		else	 // zip file with prefix (".../mydata.zip/prefix")
		{
			size_t len = path.Length();
			const wchar_t* str = path;
			for (size_t i = 0; i < len; ++i)
			{
#ifdef _WIN32
				if (str[i] == L'/' || str[i] == L'\\')
#else
				if (str[i] == L'/')
#endif
				{
					DKString file = path.Left(i - 1);
					if (DKDirectory::IsDirExist(file) == false)
					{
						DKObject<DKZipUnarchiver> arc = DKZipUnarchiver::Create(file);
						if (arc)
						{
							struct ZipLocator : public Locator
							{
								DKObject<DKZipUnarchiver> arc;
								DKString prefix;

								ZipLocator(DKZipUnarchiver* z, const DKString& pf) : arc(z), prefix(pf) {}
								DKString FindSystemPath(const DKString&) const {return "";}
								DKObject<DKStream> OpenStream(const DKString& name) const
								{
									return arc->OpenFileStream(prefix + name);
								}
							};
							locator = DKOBJECT_NEW ZipLocator(arc, path.Right(i+1));
						}
						break;
					}
				}
			}
		}
		if (AddLocator(locator, path))
			return locator;
	}
	return NULL;
}

DKResourcePool::Locator* DKResourcePool::FindLocator(const DKString& name)
{
	if (name.Length() > 0)
	{
		DKCriticalSection<DKSpinLock> guard(this->lock);
		for (NamedLocator& loc : locators)
		{
			if (loc.name == name)
				return loc.locator;
		}
	}
	return NULL;
}

void DKResourcePool::RemoveLocator(const DKString& name)
{
	if (name.Length() > 0)
	{
		DKCriticalSection<DKSpinLock> guard(this->lock);
		for (size_t i = 0; i < locators.Count(); ++i)
		{
			if (locators.Value(i).name == name)
			{
				locators.Remove(i);
				return;
			}
		}
	}
}

void DKResourcePool::RemoveAllLocators(void)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	locators.Clear();
}

DKString::StringArray DKResourcePool::AllLocatorNames(void) const
{
	DKString::StringArray names;
	DKCriticalSection<DKSpinLock> guard(this->lock);
	names.Reserve(locators.Count());
	for (const NamedLocator& loc : locators)
	{
		names.Add(loc.name);
	}

	return names;
}

DKString DKResourcePool::ResourceFilePath(const DKString& name) const
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	for (const NamedLocator& loc : locators)
	{
		DKString path = loc.locator->FindSystemPath(name);
		if (path.Length() > 0)
			return path;
	}
	return "";
}

DKObject<DKStream> DKResourcePool::OpenResourceStream(const DKString& name) const
{
	DKObject<DKData> ret = FindResourceData(name); // use previous loaded data
	if (ret)
	{
		DKObject<DKDataStream> stream = DKOBJECT_NEW DKDataStream(ret);
		return stream.SafeCast<DKStream>();
	}

	DKCriticalSection<DKSpinLock> guard(this->lock);
	for (const NamedLocator& loc : locators)
	{
		DKObject<DKStream> s = loc.locator->OpenStream(name);
		if (s)
			return s;
	}
	return NULL;
}

void DKResourcePool::AddResource(const DKString& name, DKResource* res)
{
	if (name.Length() > 0 && res)
	{
		DKCriticalSection<DKSpinLock> guard(this->lock);
		resources.Update(name, res);
	}
}

void DKResourcePool::AddResourceData(const DKString& name, DKData* data)
{
	if (name.Length() > 0 && data)
	{
		DKCriticalSection<DKSpinLock> guard(this->lock);
		resourceData.Update(name, data);
	}
}

void DKResourcePool::RemoveResource(const DKString& name)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	resources.Remove(name);
}

void DKResourcePool::RemoveResourceData(const DKString& name)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	resourceData.Remove(name);
}

void DKResourcePool::RemoveAllResourceData(void)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	resourceData.Clear();
}

void DKResourcePool::RemoveAllResources(void)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	resources.Clear();
}

void DKResourcePool::RemoveAll(void)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	resources.Clear();
	resourceData.Clear();
}

void DKResourcePool::ClearUnreferencedObjects(void)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	using ResInfo = DKMapPair<DKString, DKObject<DKResource>::Ref>;
	using DataInfo = DKMapPair<DKString, DKObject<DKData>::Ref>;

	// collect weak-refs of all objects.
	DKArray<ResInfo> resRefs;
	DKArray<DataInfo> dataRefs;

	resRefs.Reserve(resources.Count());
	dataRefs.Reserve(resourceData.Count());

	resources.EnumerateForward([&resRefs](const ResourceMap::Pair& pair)
	{
		ResInfo info = {pair.key, pair.value};
		resRefs.Add(info);
	});
	resourceData.EnumerateForward([&dataRefs](const DataMap::Pair& pair)
	{
		DataInfo info = {pair.key, pair.value};
		dataRefs.Add(info);
	});

	// clear objects. unreferenced objects will be deleted.
	resources.Clear();
	resourceData.Clear();

	// picking out alive objects only.
	// accessing deleted object with weak-ref will produces NULL.
	for (ResInfo& info : resRefs)
	{
		DKObject<DKResource> r = info.value;
		if (r.Ptr() != NULL)
			resources.Update(info.key, r);
	}
	for (DataInfo& info : dataRefs)
	{
		DKObject<DKData> d = info.value;
		if (d.Ptr() != NULL)
			resourceData.Update(info.key, d);
	}
}

DKObject<DKResource> DKResourcePool::FindResource(const DKString& name) const
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	const ResourceMap::Pair* p = resources.Find(name);
	if (p)
		return p->value;

	return NULL;
}

DKObject<DKData> DKResourcePool::FindResourceData(const DKString& name) const
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	const DataMap::Pair* p = resourceData.Find(name);
	if (p)
		return p->value;

	return NULL;
}

DKObject<DKResource> DKResourcePool::LoadResource(const DKString& name)
{
	DKObject<DKResource> ret = FindResource(name);
	if (ret)
		return ret;

	if (name.Length() > 0)
	{
		if (name.Left(7).CompareNoCase(L"http://") && name.Left(6).CompareNoCase(L"ftp://") && name.Left(7).CompareNoCase(L"file://"))
		{
			// open stream (includes zip-file contents)
			DKObject<DKStream> stream = OpenResourceStream(name);
			if (stream)
				ret = DKResourceLoader::ResourceFromStream(stream, name);
		}
		else
		{
			ret = DKResourceLoader::ResourceFromFile(name, name);
		}

		if (ret == NULL)
		{
			DKString path = ResourceFilePath(name);
			if (path.Length() == 0)
				path = name;

			ret = DKResourceLoader::ResourceFromFile(path, name);
		}
	}

	if (ret)
	{
		if (ret->Validate() == false)
			DKLog("Warning: resource \"%ls\" validation failed.\n", (const wchar_t*)name);

		ret->SetName(name);

		AddResource(name, ret);
		
		DKLog("Resource \"%ls\" loaded.\n", (const wchar_t*)name);
	}
	return ret;
}

DKObject<DKData> DKResourcePool::LoadResourceData(const DKString& name, bool mapFileIfPossible)
{
	DKObject<DKData> ret = FindResourceData(name);
	if (ret)
		return ret;

	if (name.Length() > 0)
	{
		if (name.Left(7).CompareNoCase(L"http://") && name.Left(6).CompareNoCase(L"ftp://") && name.Left(7).CompareNoCase(L"file://"))
		{
			DKString path = ResourceFilePath(name);
			if (path.Length() > 0)
			{
				if (mapFileIfPossible)
					ret = DKFileMap::Open(path, 0, false);
				if (ret == NULL)
					ret = DKBuffer::Create(path);
			}
			else	// file could not be located. (or could be zip-file contents)
			{
				// open stream first (includes zip-file contents)
				DKObject<DKStream> stream = OpenResourceStream(name);
				if (stream)
					ret = DKBuffer::Create(stream);
				if (ret == NULL && mapFileIfPossible)
					ret = DKFileMap::Open(name, 0, false);
				if (ret == NULL)
					ret = DKBuffer::Create(name);
			}
		}
		else
		{
			ret = DKBuffer::Create(name).SafeCast<DKData>();
		}
	}

	if (ret)
	{
		AddResourceData(name, ret);
		
		DKLog("Resource Data:%ls loaded.\n", (const wchar_t*)name);	
	}
	
	return ret;
}

DKObject<DKResourcePool> DKResourcePool::Clone(void) const
{	
	DKObject<DKResourcePool> pool = DKObject<DKResourcePool>::New();

	DKCriticalSection<DKSpinLock> guard(this->lock);
	pool->locators = this->locators;
	pool->allocator = this->allocator;
	pool->resources = this->resources;
	pool->resourceData = this->resourceData;
	
	return pool;
}

void DKResourcePool::SetAllocator(DKMemoryLocation loc)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	this->allocator = &DKAllocator::DefaultAllocator(loc);
}

void DKResourcePool::SetAllocator(DKAllocator* alloc)
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	this->allocator = alloc;
}

DKAllocator& DKResourcePool::Allocator(void) const
{
	DKCriticalSection<DKSpinLock> guard(this->lock);
	
	if (this->allocator)
		return *this->allocator;
	return DKAllocator::DefaultAllocator();
}
