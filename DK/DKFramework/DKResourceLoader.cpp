//
//  File: DKResourceLoader.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2014 Hongtae Kim. All rights reserved.
//

#include "DKResource.h"
#include "DKResourceLoader.h"
#include "DKSerializer.h"

// 2011-08-16
// includes resource class headers
// to register default handler.
#include "DKAnimation.h"
#include "DKMaterial.h"
#include "DKTexture.h"
#include "DKTexture2D.h"
#include "DKTexture3D.h"
#include "DKTextureCube.h"
#include "DKVertexBuffer.h"
#include "DKIndexBuffer.h"

#include "DKCollisionShape.h"

#include "DKModel.h"
#include "DKRigidBody.h"
#include "DKSoftBody.h"
#include "DKStaticMesh.h"
#include "DKSkinMesh.h"

// constraints
#include "DKConeTwistConstraint.h"
#include "DKFixedConstraint.h"
#include "DKGeneric6DofConstraint.h"
#include "DKGeneric6DofSpringConstraint.h"
#include "DKHingeConstraint.h"
#include "DKPoint2PointConstraint.h"
#include "DKSliderConstraint.h"

// others
#include "DKRenderTarget.h"


using namespace DKFoundation;
namespace DKFramework
{
	namespace Private
	{
		typedef DKMap<DKString, DKObject<DKResourceLoader::ResourceAllocator>>	AllocatorMap;
		typedef DKMap<DKString, AllocatorMap, DKFoundation::DKDummyLock>		AllocatorURIMap;
		typedef DKMap<DKString, DKObject<DKResourceLoader::ResourceLoader>>		LoaderMap;

		DKSpinLock& GetAllocatorURIMapLock(void)
		{
			static DKSpinLock lock;
			return lock;
		}
		AllocatorURIMap& GetAllocatorURIMap(void)
		{
			static DKAllocator::Maintainer init;

			static AllocatorURIMap map;
			return map;
		}
		DKSpinLock& GetLoaderMapLock(void)
		{
			static DKSpinLock lock;
			return lock;
		}
		LoaderMap& GetLoaderMap(void)
		{
			static DKAllocator::Maintainer init;

			static LoaderMap map;
			return map;
		}
		DKObject<DKResourceLoader::ResourceLoader> GetExtLoader(const DKString& ext)
		{
			DKCriticalSection<DKSpinLock> guard(GetLoaderMapLock());
			LoaderMap::Pair* p = GetLoaderMap().Find(ext);
			if (p)
			{
				return p->value;
			}
			return NULL;
		}
		DKObject<DKResourceLoader::ResourceLoader> FindExtLoader(const DKString& name)
		{
			DKCriticalSection<DKSpinLock> guard(GetLoaderMapLock());
			LoaderMap& map = GetLoaderMap();
			DKObject<DKResourceLoader::ResourceLoader> loader = NULL;
			map.EnumerateForward([&](LoaderMap::Pair& pair, bool* stop)
			{
				if (name.HasSuffix(pair.key))
				{
					*stop = true;
					loader = pair.value;
				}
			});
			return loader;
		}
		DKObject<DKResourceLoader::ResourceAllocator> GetAllocator(const DKString& URI, const DKString& key)
		{
			DKObject<DKResourceLoader::ResourceAllocator> allocator = NULL;
			{
				DKCriticalSection<DKSpinLock> guard(GetAllocatorURIMapLock());
				AllocatorURIMap::Pair* pURI = GetAllocatorURIMap().Find(URI);
				if (pURI)
				{
					AllocatorMap::Pair* pAlloc = pURI->value.Find(key);
					if (pAlloc && pAlloc->value)
						return pAlloc->value;
				}
			}
			return NULL;
		}

		bool InitBuiltinResourceTypes(void)
		{
#define REGISTER_RESOURCE_CLASS(CLASS)		DKResourceLoader::SetResourceAllocator(L"", #CLASS, DKFunction(DKResourceAlloc<CLASS>))

			// make sure to allocator exists before register class-types.
			static DKAllocator::Maintainer init;


			REGISTER_RESOURCE_CLASS(DKAnimation);
			REGISTER_RESOURCE_CLASS(DKMaterial);

			REGISTER_RESOURCE_CLASS(DKTexture);
			REGISTER_RESOURCE_CLASS(DKTexture2D);
			REGISTER_RESOURCE_CLASS(DKTexture3D);
			REGISTER_RESOURCE_CLASS(DKTextureCube);

			REGISTER_RESOURCE_CLASS(DKVertexBuffer);
			REGISTER_RESOURCE_CLASS(DKIndexBuffer);

			// collision shape helper
			REGISTER_RESOURCE_CLASS(DKCollisionShape::SerializeHelper);
			// model object
			REGISTER_RESOURCE_CLASS(DKModel);
			REGISTER_RESOURCE_CLASS(DKStaticMesh);
			REGISTER_RESOURCE_CLASS(DKSkinMesh);
			REGISTER_RESOURCE_CLASS(DKCollisionObject);
			REGISTER_RESOURCE_CLASS(DKRigidBody);
			REGISTER_RESOURCE_CLASS(DKSoftBody);
			// constraints
			REGISTER_RESOURCE_CLASS(DKConeTwistConstraint);
			REGISTER_RESOURCE_CLASS(DKFixedConstraint);
			REGISTER_RESOURCE_CLASS(DKGeneric6DofConstraint);
			REGISTER_RESOURCE_CLASS(DKGeneric6DofSpringConstraint);
			REGISTER_RESOURCE_CLASS(DKHingeConstraint);
			REGISTER_RESOURCE_CLASS(DKPoint2PointConstraint);
			REGISTER_RESOURCE_CLASS(DKSliderConstraint);

#undef REGISTER_RESOURCE_CLASS
			return true;
		}
		static bool init = InitBuiltinResourceTypes();
	}
}
using namespace DKFramework;
using namespace DKFramework::Private;

DKResourceLoader::DKResourceLoader(void)
{
}

DKResourceLoader::~DKResourceLoader(void)
{
}

void DKResourceLoader::SetResourceAllocator(const DKString& URI, const DKString& classId, ResourceAllocator* alloc)
{
	if (classId.Length() > 0)
	{
		DKCriticalSection<DKSpinLock> guard(GetAllocatorURIMapLock());
		if (alloc)
			GetAllocatorURIMap().Value(URI).Update(classId, alloc);
		else
			GetAllocatorURIMap().Value(URI).Remove(classId);
	}
}

void DKResourceLoader::SetResourceFileExtension(const DKString& ext, ResourceLoader* loader)
{
	if (ext.Length() > 0)
	{
		DKCriticalSection<DKSpinLock> guard(GetLoaderMapLock());
		if (loader)
			GetLoaderMap().Update(ext.LowercaseString(), loader);
		else
			GetLoaderMap().Remove(ext.LowercaseString());
	}
}

DKObject<DKResource> DKResourceLoader::ResourceFromXML(const DKXMLElement* e)
{
	DKObject<DKResource> res = NULL;
	if (e)
	{
		DKString URI = e->ns != NULL ? e->ns->URI : L"";
		DKString name = e->name;

		ResourceAllocator* allocator = GetAllocator(URI, name);
		if (allocator)
		{
			DKAllocator& alloc = this->Allocator();
			DKObject<DKResource> obj = allocator->Invoke(alloc);
			if (obj)
			{
				if (obj->allocator == NULL)
					obj->allocator = &alloc;
				if (obj->Deserialize(e, this))
					res = obj;
			}
		}
		else
		{
			// Open with DKSerializer.
			DKAllocator& alloc = this->Allocator();
			DKObject<DKResource> obj = NULL;

			auto selector = [&alloc, &obj](const DKString& name) -> DKObject<DKSerializer>
			{
				ResourceAllocator* allocator = GetAllocator(L"", name);
				if (allocator)
				{
					obj = allocator->Invoke(alloc);
					if (obj)
					{
						if (obj->allocator == NULL)
							obj->allocator = &alloc;
						return obj->Serializer();
					}
				}
				DKLog("DKResourceLoader Warning: DKSerializer Class(%ls) not found!\n", (const wchar_t*)name);
				return NULL;
			};

			if (DKSerializer::RestoreObject(e, this, DKFunction(selector)))
				res = obj;
		}
	}
	return res;
}

DKObject<DKResource> DKResourceLoader::ResourceFromData(const DKData* data, const DKString& name)
{
	DKObject<DKResource> res = NULL;
	if (data)
	{
		DKObject<ResourceLoader> loader = FindExtLoader(name.LowercaseString());
		if (loader)
		{
			const void* p = data->LockShared();
			size_t len = data->Length();
			if (p && len > 0)
			{
				DKObject<DKData> data2 = DKData::StaticData(p, len);
				if (data2)
				{
					DKAllocator& alloc = this->Allocator();
					DKDataStream stream(data2);
					DKObject<DKResource> obj = loader->Invoke(&stream, alloc);
					if (obj)
					{
						if (obj->allocator == NULL)
							obj->allocator = &alloc;
						res = obj;
					}
				}
			}
			data->UnlockShared();
		}
		if (res == NULL)
		{
			// try to open with XML (DKXMLDocument)
			DKObject<DKXMLDocument> xmlDoc = DKXMLDocument::Open(DKXMLDocument::TypeXML, data);
			if (xmlDoc)
			{
				res = this->ResourceFromXML(xmlDoc->RootElement());
			}
			else
			{
				// Open with DKSerializer.
				DKAllocator& alloc = this->Allocator();
				DKObject<DKResource> obj = NULL;

				auto selector = [&alloc, &obj](const DKString& name) -> DKObject<DKSerializer>
				{
					ResourceAllocator* allocator = GetAllocator(L"", name);
					if (allocator)
					{
						obj = allocator->Invoke(alloc);
						if (obj)
						{
							if (obj->allocator == NULL)
								obj->allocator = &alloc;
							return obj->Serializer();
						}
					}
					DKLog("DKResourceLoader Warning: DKSerializer Class(%ls) not found!\n", (const wchar_t*)name);
					return NULL;
				};

				if (DKSerializer::RestoreObject(data, this, DKFunction(selector)))
					res = obj;
			}
		}
	}
	return res;
}

DKObject<DKResource> DKResourceLoader::ResourceFromStream(DKStream* stream, const DKString& name)
{
	if (stream && stream->IsReadable())
	{
		DKObject<ResourceLoader> loader = FindExtLoader(name.LowercaseString());
		if (loader)
		{
			DKAllocator& alloc = this->Allocator();
			DKObject<DKResource> obj = loader->Invoke(stream, alloc);
			if (obj)
			{
				if (obj->allocator == NULL)
					obj->allocator = &alloc;
				return obj;
			}
		}

		DKStream::Position pos = stream->GetPos();
		DKObject<DKXMLDocument> xmlDoc = DKXMLDocument::Open(DKXMLDocument::TypeXML, stream);
		if (xmlDoc && xmlDoc->RootElement())
		{
			return this->ResourceFromXML(xmlDoc->RootElement());
		}
		else
		{
			stream->SetPos(pos);
			// Open with DKSerializer.
			DKAllocator& alloc = this->Allocator();
			DKObject<DKResource> obj = NULL;

			auto selector = [&alloc, &obj](const DKString& name) -> DKObject<DKSerializer>
			{
				ResourceAllocator* allocator = GetAllocator(L"", name);
				if (allocator)
				{
					obj = allocator->Invoke(alloc);
					if (obj)
					{
						if (obj->allocator == NULL)
							obj->allocator = &alloc;
						return obj->Serializer();
					}
				}
				DKLog("DKResourceLoader Warning: DKSerializer Class(%ls) not found!\n", (const wchar_t*)name);
				return NULL;
			};

			if (DKSerializer::RestoreObject(stream, this, DKFunction(selector)))
				return obj;
		}
	}
	return NULL;
}

DKObject<DKResource> DKResourceLoader::ResourceFromFile(const DKString& path, const DKString& name)
{
	if (path.Left(7).CompareNoCase(L"http://") == 0 || path.Left(6).CompareNoCase(L"ftp://") == 0 || path.Left(7).CompareNoCase(L"file://") == 0)
	{
		DKObject<DKBuffer> data = DKBuffer::Create(path);
		if (data)
			return this->ResourceFromData(data.SafeCast<DKData>(), name);
	}
	else
	{
		DKObject<DKFile> file = DKFile::Create(path, DKFile::ModeOpenReadOnly, DKFile::ModeShareAll);
		if (file)
			return this->ResourceFromStream(file.SafeCast<DKStream>(), name);
	}
	return NULL;
}

DKAllocator& DKResourceLoader::Allocator(void) const
{
	return DKAllocator::DefaultAllocator();
}
