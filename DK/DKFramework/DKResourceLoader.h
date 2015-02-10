//
//  File: DKResourceLoader.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"

///////////////////////////////////////////////////////////////////////////////
// DKResourceLoader
// Abstract class, interface for resource loading.
// You have to manage resource by orverride some functions which is pure
// virtual.
//
// There are two ways of how DKRResource can be loaded by this class.
//  1. Override DKResource::Serializer() function for DKResource inherited.
//     You need to create your own DKSerializer object.
//  2. Override DKResource::Serialize(), DKResource::Deserialize().
//     You can override these functions and you can handle data manually.
//     You don't have to use DKSerializer class in this case.
//
// You can load data from any file or URL.
// You need to register by calling SetResourceAllocator for your class.
//
// Note:
//   You can use DKResourcePool for default behavior. (see DKResourcePool.h)
///////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKResource;
	class DKLIB_API DKResourceLoader
	{
	public:
		DKResourceLoader(void);
		virtual ~DKResourceLoader(void);

		DKFoundation::DKObject<DKResource> ResourceFromXML(const DKFoundation::DKXMLElement* element);
		DKFoundation::DKObject<DKResource> ResourceFromData(const DKFoundation::DKData* data, const DKFoundation::DKString& name);
		DKFoundation::DKObject<DKResource> ResourceFromStream(DKFoundation::DKStream* stream, const DKFoundation::DKString& name);
		DKFoundation::DKObject<DKResource> ResourceFromFile(const DKFoundation::DKString& path, const DKFoundation::DKString& name);

		// resource-pool
		virtual void AddResource(const DKFoundation::DKString& name, DKResource* res) = 0;
		virtual DKFoundation::DKObject<DKResource> FindResource(const DKFoundation::DKString& name) const = 0;
		virtual DKFoundation::DKObject<DKResource> LoadResource(const DKFoundation::DKString& name) = 0;
		virtual DKFoundation::DKObject<DKFoundation::DKStream> OpenResourceStream(const DKFoundation::DKString& name) const = 0;

		// resource memory location (Memory-Allocator)
		virtual DKFoundation::DKAllocator& Allocator(void) const;

		// ResourceAllocator : allocate DKResource object. (or subclass of DKResource)
		typedef DKFoundation::DKFunctionSignature<DKFoundation::DKObject<DKResource>(DKFoundation::DKAllocator&)> ResourceAllocator;
		// ResourceLoader : restore object from stream.
		typedef DKFoundation::DKFunctionSignature<DKFoundation::DKObject<DKResource>(DKFoundation::DKStream*, DKFoundation::DKAllocator&)> ResourceLoader;

		static void SetResourceAllocator(const DKFoundation::DKString& URI, const DKFoundation::DKString& classId, ResourceAllocator* allocator);
		static void SetResourceFileExtension(const DKFoundation::DKString& ext, ResourceLoader* loader);
	};

	// DKResourceAlloc is default function of DKResourceLoader::ResourceAllocator.
	// It can be used when object being restored. You can use your own allocator function if you want.
	template <typename T> static DKFoundation::DKObject<DKResource> DKResourceAlloc(DKFoundation::DKAllocator& alloc)
	{
		return DKFoundation::DKObject<T>(new (alloc)T()).template SafeCast<DKResource>();
	}
}
