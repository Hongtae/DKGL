//
//  File: DKResourcePool.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKResource.h"
#include "DKResourceLoader.h"

#ifdef FindResource
#undef FindResource
#endif

////////////////////////////////////////////////////////////////////////////////
// DKResourcePool
// Loads resources which used by DKFramework.
// You set your paths for file located, you can also set path as zip file's
// content by calling AddSearchPath().
// A locator that can find destination file from system directory or specified
// zip file content. You can use your custom locator to locating your data also.
// Subclass DKResourcePool::Locator and call DKResourcePool::AddLocator().
//
// You can open file content without restore object. this can be useful to
// handling raw-data. To load data into memory, call LoadResourceData().
// To open file(or anything locator can locating) call OpenResourceStream().
//
// You can control how object allocated by setting Allocator with
// DKResourcePool::SetAllocator() function. you can use default allocator or
// your own custom allocator.
//
// example:
//  DKResourcePool pool;
//  pool.AddSearchPath("/data/dir");                 // add search path of '/data/dir'
//  pool.AddSearchPath("/data/dir/file.zip");        // add search path of 'file.zip'
//  pool.AddSearchPath("/data/dir/file.zip/prefix"); // add search path of 'file.zip/prefix*'
//
//  pool.LoadResource("MyFile.dat");   // load 'MyFile.data' and restore object.
//  pool.LoadResourceData("MyFile.dat"); // load 'MyFile.data' data only.
//
////////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKGL_API DKResourcePool : public DKResourceLoader
	{
	public:
		// Locator
		// interface for file or content locating.
		// You need to subclass to use your custom locator.
		struct Locator
		{
			virtual ~Locator(void) {}
			virtual DKString FindSystemPath(const DKString&) const = 0;
			virtual DKObject<DKStream> OpenStream(const DKString&) const = 0;
		};

		DKResourcePool(void);
		~DKResourcePool(void);

		bool AddLocator(Locator*, const DKString& name);
		Locator* AddLocatorForPath(const DKString& path);
		Locator* FindLocator(const DKString& name);
		void RemoveLocator(const DKString& name);
		void RemoveAllLocators(void);
		DKString::StringArray AllLocatorNames(void) const;

		bool AddSearchPath(const DKString& path)
		{
			return AddLocatorForPath(path) != NULL;
		}

		// find resource object from pool. (previous loaded)
		DKObject<DKResource> FindResource(const DKString& name) const;
		// find resource data from pool. (previous loaded)
		DKObject<DKData> FindResourceData(const DKString& name) const;
		// load resource object. recycles if object loaded already.
		DKObject<DKResource> LoadResource(const DKString& name);
		// load resource data. recycles if data loaded already.
		DKObject<DKData> LoadResourceData(const DKString& name, bool mapFileIfPossible = true);

		// insert resource object into pool.
		void AddResource(const DKString& name, DKResource* res);
		// insert resource data into pool.
		void AddResourceData(const DKString& name, DKData* data);
		// remove resource object from pool.
		void RemoveResource(const DKString& name);
		// remove resource data from pool.
		void RemoveResourceData(const DKString& name);
		// remove all resource data from pool.
		void RemoveAllResourceData(void);
		// remove all resource objects from pool.
		void RemoveAllResources(void);
		// remove everything in pool.
		void RemoveAll(void);

		// remove unreferenced objects only.
		void ClearUnreferencedObjects(void);

		// return absolute file path string, if specified file are exists in file-system directory.
		DKString ResourceFilePath(const DKString& name) const;
		// open resource as stream.
		DKObject<DKStream> OpenResourceStream(const DKString& name) const;

		DKObject<DKResourcePool> Clone(void) const;

		void SetAllocator(DKMemoryLocation loc);
		void SetAllocator(DKAllocator* alloc);
		DKAllocator& Allocator(void) const;

	private:
		struct NamedLocator
		{
			DKString name;
			DKObject<Locator> locator;
		};
		DKArray<NamedLocator> locators;

		typedef DKMap<DKString, DKObject<DKResource>>						ResourceMap;
		typedef DKMap<DKString, DKObject<DKData>>			DataMap;
		ResourceMap			resources;
		DataMap				resourceData;

		DKSpinLock lock;
		mutable DKAllocator* allocator;
	};
}
