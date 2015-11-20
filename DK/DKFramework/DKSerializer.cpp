//
//  File: DKSerializer.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKSerializer.h"
#include "DKResource.h"
#include "DKResourceLoader.h"

using namespace DKFoundation;
using namespace DKFramework;


// generate operations which can restore data from DKVariant
// 1. picking out operations restorable object from this->entryMap
// 2. picking out faultHandler operations from previous picked out operations.
// 3. fail if entry remains.
class DKSerializer::EntityRestore
{
public:
	typedef DKArray<DKObject<DeserializerEntity>> DeserializerArray;
	struct Entity
	{
		Entity(void) : deserializer(DKObject<DeserializerEntity>::New())
		{
			deserializer->rootValue.SetValueType(DKVariant::TypePairs);
		}
		DKObject<DeserializerEntity> deserializer;
		DKMap<DKString, DeserializerArray> includes;			// includes
	};

	bool ExtractOperations(const DKSerializer* target, Entity& entity, DeserializerArray& output)
	{
		if (target == NULL)
		{
			//DKLog("DKSerializer is NULL.\n");
			return false;
		}

		serializer = target;
		remainsEntities = serializer->entityMap;
		includes.Clear();

		entityError = false;

		if (entity.deserializer->rootValue.ValueType() != DKVariant::TypePairs)
			return false;

		deserializer = entity.deserializer;
		deserializer->operations.Reserve(remainsEntities.Count());

		// included nodes first.
		entity.includes.EnumerateForward([this](DKMap<DKString, DeserializerArray>::Pair& p) {this->EnumerateIncludes(p); });
		deserializer->rootValue.Pairs().EnumerateForward([this](DKVariant::VPairs::Pair& p) {this->EnumerateVariants(p); });
		deserializer->externals.EnumerateForward([this](DKMap<DKString, ExternalType>::Pair& p) {this->EnumerateExternals(p); });
		deserializer->externalArrays.EnumerateForward([this](DKMap<DKString, ExternalArrayType>::Pair& p) {this->EnumerateExternalArrays(p); });
		deserializer->externalMaps.EnumerateForward([this](DKMap<DKString, ExternalMapType>::Pair& p) {this->EnumerateExternalMaps(p); });

		// check remained entities (check for missing nodes)
		remainsEntities.EnumerateForward([this](const EntityMap::Pair& p) {this->EnumerateRemains(p); });

		if (entityError)
			return false;

		deserializer->callback = target->callback;
		output.Reserve(includes.Count() + 1);
		output.Add(includes);
		output.Add(deserializer);

		deserializer = NULL;
		return true;
	}
private:
	void EnumerateVariants(DKVariant::VPairs::Pair& p)
	{
		const EntityMap::Pair* ep = remainsEntities.Find(p.key);
		if (ep)
		{
			const VariantEntity* ve = ep->value->Variant();
			if (ve && ve->setter)
			{
				// validator->invoke() should return true, if validator available.
				if (ve->validator == NULL || ve->validator->Invoke(p.value))
				{
					deserializer->operations.Add(ve->setter->Invocation(p.value).SafeCast<DKOperation>());
					remainsEntities.Remove(p.key);
				}
			}
		}
	}
	void EnumerateIncludes(DKMap<DKString, DeserializerArray>::Pair& p)
	{
		const EntityMap::Pair* ep = remainsEntities.Find(p.key);
		if (ep)
		{
			includes.Add(p.value);
			remainsEntities.Remove(p.key);
		}
	}
	void EnumerateExternals(DKMap<DKString, ExternalType>::Pair& p)
	{
		const EntityMap::Pair* ep = remainsEntities.Find(p.key);
		if (ep)
		{
			const ExternalEntity* ee = ep->value->External();
			if (ee && ee->setter)
			{
				// validator->invoke() should return true, if validator available.
				if (ee->validator == NULL || ee->validator->Invoke(p.value))
				{
					deserializer->operations.Add(ee->setter->Invocation(p.value).SafeCast<DKOperation>());
					remainsEntities.Remove(p.key);
				}
			}
		}
	}
	void EnumerateExternalArrays(DKMap<DKString, ExternalArrayType>::Pair& p)
	{
		const EntityMap::Pair* ep = remainsEntities.Find(p.key);
		if (ep)
		{
			const ExternalEntityArray* ea = ep->value->ExternalArray();
			if (ea && ea->setter)
			{
				// validator->invoke() should return true, if validator available.
				if (ea->validator == NULL || ea->validator->Invoke(p.value))
				{
					deserializer->operations.Add(ea->setter->Invocation(p.value).SafeCast<DKOperation>());
					remainsEntities.Remove(p.key);
				}
			}
		}
	}
	void EnumerateExternalMaps(DKMap<DKString, ExternalMapType>::Pair& p)
	{
		const EntityMap::Pair* ep = remainsEntities.Find(p.key);
		if (ep)
		{
			const ExternalEntityMap* em = ep->value->ExternalMap();
			if (em && em->setter)
			{
				// validator->invoke() should return true, if validator available.
				if (em->validator == NULL || em->validator->Invoke(p.value))
				{
					deserializer->operations.Add(em->setter->Invocation(p.value).SafeCast<DKOperation>());
					remainsEntities.Remove(p.key);
				}
			}
		}
	}
	bool EnumerateRemains(const EntityMap::Pair& p)
	{
		if (p.value->faultHandler)
		{
			deserializer->operations.Add(p.value->faultHandler);
		}
		else
		{
			const VariantEntity* ve = p.value->Variant();
			const SerializerEntity* se = p.value->Serializer();
			const ExternalEntity* ee = p.value->External();
			const ExternalEntityArray* ea = p.value->ExternalArray();
			const ExternalEntityMap* em = p.value->ExternalMap();

			if ((ve && ve->setter) || (se && se->serializer) || (ee && ee->setter) || (ea && ea->setter) || (em && em->setter))
			{
				DKLog("DKSerializer Error: Cannot restore data for key:%ls. (Class:%ls)\n", (const wchar_t*)p.key, (const wchar_t*)serializer->resourceClass);
				entityError = true;
			}
		}
		return !entityError;
	}
private:
	const DKSerializer* serializer;
	DeserializerEntity* deserializer;
	DKArray<DKObject<DeserializerEntity>> includes;
	EntityMap remainsEntities;
	bool entityError;
};

DKSerializer::DKSerializer(void)
{
}

DKSerializer::~DKSerializer(void)
{
	auto dtor = [](EntityMap::Pair& p)
	{
		p.value->~Entity();
		DKMemoryDefaultAllocator::Free(p.value);
	};
	entityMap.EnumerateForward(dtor);
	entityMap.Clear();
}

void DKSerializer::SetCallback(Callback* c)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	callback = c;
}

void DKSerializer::SetResourceClass(const DKString& rc)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	resourceClass = rc;
}

DKString DKSerializer::ResourceClass(void) const
{
	return resourceClass;
}

bool DKSerializer::Bind(const DKString& key, DKSerializer* s, FaultHandler* faultHandler)
{
	if (s == NULL)
		return false;

	SerializerEntity* se = new (DKMemoryDefaultAllocator::Alloc(sizeof(SerializerEntity))) SerializerEntity;
	se->serializer = s;
	se->faultHandler = faultHandler;

	DKCriticalSection<DKSpinLock> guard(lock);
	return entityMap.Insert(key, se);
}

bool DKSerializer::Bind(const DKString& key, ValueGetter* getter, ValueSetter* setter, ValueValidator* validator, FaultHandler* faultHandler)
{
	VariantEntity *ve = new (DKMemoryDefaultAllocator::Alloc(sizeof(VariantEntity))) VariantEntity;
	ve->getter = getter;
	ve->setter = setter;
	ve->validator = validator;
	ve->faultHandler = faultHandler;

	DKCriticalSection<DKSpinLock> guard(lock);
	return entityMap.Insert(key, ve);
}

bool DKSerializer::Bind(const DKString& key, ExternalGetter* getter, ExternalSetter* setter, ExternalValidator* validator, ExternalResource ext, FaultHandler* faultHandler)
{
	ExternalEntity* ee = new (DKMemoryDefaultAllocator::Alloc(sizeof(ExternalEntity))) ExternalEntity;
	ee->getter = getter;
	ee->setter = setter;
	ee->validator = validator;
	ee->faultHandler = faultHandler;
	ee->external = ext;

	DKCriticalSection<DKSpinLock> guard(lock);
	return entityMap.Insert(key, ee);
}

bool DKSerializer::Bind(const DKString& key, ExternalArrayGetter* getter, ExternalArraySetter* setter, ExternalArrayValidator* validator, ExternalResource ext, FaultHandler* faultHandler)
{
	ExternalEntityArray* ea = new (DKMemoryDefaultAllocator::Alloc(sizeof(ExternalEntityArray))) ExternalEntityArray;
	ea->getter = getter;
	ea->setter = setter;
	ea->validator = validator;
	ea->faultHandler = faultHandler;
	ea->external = ext;

	DKCriticalSection<DKSpinLock> guard(lock);
	return entityMap.Insert(key, ea);
}

bool DKSerializer::Bind(const DKString& key, ExternalMapGetter* getter, ExternalMapSetter* setter, ExternalMapValidator* validator, ExternalResource ext, FaultHandler* faultHandler)
{
	ExternalEntityMap* em = new (DKMemoryDefaultAllocator::Alloc(sizeof(ExternalEntityMap))) ExternalEntityMap;
	em->getter = getter;
	em->setter = setter;
	em->validator = validator;
	em->faultHandler = faultHandler;
	em->external = ext;

	DKCriticalSection<DKSpinLock> guard(lock);
	return entityMap.Insert(key, em);
}

void DKSerializer::Unbind(const DKString& key)
{
	DKCriticalSection<DKSpinLock> guard(lock);
	EntityMap::Pair* p = entityMap.Find(key);
	if (p)
	{
		p->value->~Entity();
		DKMemoryDefaultAllocator::Free(p->value);
		entityMap.Remove(key);
	}
}

DKObject<DKXMLElement> DKSerializer::SerializeXML(SerializeForm sf) const
{
	auto EntityFailCheck = [](const EntityMap& map, const DKString& key) -> bool
	{
		const EntityMap::Pair* p = map.Find(key);
		if (p && p->value && p->value->faultHandler)
			return true;
		return false;
	};

	struct SerializerElementWithKey
	{
		DKString key;
		DKObject<DKSerializer> serializer;
		DKObject<DKXMLElement> Serialize(SerializeForm sf) const
		{
			DKObject<DKXMLElement> e = this->serializer->SerializeXML(sf);
			if (e)
			{
				DKObject<DKXMLElement> incNode = DKObject<DKXMLElement>::New();
				incNode->name = L"Include";
				DKXMLAttribute attrKey = {0, L"key", this->key};
				incNode->attributes.Add(attrKey);
				incNode->nodes.Add(e.SafeCast<DKXMLNode>());
				return incNode;
			}
			return NULL;
		}
	};
	struct ExternalEntityInfo
	{
		DKObject<DKResource> res;
		DKString referenceFile;
		bool reference;
		DKObject<DKXMLElement> Serialize(SerializeForm sf) const
		{
			if (this->reference)  // add reference name. (usually, a filename)
			{
				if (this->referenceFile.Length() > 0)
				{
					DKObject<DKXMLElement> extNode = DKObject<DKXMLElement>::New();
					extNode->name = L"External";
					DKXMLAttribute attrFile = {0, L"file", this->referenceFile};
					extNode->attributes.Add(attrFile);
					return extNode;
				}
			}
			else        // embed reference
			{
				// check reference object whether it can be serialized with XML. (XMLElement)
				DKObject<DKXMLElement> node = this->res->SerializeXML(sf);
				if (node) 
				{ 
					DKObject<DKXMLElement> extNode = DKObject<DKXMLElement>::New();
					extNode->name = L"External"; 
					extNode->nodes.Add(node.SafeCast<DKXMLNode>());
					return extNode; 
				}
				else    // embed as data (encode to be XMLCData)
				{
					DKObject<DKData> data = this->res->Serialize(sf);
					if (data)
					{
						DKObject<DKBuffer> buff = data.SafeCast<DKBuffer>();
						if (buff == NULL)
						{
							buff = DKBuffer::Create(data);
							data = NULL;
						}
						DKObject<DKBuffer> compressed = buff->Compress(DKCompressor::Deflate);
						if (compressed)
						{
							DKObject<DKXMLCData> cdata = DKObject<DKXMLCData>::New();
							if (compressed->Base64Encode(cdata->value))
							{
								DKObject<DKXMLElement> extNode = DKObject<DKXMLElement>::New();
								extNode->name = L"External";
								extNode->nodes.Add(cdata.SafeCast<DKXMLNode>());
								return extNode;
							}
						}
					}
				}
			}
			return NULL;
		}
		DKObject<DKXMLElement> Serialize(const DKString& key, SerializeForm sf) const
		{
			DKObject<DKXMLElement> e = Serialize(sf);
			if (e)
			{
				DKXMLAttribute attrKey = {0, L"key", key};
				e->attributes.Add(attrKey);
			}
			return e;
		}
	};
	struct ExternalEntityInfoWithKey
	{
		DKString key;
		ExternalEntityInfo info;
	};
	struct ExternalEntityInfoArrayWithKey
	{
		DKString key;
		DKArray<ExternalEntityInfo> infoArray;
	};
	struct ExternalEntityInfoMapWithKey
	{
		DKString key;
		DKMap<DKString, ExternalEntityInfo> infoMap;
	};

	struct QueryEntities
	{
		bool operator () (const EntityMap& map)
		{
			variant.SetValueType(DKVariant::TypePairs);
			variant.Pairs().Clear();
			externals.Clear();
			serializers.Clear();
			externals.Reserve(map.Count());
			serializers.Reserve(map.Count());
			entityError = false;

			map.EnumerateForward( [this](const EntityMap::Pair& pair) {this->Enumerate(pair);} );

			return !entityError;
		}
		DKVariant variant;
		DKArray<SerializerElementWithKey> serializers;
		DKArray<ExternalEntityInfoWithKey> externals;
		DKArray<ExternalEntityInfoArrayWithKey> externalArrays;
		DKArray<ExternalEntityInfoMapWithKey> externalMaps;
		bool entityError;
	private:
		bool Enumerate(const EntityMap::Pair& p)
		{
			bool failed = false;
			const VariantEntity* ve = p.value->Variant();
			const SerializerEntity* se = p.value->Serializer();
			const ExternalEntity* ee = p.value->External();
			const ExternalEntityArray* ea = p.value->ExternalArray();
			const ExternalEntityMap* em = p.value->ExternalMap();
			if (ve && ve->setter)
			{
				DKVariant v(DKVariant::TypeUndefined);
				ve->getter->Invoke(v);
				if (v.ValueType() != DKVariant::TypeUndefined)
					variant.Pairs().Update(p.key, v);
				else
					failed = true;
			}
			if (se && se->serializer)
			{
				SerializerElementWithKey sek = {p.key, se->serializer};
				serializers.Add(sek);
			}
			if (ee && ee->getter)
			{
				DKObject<DKResource> res = NULL;
				ee->getter->Invoke(res);
				if (res)
				{
					DKString resourceName = res->Name();
					ExternalEntityInfoWithKey ek = {p.key, {res, resourceName, false}};
					if ((ee->external == ExternalResourceReferenceIfPossible && resourceName.Length() > 0) || (ee->external == ExternalResourceForceReference))
						ek.info.reference = true;		// set reference to external resource.

					externals.Add(ek);
				}
				else
					failed = true;
			}
			if (ea && ea->getter)
			{
				ExternalArrayType eat;
				ea->getter->Invoke(eat);
				ExternalEntityInfoArrayWithKey eeak;
				eeak.key = p.key;
				eeak.infoArray.Reserve(eat.Count());
				for (DKResource* res : eat)
				{
					if (res)
					{
						DKString resourceName = res->Name();
						ExternalEntityInfo info = {res, resourceName, false};
						if ((ea->external == ExternalResourceReferenceIfPossible && resourceName.Length() > 0) || (ea->external == ExternalResourceForceReference))
							info.reference = true;

						eeak.infoArray.Add(info);
					}
				}
				externalArrays.Add(eeak);
			}
			if (em && em->getter)
			{
				ExternalMapType emt;
				em->getter->Invoke(emt);
				ExternalEntityInfoMapWithKey eemk;
				eemk.key = p.key;

				emt.EnumerateForward( [&eemk, em](ExternalMapType::Pair& pair)
				{
					DKResource* res = pair.value;
					if (res)
					{
						DKString resourceName = res->Name();
						ExternalEntityInfo info = {res, resourceName, false};
						if ((em->external == ExternalResourceReferenceIfPossible && resourceName.Length() > 0) || (em->external == ExternalResourceForceReference))
							info.reference = true;

						eemk.infoMap.Insert(pair.key, info);
					}
				});
				externalMaps.Add(eemk);
			}

			if (failed && p.value->faultHandler == NULL)
			{
				DKLog("DKSerializer Error: entity(%ls) invalid.\n", (const wchar_t*)p.key);
				entityError = true;
			}
			return !entityError;
		}
	} queryEntities;

	DKObject<DKXMLElement> result = NULL;

	DKCriticalSection<DKSpinLock> guard(this->lock);

	if (this->callback)
		this->callback->Invoke(StateSerializeBegin);

	if (this->resourceClass.Length() > 0)
	{
		if (queryEntities(this->entityMap))
		{
			result = DKObject<DKXMLElement>::New();
			result->name = L"DKSerializer";
			DKXMLAttribute attrCID = {NULL, L"class", this->resourceClass};
			result->attributes.Add(attrCID);

			// add local data (target object owned)
			if (result)
			{
				if (sf == SerializeFormXML)
				{
					DKObject<DKXMLElement> node = queryEntities.variant.ExportXML();
					if (node)
					{
						DKObject<DKXMLElement> variantNode = DKObject<DKXMLElement>::New();
						variantNode->name = L"Local";
						DKXMLAttribute attrType = {0, L"type", L"xml"};
						variantNode->attributes.Add(attrType);
						variantNode->nodes.Add(node.SafeCast<DKXMLNode>());
						result->nodes.Add(variantNode.SafeCast<DKXMLNode>());
					}
					else
					{
						DKLog("Error: Failed to generate DKVariant XML data.\n");
						result = NULL;
					}
				}
				else if (sf == SerializeFormBinXML)
				{
					DKBufferStream stream;
					DKObject<DKXMLElement> variantNode = NULL;

					if (queryEntities.variant.ExportStream(&stream))
					{
						const DKBuffer* buffer = stream.BufferObject();
						if (buffer && buffer->Length() > 0)
						{
							DKObject<DKBuffer> compressed = buffer->Compress(DKCompressor::Deflate);
							if (compressed)
							{
								DKObject<DKXMLCData> cdata = DKObject<DKXMLCData>::New();
								if (compressed->Base64Encode(cdata->value))
								{
									variantNode = DKObject<DKXMLElement>::New();
									variantNode->name = L"Local";
									DKXMLAttribute attrType = {0, L"type", L"binary"};
									variantNode->attributes.Add(attrType);
									variantNode->nodes.Add(cdata.SafeCast<DKXMLNode>());
								}
							}
						}
					}					
					if (variantNode)
					{
						result->nodes.Add(variantNode.SafeCast<DKXMLNode>());								
					}
					else
					{
						DKLog("Error: Failed to generate DKVariant binary data.\n");
						result = NULL;
					}
				}
				else 
				{
					DKLog("Error: Unknown serialization format!\n");
					result = NULL;
				}
			}

			// include
			if (result)
			{
				for (size_t i = 0; i < queryEntities.serializers.Count(); ++i)
				{
					SerializerElementWithKey& sek = queryEntities.serializers.Value(i);
					DKObject<DKXMLElement> e = sek.Serialize(sf);
					if (e)
					{
						result->nodes.Add(e.SafeCast<DKXMLNode>());
					}
					else if (EntityFailCheck(this->entityMap, sek.key) == false)
					{
						DKLog("DKSerializer Error: Key(%ls) is not serializable!\n", (const wchar_t*)sek.key);
						result = NULL;
						break;
					}
				}
			}

			// picking out externals
			if (result)
			{
				for (size_t i = 0; i < queryEntities.externals.Count(); ++i)
				{
					ExternalEntityInfoWithKey& ek = queryEntities.externals.Value(i);

					DKObject<DKXMLElement> node = ek.info.Serialize(ek.key, sf);
					if (node)
					{
						result->nodes.Add(node.SafeCast<DKXMLNode>());
					}
					else if (EntityFailCheck(this->entityMap, ek.key) == false)
					{
						DKLog("DKSerializer Error: Key(%ls) is not serializable!\n", (const wchar_t*)ek.key);
						result = NULL;
						break;
					}
				}
			}
			// external array
			if (result)
			{
				for (size_t i = 0; i < queryEntities.externalArrays.Count(); ++i)
				{
					ExternalEntityInfoArrayWithKey& eak = queryEntities.externalArrays.Value(i);
					DKObject<DKXMLElement> extNode = DKObject<DKXMLElement>::New();
					extNode->name = L"ExternalArray";
					DKXMLAttribute attrKey = {0, L"key", eak.key};
					extNode->attributes.Add(attrKey);

					bool entityError = false;
					for (size_t j = 0; j < eak.infoArray.Count(); ++j)
					{
						ExternalEntityInfo& info = eak.infoArray.Value(j);

						DKObject<DKXMLElement> node = info.Serialize(sf);
						if (node)
						{
							extNode->nodes.Add(node.SafeCast<DKXMLNode>());
						}
						else
						{
							entityError = true;
							break;
						}
					}
					if (entityError)
					{
						if (EntityFailCheck(this->entityMap, eak.key) == false)
						{
							DKLog("DKSerializer Error: Key(%ls) is not serializable!\n", (const wchar_t*)eak.key);
							result = NULL;
							break;
						}
					}
					else
					{
						result->nodes.Add(extNode.SafeCast<DKXMLNode>());
					}
				}
			}
			// external map
			if (result)
			{
				for (size_t i = 0; i < queryEntities.externalMaps.Count(); ++i)
				{
					ExternalEntityInfoMapWithKey& emk = queryEntities.externalMaps.Value(i);
					DKObject<DKXMLElement> extNode = DKObject<DKXMLElement>::New();
					extNode->name = L"ExternalMap";
					DKXMLAttribute attrKey = {0, L"key", emk.key};
					extNode->attributes.Add(attrKey);

					bool entityError = false;
					emk.infoMap.EnumerateForward([&](DKMap<DKString, ExternalEntityInfo>::Pair& pair, bool* stop)
					{
						ExternalEntityInfo& info = pair.value;

						DKObject<DKXMLElement> node = info.Serialize(pair.key, sf);
						if (node != NULL)
						{
							extNode->nodes.Add(node.SafeCast<DKXMLNode>());
						}
						else
						{
							entityError = true;
							*stop = true;
						}
					});
					if (entityError)
					{
						if (EntityFailCheck(this->entityMap, emk.key) == false)
						{
							DKLog("DKSerializer Error: Key(%ls) is not serializable!\n", (const wchar_t*)emk.key);
							result = NULL;
							break;
						}
					}
					else
					{
						result->nodes.Add(extNode.SafeCast<DKXMLNode>());
					}
				}
			}
		}
		else	// queryEntity failed
		{
			DKLog("DKSerializer::Serialize failed. (entity error)\n");
		}
	}
	else // resourceClass name error
	{
		DKLog("DKSerializer::Serialize failed. (Class-Id invalid)\n");
	}

	if (this->callback)
	{
		if (result)
			this->callback->Invoke(StateSerializeSucceed);
		else
			this->callback->Invoke(StateSerializeFailed);
	}
	return result;
}

bool DKSerializer::DeserializeXMLOperations(const DKXMLElement* e, DKArray<DKObject<DeserializerEntity>>& entities, DKResourceLoader* loader) const
{
	if (e == NULL || e->name.CompareNoCase(L"DKSerializer") != 0)
		return false;

	EntityRestore::Entity restoreEntities;

	struct ExternalResource
	{
		DKString key;
		DKObject<DKResource> res;
		bool LoadFromElement(const DKXMLElement* e, DKResourceLoader* loader)
		{
			key = L"";
			res = NULL;

			if (e->name.CompareNoCase(L"External") == 0)
			{
				DKString objectKey = L"";
				DKString externalFile = L"";
				for (size_t i = 0; i < e->attributes.Count(); ++i)
				{
					if (e->attributes.Value(i).name.CompareNoCase(L"key") == 0)
						objectKey = e->attributes.Value(i).value;
					else if (e->attributes.Value(i).name.CompareNoCase(L"file") == 0)
						externalFile = e->attributes.Value(i).value;
				}
				if (externalFile.Length() > 0)
				{
					if (loader)
						res = loader->LoadResource(externalFile);
				}
				else if (loader)
				{
					for (size_t i = 0; i < e->nodes.Count(); ++i)
					{
						if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
						{
							res = loader->ResourceFromXML(e->nodes.Value(i).SafeCast<DKXMLElement>());
							break;
						}
						else if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeCData)
						{
							const DKXMLCData* cdata = e->nodes.Value(i).SafeCast<DKXMLCData>();
							DKASSERT_DEBUG(cdata != NULL);

							DKObject<DKBuffer> compressed = DKBuffer::Base64Decode(cdata->value);
							if (compressed)
							{
								DKObject<DKBuffer> data = compressed->Decompress();
								if (data)
								{
									res = loader->ResourceFromData(data, L"");
									break;
								}

							}
						}
					}
				}
				if (res)
				{
					key = objectKey;
					return true;
				}
			}
			return false;
		}
	};

	DKCriticalSection<DKSpinLock> guard(lock);

	// class-id verification
	DKString classId = L"";
	for (size_t i = 0; i < e->attributes.Count(); ++i)
	{
		if (e->attributes.Value(i).name.CompareNoCase(L"class") == 0)
		{
			classId = e->attributes.Value(i).value;
			break;
		}
	}

	if (this->resourceClass.Compare(classId))
	{
		DKLog("DKSerializer::Deserialize failed: ClassId mismatch. (%ls != %ls)\n", (const wchar_t*)this->resourceClass, (const wchar_t*)classId);
		return false;
	}

	// parse XML and extract data.
	for (size_t i = 0; i < e->nodes.Count(); ++i)
	{
		if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
		{
			const DKXMLElement* node1 = e->nodes.Value(i).SafeCast<DKXMLElement>();
			if (node1->name.CompareNoCase(L"Local") == 0)
			{
				DKString type = L"";
				for (size_t j = 0; j < node1->attributes.Count(); ++j)
				{
					if (node1->attributes.Value(j).name.CompareNoCase(L"type") == 0)
					{
						type = node1->attributes.Value(j).value;
					}
				}
				// variant import
				if (type.CompareNoCase(L"xml") == 0)
				{
					for (size_t j = 0; j < node1->nodes.Count(); ++j)
					{
						if (node1->nodes.Value(j)->Type() == DKXMLNode::NodeTypeElement)
						{
							if (restoreEntities.deserializer->rootValue.ImportXML(node1->nodes.Value(j).SafeCast<DKXMLElement>()))
								break;
						}
					}
				}
				else if (type.CompareNoCase(L"binary") == 0)
				{
					for (size_t j = 0; j < node1->nodes.Count(); ++j)
					{
						if (node1->nodes.Value(j)->Type() == DKXMLNode::NodeTypeCData)
						{
							DKObject<DKBuffer> compressed = DKBuffer::Base64Decode(node1->nodes.Value(j).SafeCast<DKXMLCData>()->value);
							if (compressed)
							{
								DKObject<DKBuffer> d = compressed->Decompress();
								if (d)
								{
									DKDataStream stream(d);
									if (restoreEntities.deserializer->rootValue.ImportStream(&stream))
										break;
								}
							}
						}
					}
				}
			}
			else if (node1->name.CompareNoCase(L"Include") == 0)
			{
				// include node
				DKString objectKey = L"";
				const DKXMLElement* incNode = NULL;
				for (size_t j = 0; j < node1->attributes.Count(); ++j)
				{
					if (node1->attributes.Value(j).name.CompareNoCase(L"key") == 0)
						objectKey = node1->attributes.Value(j).value;
				}
				for (size_t j = 0; j < node1->nodes.Count(); ++j)
				{
					if (node1->nodes.Value(j)->Type() == DKXMLNode::NodeTypeElement)
					{
						incNode = node1->nodes.Value(j).SafeCast<DKXMLElement>();
						break;
					}
				}
				if (incNode)
				{
					const EntityMap::Pair* ep = this->entityMap.Find(objectKey);
					if (ep)
					{
						const SerializerEntity* se = ep->value->Serializer();
						if (se && se->serializer)
						{
							EntityRestore::DeserializerArray de;
							if (se->serializer->DeserializeXMLOperations(incNode, de, loader))
							{
								restoreEntities.includes.Insert(objectKey, de);
							}
						}
					}
				}
			}
			else if (node1->name.CompareNoCase(L"External") == 0)
			{
				// load external resource, add to restoreEntities.externals.
				ExternalResource ex;
				if (ex.LoadFromElement(node1, loader))
				{
					if (ex.key.Length() > 0 && ex.res != NULL)
						restoreEntities.deserializer->externals.Insert(ex.key, ex.res);
				}
			}
			else if (node1->name.CompareNoCase(L"ExternalArray") == 0)
			{
				DKString objectKey = L"";
				for (size_t j = 0; j < node1->attributes.Count(); ++j)
				{
					if (node1->attributes.Value(j).name.CompareNoCase(L"key") == 0)
						objectKey = node1->attributes.Value(j).value;
				}
				if (objectKey.Length() > 0)
				{
					bool loadError = false;
					ExternalArrayType eat;
					for (size_t j = 0; j < node1->nodes.Count(); ++j)
					{
						if (node1->nodes.Value(j)->Type() == DKXMLNode::NodeTypeElement)
						{
							const DKXMLElement* node2 = node1->nodes.Value(j).SafeCast<DKXMLElement>();
							if (node2->name.CompareNoCase(L"External") == 0)
							{
								ExternalResource ex;
								if (ex.LoadFromElement(node2, loader) && ex.res != NULL)
								{
									eat.Add(ex.res);
								}
								else
								{
									loadError = true;
									break;
								}
							}
						}
					}
					if (loadError == false)
						restoreEntities.deserializer->externalArrays.Insert(objectKey, eat);
				}
			}
			else if (node1->name.CompareNoCase(L"ExternalMap") == 0)
			{
				DKString objectKey = L"";
				for (size_t j = 0; j < node1->attributes.Count(); ++j)
				{
					if (node1->attributes.Value(j).name.CompareNoCase(L"key") == 0)
						objectKey = node1->attributes.Value(j).value;
				}
				if (objectKey.Length() > 0)
				{
					bool loadError = false;
					ExternalMapType emt;
					for (size_t j = 0; j < node1->nodes.Count(); ++j)
					{
						if (node1->nodes.Value(j)->Type() == DKXMLNode::NodeTypeElement)
						{
							const DKXMLElement* node2 = node1->nodes.Value(j).SafeCast<DKXMLElement>();
							if (node2->name.CompareNoCase(L"External") == 0)
							{
								ExternalResource ex;
								if (ex.LoadFromElement(node2, loader) && ex.key.Length() > 0 && ex.res != NULL)
								{
									emt.Insert(ex.key, ex.res);
								}
								else
								{
									loadError = true;
									break;
								}
							}
						}
					}
					if (loadError == false)
						restoreEntities.deserializer->externalMaps.Insert(objectKey, emt);
				}
			}
		}
	}

	// generate operations (DKOperation)
	return EntityRestore().ExtractOperations(this, restoreEntities, entities);
}

bool DKSerializer::Deserialize(const DKXMLElement* e, DKResourceLoader* loader) const
{
	// To determine, object can be restored, generate restore operation in advance.
	// embedded serializer should be restorable also.

	DKArray<DKObject<DeserializerEntity>> deserializers;

	if (DeserializeXMLOperations(e, deserializers, loader))
	{
		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			if (deserializers.Value(i)->callback)
				deserializers.Value(i)->callback->Invoke(StateDeserializeBegin);
		}

		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			DeserializerEntity* de = deserializers.Value(i);
			for (size_t k = 0; k < de->operations.Count(); ++k)
				de->operations.Value(k)->Perform();

			// clear value (finished)
			de->operations.Clear();
			de->rootValue.SetValueType(DKVariant::TypeUndefined);
		}

		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			if (deserializers.Value(i)->callback)
				deserializers.Value(i)->callback->Invoke(StateDeserializeSucceed);
		}
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// DKSerializer binary format layout
//
//  HEADER_STRING(fixed)
//		HEADER_STRING = "DKSerialize" + byte-order-char (B/L)
//		(big-endian = "DKSerializeB")
//		(little-endian = "DKSerializeL")
//
// Version(uint16)
// resourceClassLength(uint64)
// resourceClass(variable length)
// num-chunks(uint64)
//
// chunk structure
//		header(uint32) = 'chdr'
//		type(uint32)
//			'vars'	VariantEntity
//			'varc'	compressed VariantEntity
//			'sers'	SerializerEntity
//			'serc'	compressed SerializerEntity
//			'extn'	ExternalResource name (filename)
//			'exts'	regular ExternalResource data
//			'extc'	compressed ExternalResource data
//		cType(uint32)
//			CRC32	hash for vars, varc (uncompressed data CRC32)
//			0		SerializerEntitey, values of ExternalResource(single)
//			'exta'	ExternalEntityArray
//			'extm'	ExternalEntityMap
//		objectKeyLength(uint64)
//		objectKey(variable-length), key for entity (utf-8)
//		containerKeyLength(uint64)
//		containerKey(variable-length), key for container (utf-8)
//		unpackSize(uint64) length of data (uncompressed length)
//		size(uint64) length of data
//		data(variable-length)
//
// In case of types: 'vars','varc','sers','serc',
// should be containerType = 'none', containerKeyLen = 0.
// CRC checking is valid for 'vars', 'varc' only.
//
// if container is array, objectKey='', objectKeyLen = 0.
//
////////////////////////////////////////////////////////////////////////////////

#define DKSERIALIZER_VERSION	12
#define DKSERIALIZER_HEADER_STRING_BIG_ENDIAN		"DKSerializeB"
#define DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN	"DKSerializeL"


#if __LITTLE_ENDIAN__
#define DKSERIALIZER_HEADER_STRING		DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN
#else
#define DKSERIALIZER_HEADER_STRING		DKSERIALIZER_HEADER_STRING_BIG_ENDIAN
#endif

size_t DKSerializer::SerializeBinary(SerializeForm sf, DKStream* output) const
{
	if (sf != SerializeFormBinary && sf != SerializeFormCompressedBinary)
		return 0;
	if (output == NULL || output->IsWritable() == false)
		return 0;

	// SerializeFormBinary : regular binary format. (fastest)
	// SerializeFormCompressedBinary : compressed binary. (smallest)
	//
	// root node of serializer can compress data. (root-node only)
	// sub nodes do not compress. (double-compression worse)

	struct EntityChunk
	{
		DKStringU8			key;
		DKStringU8			containerKey;
		unsigned int		type;
		unsigned int		cType;				// CRC or container type
		DKObject<DKData>	data;
		unsigned long long 	unpackedSize;		// original length
		bool Create(const DKString& key, const DKString& ckey, unsigned int type, unsigned int ctype, DKData* rawData, bool compress)
		{
			if (rawData)
			{
				if (ctype == 'hash' || compress)
				{
					const void* ptr = rawData->LockShared();
					size_t length = rawData->Length();

					if (ctype == 'hash')
						ctype = DKHashCRC32(ptr, length).digest[0];

					DKObject<DKData> compressedData = NULL;
					if (compress)
						compressedData = DKBuffer::Compress(ptr, length, DKCompressor::Deflate).SafeCast<DKData>();

					rawData->UnlockShared();

					if (compress)
						this->data = compressedData;
					else
						this->data = rawData;

					this->unpackedSize = length;
				}
				else
				{
					this->data = rawData;
					this->unpackedSize = rawData->Length();
				}

				if (this->data)
				{
					this->key.SetValue(key);
					this->containerKey.SetValue(ckey);
					this->type = type;
					this->cType = ctype;
					return true;
				}
			}
			return false;
		}
		bool CreateExternal(const DKString& key, const DKString& ckey, unsigned int ctype, DKResource* res, ExternalResource ext, bool compress)
		{
			if (res)
			{
				DKString resourceName = res->Name();
				if ((ext == ExternalResourceReferenceIfPossible && resourceName.Length() > 0) || (ext == ExternalResourceForceReference))
				{
					// includes filename
					DKStringU8 filename(resourceName);
					size_t len = filename.Bytes();
					if (len > 0)
					{
						return Create(key, ckey, 'extn', ctype, DKBuffer::Create((const char*)filename, len), false);
					}
					return false;
				}
				else
				{
					// includes content
					return Create(key, ckey, compress? 'extc' : 'exts', ctype, res->Serialize(SerializeFormBinary), compress);
				}
			}
			return false;
		}
	};

	struct QueryEntities
	{
		bool operator () (const EntityMap& map, bool comp)
		{
			variant.SetValueType(DKVariant::TypePairs);
			variant.Pairs().Clear();
			chunks.Clear();
			chunks.Reserve(map.Count());
			entityError = false;
			compress = comp;

			map.EnumerateForward( [this](const EntityMap::Pair& pair) {this->Enumerate(pair);});
			if (!entityError)
			{
				if (variant.Pairs().Count() > 0)
				{
					DKBufferStream stream;
					if (variant.ExportStream(&stream))
					{
						DKBuffer* buffer = stream.BufferObject();
						EntityChunk chunk;
						if (chunk.Create(L"",L"", compress ? 'varc' : 'vars', 'hash', buffer, compress))
							chunks.Add(chunk);
						else
							entityError = true;						
					}
					else
						entityError = true;
				}
			}
			return !entityError;
		}
		DKVariant variant;
		DKArray<EntityChunk> chunks;
		bool compress;
		bool entityError;
	private:
		bool Enumerate(const EntityMap::Pair& p)
		{
			bool failed = false;
			const VariantEntity* ve = p.value->Variant();
			const SerializerEntity* se = p.value->Serializer();
			const ExternalEntity* ee = p.value->External();
			const ExternalEntityArray* ea = p.value->ExternalArray();
			const ExternalEntityMap* em = p.value->ExternalMap();
			if (ve && ve->setter)
			{
				DKVariant v(DKVariant::TypeUndefined);
				ve->getter->Invoke(v);
				if (v.ValueType() != DKVariant::TypeUndefined)
					variant.Pairs().Update(p.key, v);
				else
					failed = true;
			}
			if (se && se->serializer)
			{
				EntityChunk chunk;
				if (chunk.Create(p.key, L"", compress? 'serc' : 'sers', 0, se->serializer->Serialize(SerializeFormBinary), compress))
					chunks.Add(chunk);
				else
					failed = true;
			}
			if (ee && ee->getter)
			{
				EntityChunk chunk;
				DKObject<DKResource> res = NULL;
				ee->getter->Invoke(res);
				if (chunk.CreateExternal(p.key, L"", 0, res, ee->external, compress))
					chunks.Add(chunk);
				else
					failed = true;
			}
			if (ea && ea->getter)
			{
				ExternalArrayType eat;
				ea->getter->Invoke(eat);
				for (DKResource* res : eat)
				{
					if (res)
					{
						EntityChunk chunk;
						if (chunk.CreateExternal(L"", p.key, 'exta', res, ea->external, compress))
							chunks.Add(chunk);
						else
							failed = true;
					}
				}
			}
			if (em && em->getter)
			{
				ExternalMapType emt;
				em->getter->Invoke(emt);
				emt.EnumerateForward( [&](ExternalMapType::Pair& pair)
				{
					DKResource* res = pair.value;
					if (res)
					{
						EntityChunk chunk;
						if (chunk.CreateExternal(pair.key, p.key, 'extm', res, em->external, compress))
							chunks.Add(chunk);
						else
							failed = true;
					}
				});
			}

			if (failed && p.value->faultHandler == NULL)
			{
				DKLog("DKSerializer Error: entity(%ls) invalid.\n", (const wchar_t*)p.key);
				entityError = true;
			}
			return !entityError;
		}
	} queryEntities;

	DKCriticalSection<DKSpinLock> guard(this->lock);

	bool serializeSucceed = false;
	size_t numBytesWritten = 0;

	if (this->callback)
		this->callback->Invoke(StateSerializeBegin);	

	if (this->resourceClass.Length() > 0)
	{
		if (queryEntities(this->entityMap, (sf == SerializeFormCompressedBinary)))
		{
			size_t unpackedSize = 0;		// original length
			size_t packedSize = 0;			// compressed length

			bool proceed = true;

			// writing file header (byte order mark included)
			if (proceed)
			{
				size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
				size_t wrote = output->Write(DKSERIALIZER_HEADER_STRING, headerLen);
				numBytesWritten += wrote;
				if (wrote != headerLen)
					proceed = false;
			}
			// writing version
			if (proceed)
			{
				unsigned short version = DKSERIALIZER_VERSION;
				size_t wrote = output->Write(&version, sizeof(version));
				numBytesWritten += wrote;
				if (wrote != sizeof(version))
					proceed = false;
			}
			// writing class-id
			if (proceed)
			{
				DKStringU8 classId(this->resourceClass);
				unsigned long long classIdLen = classId.Bytes();
				size_t wrote = output->Write(&classIdLen, sizeof(classIdLen));
				numBytesWritten += wrote;
				if (wrote == sizeof(classIdLen))
				{
					wrote = output->Write((const char*)classId, classIdLen);
					numBytesWritten += wrote;
					if (wrote != classIdLen)
						proceed = false;
				}
				else
					proceed = false;
			}
			// num-chunks
			if (proceed)
			{
				unsigned long long numChunks = queryEntities.chunks.Count();
				size_t wrote = output->Write(&numChunks, sizeof(numChunks));
				if (wrote != sizeof(numChunks))
					proceed = false;
			}

			if (proceed)
			{
				size_t wrote = 0;
				const unsigned int chunkHdr = 'chdr';
				for (size_t i = 0; i < queryEntities.chunks.Count(); ++i)
				{
					EntityChunk& chunk = queryEntities.chunks.Value(i);

					DKASSERT_DEBUG(chunk.data != NULL);
					DKASSERT_DEBUG(chunk.data->Length() > 0);

					unpackedSize += chunk.unpackedSize;
					packedSize += chunk.data->Length();

					// chunk-header
					wrote = output->Write(&chunkHdr, sizeof(chunkHdr));
					numBytesWritten += wrote;
					if (wrote != sizeof(chunkHdr))
					{
						proceed = false;
						break;
					}
					// chunk-type
					wrote = output->Write(&chunk.type, sizeof(chunk.type));
					numBytesWritten += wrote;
					if (wrote != sizeof(chunk.type))
					{
						proceed = false;
						break;
					}
					// ctype
					wrote = output->Write(&chunk.cType, sizeof(chunk.cType));
					numBytesWritten += wrote;
					if (wrote != sizeof(chunk.cType))
					{
						proceed = false;
						break;
					}
					// objectKey
					unsigned long long objKeyLen = chunk.key.Bytes();
					wrote = output->Write(&objKeyLen, sizeof(objKeyLen));
					numBytesWritten += wrote;
					if (wrote == sizeof(objKeyLen))
					{
						wrote = output->Write((const char*)chunk.key, objKeyLen);
						numBytesWritten += wrote;
						if (wrote != objKeyLen)
						{
							proceed = false;
							break;
						}
					}
					else
					{
						proceed = false;
						break;
					}
					// containerKey
					unsigned long long ctKeyLen = chunk.containerKey.Bytes();
					wrote = output->Write(&ctKeyLen, sizeof(ctKeyLen));
					numBytesWritten += wrote;
					if (wrote == sizeof(ctKeyLen))
					{
						wrote = output->Write((const char*)chunk.containerKey, ctKeyLen);
						numBytesWritten += wrote;
						if (wrote != ctKeyLen)
						{
							proceed = false;
							break;
						}
					}
					else
					{
						proceed = false;
						break;
					}
					// data original size
					wrote = output->Write(&chunk.unpackedSize, sizeof(chunk.unpackedSize));
					numBytesWritten += wrote;
					if (wrote != sizeof(chunk.unpackedSize))
					{
						proceed = false;
						break;
					}
					// data (size, bytes)
					const void* ptr = chunk.data->LockShared();
					unsigned long long dataLen = chunk.data->Length();
					wrote = output->Write(&dataLen, sizeof(dataLen));
					numBytesWritten += wrote;
					if (wrote == sizeof(dataLen))
					{
						wrote = output->Write(ptr, dataLen);
						numBytesWritten += wrote;
						if (wrote != dataLen)
							proceed = false;
					}
					else
						proceed = false;
					chunk.data->UnlockShared();

					if (!proceed)
						break;
				}
			}
			if (proceed)
			{
				serializeSucceed = true;
				if (sf == SerializeFormCompressedBinary)
				{
					//size_t cmp = unpackedSize - packedSize;
					//DKLog("DKSerializer class:%ls data size:%llu/%llu (compress ratio:%.1f%%)\n",
					//	(const wchar_t*)this->resourceClass,
					//	(unsigned long long)packedSize,
					//	(unsigned long long)unpackedSize,
					//	(static_cast<double>(cmp) / static_cast<double>(unpackedSize)) * 100.0);
				}
				else
				{
					//DKLog("DKSerializer class:%ls data size:%llu\n", (const wchar_t*)this->resourceClass, (unsigned long long)packedSize);
				}
			}
			else
			{
				DKLog("DKSerializer::Serialize failed. (stream write error)\n");
			}
		}
		else	// queryEntity failed
		{
			DKLog("DKSerializer::Serialize failed. (entity error)\n");
		}
	}
	else // resourceClass name error
	{
		DKLog("DKSerializer::Serialize failed. (Class-Id invalid)\n");
	}

	if (this->callback)
	{
		if (serializeSucceed)
			this->callback->Invoke(StateSerializeSucceed);
		else
			this->callback->Invoke(StateSerializeFailed);
	}

	return numBytesWritten;
}

bool DKSerializer::DeserializeBinaryOperations(DKStream* s, DKArray<DKObject<DeserializerEntity>>& entities, DKResourceLoader* loader) const
{
	if (s == NULL || s->IsReadable() == false)
		return false;

	size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
	char name[64];
	bool validHeader = false;
	bool littleEndian = false;
	if (s->Read(name, headerLen) == headerLen)
	{
		if (strncmp(name, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0)
		{
			validHeader = true;
			littleEndian = false;
		}
		else if (strncmp(name, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0)
		{
			validHeader = true;
			littleEndian = true;
		}
	}
	if (!validHeader)
		return false;

	struct
	{
		uint16_t operator () (uint16_t v) const
		{
			return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
		}
		uint32_t operator () (uint32_t v) const
		{
			return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
		}
		uint64_t operator () (uint64_t v) const
		{
			return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
		}
		bool littleEndian;
	} byteorder = {littleEndian};

	DKString classId = L"";
	if (true)		// verify essential info
	{
		// verify version
		unsigned short version;
		if (s->Read(&version, sizeof(version)) != sizeof(version))
			return false;
		version = byteorder(version);
		if (version > DKSERIALIZER_VERSION)
		{
			DKLog("DKSerializer::Deserialize failed: wrong binary version: 0x%x.", static_cast<unsigned int>(version));
			return false;
		}
		else if (version < DKSERIALIZER_VERSION)
		{
			DKLog("DKSerializer::Deserialize warning: file is older version:0x%x current-version:0x%0x.\n", static_cast<unsigned int>(version), static_cast<unsigned int>(DKSERIALIZER_VERSION));
		}
		// verify class-id
		unsigned long long classLen;
		if (s->Read(&classLen, sizeof(classLen)) != sizeof(classLen))
			return false;
		classLen = byteorder(classLen);
		if (classLen > 0 && classLen <= s->RemainLength())
		{
			char* cid = (char*)malloc(classLen);
			if (s->Read(cid, classLen) == classLen)
			{
				classId.SetValue(cid, classLen);
			}
			free(cid);
		}
		else
			return false;
	}

	DKCriticalSection<DKSpinLock> guard(lock);

	struct EntityLoader
	{
		void operator () (
			const DKString& key,
			const DKString& cKey,
			unsigned int type, unsigned int ctype, 
			unsigned long long unpackedSize,
			DKData* d,
			DKResourceLoader* loader,
			EntityRestore::Entity& entity,
			const EntityMap& entityMap) const
		{
			DKObject<DKData> data = NULL;
			if (type == 'varc' || type == 'serc' || type == 'extc')
			{
				// uncompress data
				const void* p = d->LockShared();
				data = DKBuffer::Decompress(p, d->Length()).SafeCast<DKData>();
				d->UnlockShared();
			}
			else
			{
				data = d;
			}

			if (data && data->Length() == unpackedSize)
			{
				unsigned int crc = 0;
				if (type == 'varc' || type == 'vars')
				{
					const void* ptr = data->LockShared();
					crc = DKHashCRC32(ptr, data->Length()).digest[0];
					data->UnlockShared();
					if (crc == ctype)
					{
						DKDataStream stream(data);
						if (entity.deserializer->rootValue.ImportStream(&stream) == false)
						{
							// restoration error!
							entity.deserializer->rootValue.SetValueType(DKVariant::TypeUndefined);
						}
					}
					else	// CRC error.
					{
						DKLog("DKSerializer warning: CRC error!\n");
						entity.deserializer->rootValue.SetValueType(DKVariant::TypeUndefined);
					}
				}
				else if (type == 'sers' || type == 'serc')
				{
					const EntityMap::Pair* ep = entityMap.Find(key);
					if (ep)
					{
						const SerializerEntity* se = ep->value->Serializer();
						if (se && se->serializer)
						{
							DKDataStream stream(data);
							EntityRestore::DeserializerArray de;
							if (se->serializer->DeserializeBinaryOperations(&stream, de, loader))
							{
								entity.includes.Insert(key, de);
							}
						}
					}
				}
				else if (type == 'extn' || type == 'exts' || type == 'extc')
				{
					DKObject<DKResource> resource = NULL;
					if (type == 'extn')
					{
						const char* p = (const char*)d->LockShared();
						DKString filename(p, d->Length());
						d->UnlockShared();
						size_t len = filename.Length();
						if (len > 0 && loader)
						{
							resource = loader->LoadResource(filename);
						}
					}
					else if (loader)
					{
						resource = loader->ResourceFromData(data, key);
					}
					if (resource)
					{
						switch (ctype)
						{
						case 0:
							entity.deserializer->externals.Insert(key, resource);
							break;
						case 'exta':
							entity.deserializer->externalArrays.Value(cKey).Add(resource);
							break;
						case 'extm':
							entity.deserializer->externalMaps.Value(cKey).Update(key, resource);
							break;
						}
					}
				}
				else	// unknown type?
				{
					DKLog("DKSerializer warning: Unknown type(0x%x) found! (ignored)\n", type);
				}
			}
		}
	} entityLoader;

	if (this->resourceClass.Compare(classId))
	{
		DKLog("DKSerializer::Deserialize failed: ClassId mismatch. (%ls != %ls)\n", (const wchar_t*)this->resourceClass, (const wchar_t*)classId);
		return false;
	}
	unsigned long long numChunks;
	if (s->Read(&numChunks, sizeof(numChunks)) != sizeof(numChunks))
	{
		DKLog("DKSerializer::Deserialize failed: Stream error.");
		return false;
	}
	numChunks = byteorder(numChunks);
	//DKLog("DKSerializer(%ls) total chunks:%llu\n", (const wchar_t*)this->resourceClass, numChunks);

	DKDataStream* dataStream = DKObject<DKStream>(s).SafeCast<DKDataStream>();

	EntityRestore::Entity restoreEntities;

	DKString errorDesc = L"Unknown error";
	bool error = false;
	for (size_t i = 0; i < numChunks && error == false; ++i)
	{
#define ERROR_BREAK(mesg)		errorDesc = mesg; error = true; break;

		unsigned int hdr;
		if (s->Read(&hdr, sizeof(hdr)) != sizeof(hdr))	{ERROR_BREAK(L"Stream Error")}
		hdr = byteorder(hdr);
		if (hdr != 'chdr')								{ERROR_BREAK(L"Stream Error")}

		unsigned int type;
		unsigned int ctype;
		unsigned long long keyLen;
		unsigned long long cKeyLen;
		unsigned long long unpackedSize;
		unsigned long long dataLength;
		DKString objectKey = L"";
		DKString containerKey = L"";

		if (s->Read(&type, sizeof(type)) != sizeof(type))		{ERROR_BREAK(L"Stream Error")}
		if (s->Read(&ctype, sizeof(ctype)) != sizeof(ctype))	{ERROR_BREAK(L"Stream Error")}	
		if (s->Read(&keyLen, sizeof(keyLen)) != sizeof(keyLen))	{ERROR_BREAK(L"Stream Error")}

		type = byteorder(type);
		ctype = byteorder(ctype);
		keyLen = byteorder(keyLen);

		if (keyLen > 0 && keyLen <= s->RemainLength())
		{
			DKData* data = NULL;
			if (dataStream && s->IsSeekable())
				data = dataStream->DataSource();

			if (data)
			{
				DKStream::Position pos = s->GetPos();
				const char* ptr = (const char*)data->LockShared();
				objectKey.SetValue(&ptr[pos], keyLen);
				data->UnlockShared();
				pos += keyLen;
				s->SetPos(pos);
			}
			else
			{
				char* tmp = (char*)malloc(keyLen);
				if (s->Read(tmp, keyLen) == keyLen)
					objectKey.SetValue(tmp, keyLen);
				else
					error = true;
				free(tmp);
				if (error)	break;
			}
		}
		if (s->Read(&cKeyLen, sizeof(cKeyLen)) != sizeof(cKeyLen))	{ERROR_BREAK(L"Stream Error")}
		cKeyLen = byteorder(cKeyLen);

		if (cKeyLen > 0 && cKeyLen <= s->RemainLength())
		{
			DKData* data = NULL;
			if (dataStream && s->IsSeekable())
				data = dataStream->DataSource();

			if (data)
			{
				DKStream::Position pos = s->GetPos();
				const char* ptr = (const char*)data->LockShared();
				containerKey.SetValue(&ptr[pos], cKeyLen);
				data->UnlockShared();
				pos += cKeyLen;
				s->SetPos(pos);
			}
			else
			{
				char* tmp = (char*)malloc(cKeyLen);
				if (s->Read(tmp, cKeyLen) == cKeyLen)
					containerKey.SetValue(tmp, cKeyLen);
				else
					error = true;
				free(tmp);
				if (error) break;
			}
		}

		if (s->Read(&unpackedSize, sizeof(unpackedSize)) != sizeof(unpackedSize))	{ERROR_BREAK(L"Stream Error")}
		unpackedSize = byteorder(unpackedSize);

		if (s->Read(&dataLength, sizeof(dataLength)) != sizeof(dataLength))		{ERROR_BREAK(L"Stream Error")}
		dataLength = byteorder(dataLength);

		if (dataLength > 0 && dataLength <= s->RemainLength())
		{
			DKData* data = NULL;
			if (dataStream && s->IsSeekable())
				data = dataStream->DataSource();

			if (data)
			{
				DKStream::Position pos = s->GetPos();
				const char* ptr = (const char*)data->LockShared();
				const void* p = &ptr[pos];

				entityLoader(objectKey, containerKey, type, ctype, unpackedSize, DKData::StaticData(p, dataLength), loader, restoreEntities, this->entityMap);

				data->UnlockShared();
				pos += dataLength;
				s->SetPos(pos);
			}
			else
			{
				DKObject<DKBuffer> buffer = DKBuffer::Create(NULL, dataLength);
				void* tmp = buffer->LockExclusive();
				size_t numRead = s->Read(tmp, dataLength);
				buffer->UnlockExclusive();
				if (numRead == dataLength)
					entityLoader(objectKey, containerKey, type, ctype, unpackedSize, buffer, loader, restoreEntities, this->entityMap);
				else
				{ERROR_BREAK(L"Stream Error")}
			}
		}
	}

	if (error)
	{
		DKLog("DKSerializer::Deserialize failed: %ls", (const wchar_t*)errorDesc);
		return false;
	}

	// compare internal type with data that picked out, and generate operations.
	return EntityRestore().ExtractOperations(this, restoreEntities, entities);
}

bool DKSerializer::DeserializeBinary(DKStream* s, DKResourceLoader* p) const
{
	// create DKOperation for determine serializable.
	// all includes should be serializable.

	DKArray<DKObject<DeserializerEntity>> deserializers;

	if (DeserializeBinaryOperations(s, deserializers, p))
	{
		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			if (deserializers.Value(i)->callback)
				deserializers.Value(i)->callback->Invoke(StateDeserializeBegin);
		}

		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			DeserializerEntity* de = deserializers.Value(i);
			for (size_t k = 0; k < de->operations.Count(); ++k)
				de->operations.Value(k)->Perform();

			// clear finished operations.
			de->operations.Clear();
			de->rootValue.SetValueType(DKVariant::TypeUndefined);
		}

		for (size_t i = 0; i < deserializers.Count(); ++i)
		{
			if (deserializers.Value(i)->callback)
				deserializers.Value(i)->callback->Invoke(StateDeserializeSucceed);
		}
		return true;
	}
	return false;
}

bool DKSerializer::Deserialize(DKStream* s, DKResourceLoader* p) const
{
	if (s->IsReadable())
	{
		DKStream::Position pos = s->GetPos();
		// determine file type, binary or XML.
		size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
		char name[64];
		bool validHeader = false;
		if (s->Read(name, headerLen) == headerLen)
		{
			if (strncmp(name, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0 ||
				strncmp(name, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0)
				validHeader = true;
		}

		s->SetPos(pos);
		if (validHeader)	// format is binary
		{
			return DeserializeBinary(s, p);
		}
		else // try to open with XMLParser.
		{
			DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, s);
			if (doc)
				return Deserialize(doc->RootElement(), p);
		}
	}
	return false;
}

bool DKSerializer::Deserialize(const DKData* d, DKResourceLoader* p) const
{
	if (d)
	{
		// determine file type, binary or XML.
		const char* ptr = (const char*)d->LockShared();
		size_t len = d->Length();
		size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
		bool validHeader = false;
		if (len >= headerLen)
		{
			if (strncmp(ptr, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0 ||
				strncmp(ptr, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0)
				validHeader = true;
		}
		d->UnlockShared();

		if (validHeader)		// format is binary
		{
			const void* ptr = d->LockShared();
			DKObject<DKData> data = DKData::StaticData(ptr, d->Length());
			DKDataStream stream(data);
			bool ret = DeserializeBinary(&stream, p);
			data = NULL;
			d->UnlockShared();
			return ret;
		}
		else // try to open with XMLParser.
		{
			DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, d);
			if (doc)
				return Deserialize(doc->RootElement(), p);
		}
	}
	return false;
}

bool DKSerializer::DeserializeBinary(DKStream* s, DKResourceLoader* p, Selector* sel)
{
	DKASSERT_DEBUG(s != NULL);
	DKASSERT_DEBUG(sel != NULL);

	DKObject<DKSerializer> serializer = NULL;
	DKStream::Position pos = s->GetPos();

	size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
	char name[64];
	bool validHeader = false;
	bool littleEndian = false;
	if (s->Read(name, headerLen) == headerLen)
	{
		if (strncmp(name, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0)
		{
			validHeader = true;
			littleEndian = false;
		}
		else if (strncmp(name, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0)
		{
			validHeader = true;
			littleEndian = true;
		}
	}
	if (validHeader)
	{
		struct
		{
			uint16_t operator () (uint16_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			uint32_t operator () (uint32_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			uint64_t operator () (uint64_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			bool littleEndian;
		} byteorder = {littleEndian};

		unsigned short ver;
		if (s->Read(&ver, sizeof(ver)) == sizeof(ver))
		{
			ver = byteorder(ver);
			if (ver <= DKSERIALIZER_VERSION)
			{
				unsigned long long clsLen;
				if (s->Read(&clsLen, sizeof(clsLen)) == sizeof(clsLen))
				{
					clsLen = byteorder(clsLen);
					if (clsLen > 0)
					{
						char* clsId = (char*)malloc(clsLen);

						DKString classId = L"";
						if (s->Read(clsId, clsLen) == clsLen)
						{
							classId.SetValue(clsId, clsLen);
						}
						free(clsId);

						if (classId.Length() > 0)
						{
							serializer = sel->Invoke(classId);
						}
					}
				}
			}
		}
	}

	s->SetPos(pos);
	if (serializer)
	{
		return serializer->DeserializeBinary(s, p);
	}
	return false;
}

bool DKSerializer::RestoreObject(const DKXMLElement* e, DKResourceLoader* p, Selector* sel)
{
	if (e && sel && e->name.CompareNoCase(L"DKSerializer") == 0)
	{
		for (size_t i = 0; i < e->attributes.Count(); ++i)
		{
			if (e->attributes.Value(i).name.CompareNoCase(L"class") == 0)
			{
				DKObject<DKSerializer> s = sel->Invoke(e->attributes.Value(i).value);
				if (s)
					return s->Deserialize(e, p);
				break;
			}
		}
	}
	return false;
}

bool DKSerializer::RestoreObject(DKStream* s, DKResourceLoader* p, Selector* sel)
{
	if (s && s->IsReadable() && sel)
	{
		DKStream::Position pos = s->GetPos();
		// determine file type, binary or XML.
		size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
		char name[64];
		bool validHeader = false;
		if (s->Read(name, headerLen) == headerLen)
		{
			validHeader = (strncmp(name, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0 ||
						   strncmp(name, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0);
		}

		s->SetPos(pos);
		if (validHeader)	// format is binary
		{
			return DeserializeBinary(s, p, sel);
		}
		else
		{
			DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, s);
			if (doc)
				return RestoreObject(doc->RootElement(), p, sel);
		}
	}
	return false;
}

bool DKSerializer::RestoreObject(const DKData* d, DKResourceLoader* p, Selector* sel)
{
	if (d && sel)
	{
		// determine file type, binary or XML.
		const char* ptr = (const char*)d->LockShared();
		size_t len = d->Length();
		size_t headerLen = strlen(DKSERIALIZER_HEADER_STRING);
		bool validHeader = false;
		DKString classId = L"";
		if (len >= headerLen)
		{
			validHeader = (strncmp(ptr, DKSERIALIZER_HEADER_STRING_BIG_ENDIAN, headerLen) == 0 ||
						   strncmp(ptr, DKSERIALIZER_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0);
		}
		d->UnlockShared();

		if (validHeader)
		{
			const void* ptr2 = d->LockShared();
			DKObject<DKData> data = DKData::StaticData(ptr2, d->Length());
			DKDataStream stream(data);
			bool ret = DeserializeBinary(&stream, p, sel);
			data = NULL;
			d->UnlockShared();
			return ret;
		}
		else
		{
			DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, d);
			if (doc)
				return RestoreObject(doc->RootElement(), p, sel);
		}
	}
	return false;
}

size_t DKSerializer::Serialize(SerializeForm sf, DKFoundation::DKStream* output) const
{
	size_t s = 0;
	switch (sf)
	{
	case SerializeFormXML:
	case SerializeFormBinXML:
		s = DKXMLDocument(this->SerializeXML(sf)).Export(DKStringEncoding::UTF8, output);
		break;
	case SerializeFormBinary:
	case SerializeFormCompressedBinary:
		s = this->SerializeBinary(sf, output);
		break;
	}
	return s;
}

DKObject<DKData> DKSerializer::Serialize(SerializeForm sf) const
{
	DKObject<DKData> data = NULL;
	switch (sf)
	{
	case SerializeFormXML:
	case SerializeFormBinXML:
		if (true)
		{
			DKObject<DKXMLElement> e = this->SerializeXML(sf);
			if (e)
			{
				data = DKXMLDocument(e).Export(DKStringEncoding::UTF8);
			}
		}
		break;
	case SerializeFormBinary:
	case SerializeFormCompressedBinary:
		if (true)
		{
			DKBufferStream stream;
			if (this->SerializeBinary(sf, &stream) > 0)
			{
				data = stream.DataSource();
			}
		}
		break;
	}
	return data;
}
