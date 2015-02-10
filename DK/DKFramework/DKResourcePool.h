//
//  File: DKResourcePool.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
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
	class DKLIB_API DKResourcePool : public DKResourceLoader
	{
	public:
		// Locator
		// interface for file or content locating.
		// You need to subclass to use your custom locator.
		struct Locator
		{
			virtual ~Locator(void) {}
			virtual DKFoundation::DKString FindSystemPath(const DKFoundation::DKString&) const = 0;
			virtual DKFoundation::DKObject<DKFoundation::DKStream> OpenStream(const DKFoundation::DKString&) const = 0;
		};

		DKResourcePool(void);
		~DKResourcePool(void);

		bool AddLocator(Locator*, const DKFoundation::DKString& name);
		Locator* AddLocatorForPath(const DKFoundation::DKString& path);
		Locator* FindLocator(const DKFoundation::DKString& name);
		void RemoveLocator(const DKFoundation::DKString& name);
		void RemoveAllLocators(void);
		DKFoundation::DKString::StringArray AllLocatorNames(void) const;

		bool AddSearchPath(const DKFoundation::DKString& path)
		{
			return AddLocatorForPath(path) != NULL;
		}

		// find resource object from pool. (previous loaded)
		DKFoundation::DKObject<DKResource> FindResource(const DKFoundation::DKString& name) const;
		// find resource data from pool. (previous loaded)
		DKFoundation::DKObject<DKFoundation::DKData> FindResourceData(const DKFoundation::DKString& name) const;
		// load resource object. recycles if object loaded already.
		DKFoundation::DKObject<DKResource> LoadResource(const DKFoundation::DKString& name);
		// load resource data. recycles if data loaded already.
		DKFoundation::DKObject<DKFoundation::DKData> LoadResourceData(const DKFoundation::DKString& name, bool mapFileIfPossible = true);

		// insert resource object into pool.
		void AddResource(const DKFoundation::DKString& name, DKResource* res);
		// insert resource data into pool.
		void AddResourceData(const DKFoundation::DKString& name, DKFoundation::DKData* data);
		// remove resource object from pool.
		void RemoveResource(const DKFoundation::DKString& name);
		// remove resource data from pool.
		void RemoveResourceData(const DKFoundation::DKString& name);
		// remove all resource data from pool.
		void RemoveAllResourceData(void);
		// remove all resource objects from pool.
		void RemoveAllResources(void);
		// remove everything in pool.
		void RemoveAll(void);

		// return absolute file path string, if specified file are exists in file-system directory.
		DKFoundation::DKString ResourceFilePath(const DKFoundation::DKString& name) const;
		// open resource as stream.
		DKFoundation::DKObject<DKFoundation::DKStream> OpenResourceStream(const DKFoundation::DKString& name) const;

		DKFoundation::DKObject<DKResourcePool> Clone(void) const;

		void SetAllocator(DKFoundation::DKMemoryLocation loc);
		void SetAllocator(DKFoundation::DKAllocator* alloc);
		DKFoundation::DKAllocator& Allocator(void) const;

	private:
		struct NamedLocator
		{
			DKFoundation::DKString name;
			DKFoundation::DKObject<Locator> locator;
		};
		DKFoundation::DKArray<NamedLocator> locators;

		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKObject<DKResource>>						ResourceMap;
		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKObject<DKFoundation::DKData>>			DataMap;
		ResourceMap			resources;
		DataMap				resourceData;

		DKFoundation::DKSpinLock lock;
		mutable DKFoundation::DKAllocator* allocator;
	};
}
