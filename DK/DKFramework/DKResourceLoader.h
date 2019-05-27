//
//  File: DKResourceLoader.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#ifdef _WIN32
#undef FindResource
#endif

namespace DKFramework
{
	class DKResource;
	/// @brief
	/// interface for resource loading.
	/// @details
	/// You have to manage resource by orverride some functions.
	///
	/// There are two ways of how DKRResource can be loaded by this class.
	///  1. Override DKResource::Serializer() function for DKResource inherited.
	///     You need to create your own DKSerializer object.
	///  2. Override DKResource::Serialize(), DKResource::Deserialize().
	///     You can override these functions and you can handle data manually.
	///     You don't have to use DKSerializer class in this case.
	///
	/// You can load data from any file or URL.
	/// You need to register by calling SetResourceAllocator for your class.
	///
	/// @note
	///   You can use DKResourcePool for default behavior. (see DKResourcePool.h)
	class DKGL_API DKResourceLoader
	{
	public:
		DKResourceLoader();
		virtual ~DKResourceLoader();

		DKObject<DKResource> ResourceFromXML(const DKXmlElement* element);
		DKObject<DKResource> ResourceFromData(const DKData* data, const DKString& name);
		DKObject<DKResource> ResourceFromStream(DKStream* stream, const DKString& name);
		DKObject<DKResource> ResourceFromFile(const DKString& path, const DKString& name);

		/// add resource into resource-pool
		virtual void AddResource(const DKString& name, DKResource* res) = 0;
		virtual DKObject<DKResource> FindResource(const DKString& name) const = 0;
		virtual DKObject<DKResource> LoadResource(const DKString& name) = 0;
		virtual DKObject<DKStream> OpenResourceStream(const DKString& name) const = 0;

		/// resource memory location (Memory-Allocator)
		virtual DKAllocator& Allocator() const;

		/// allocate DKResource object. (or subclass of DKResource)
		typedef DKFunctionSignature<DKObject<DKResource>(DKAllocator&)> ResourceAllocator;
		/// restore object from stream.
		typedef DKFunctionSignature<DKObject<DKResource>(DKStream*, DKAllocator&)> ResourceLoader;

		static void SetResourceAllocator(const DKString& URI, const DKString& classId, ResourceAllocator* allocator);
		static void SetResourceFileExtension(const DKString& ext, ResourceLoader* loader);
	};

	/// DKResourceAlloc is default function of DKResourceLoader::ResourceAllocator.
	/// It can be used when object being restored. You can use your own allocator function if you want.
	template <typename T> static DKObject<DKResource> DKResourceAlloc(DKAllocator& alloc)
	{
		return DKObject<T>(new (alloc)T()).template SafeCast<DKResource>();
	}
}
