//
//  File: DKPropertySet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKPropertySet.h"

using namespace DKFoundation;
using namespace DKFramework;

DKPropertySet::DKPropertySet(void)
{
}

DKPropertySet::~DKPropertySet(void)
{
}

int DKPropertySet::Import(const DKString& url, bool overwrite)
{
	DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, url);
	if (doc)
	{
		return Import(doc->RootElement(), overwrite);
	}
	return -1;
}

int DKPropertySet::Import(const DKPropertySet& prop, bool overwrite)
{
	DKCriticalSection<DKSpinLock> gaurd(prop.lock);

	int imported = 0;
	prop.properties.EnumerateForward([&](const PropertyMap::Pair& pair)
	{
		if (overwrite)
		{
			SetValue(pair.key, pair.value);
			imported++;
		}
		else if (SetInitialValue(pair.key, pair.value))
		{
			imported++;
		}
	});
	return imported;
}

int DKPropertySet::Import(const DKXMLElement* e, bool overwrite)
{
	if (e && e->name.CompareNoCase(L"DKPropertySet") == 0)
	{
		int imported = 0;
		for (int i = 0; i < e->nodes.Count(); i++)
		{
			if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
			{
				const DKXMLElement* pnode = e->nodes.Value(i).SafeCast<DKXMLElement>();
				if (pnode->name.CompareNoCase(L"Property") == 0)
				{			
					bool keyFound = false;
					DKString key = L"";
					DKVariant value;
					for (int k = 0; k < pnode->nodes.Count(); ++k)
					{
						if (pnode->nodes.Value(k)->Type() == DKXMLNode::NodeTypeElement)
						{
							const DKXMLElement* node = pnode->nodes.Value(k).SafeCast<DKXMLElement>();
							if (node->name.CompareNoCase(L"Key") == 0)
							{
								for (int x = 0; x < node->nodes.Count(); ++x)
								{
									if (node->nodes.Value(x)->Type() == DKXMLNode::NodeTypeCData)
										key += node->nodes.Value(x).SafeCast<DKXMLCData>()->value;
									else if (node->nodes.Value(x)->Type() == DKXMLNode::NodeTypePCData)
										key += node->nodes.Value(x).SafeCast<DKXMLPCData>()->value;
								}
								keyFound = true;
							}
							else if (node->name.CompareNoCase(L"DKVariant") == 0)	// XML-DKVariant node
							{
								value.ImportXML(node);
							}
						}
						else if (pnode->nodes.Value(k)->Type() == DKXMLNode::NodeTypeCData)		// DKVariant-binary
						{
							DKObject<DKBuffer> compressed = DKBuffer::Base64Decode(pnode->nodes.Value(k).SafeCast<DKXMLCData>()->value);
							if (compressed)
							{
								DKObject<DKBuffer> data = compressed->Decompress();
								if (data)
								{
									DKDataStream stream(data);
									value.ImportStream(&stream);
								}
							}
						}
					}
					if (keyFound)
					{
						if (overwrite)
						{
							SetValue(key, value);
							imported++;
						}
						else if (SetInitialValue(key, value))
						{
							imported++;
						}
					}
				}
			}
		}
		return imported;
	}
	return -1;
}

int DKPropertySet::Import(DKStream* stream, bool overwrite)
{
	DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, stream);
	if (doc)
	{
		return Import(doc->RootElement(), overwrite);
	}
	return -1;
}

int DKPropertySet::Export(const DKString& file, bool exportXML) const
{
	int exported = 0;
	DKObject<DKXMLElement> e = Export(exportXML, &exported);
	if (e)
	{
		DKObject<DKData> buffer = DKXMLDocument(NULL, e).Export(DKStringEncoding::UTF8);
		if (buffer && buffer->Length() > 0)
		{
			DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenNew, DKFile::ModeShareExclusive);
			if (f)
			{
				f->Write(buffer);
				return exported;
			}
		}
	}
	return -1;
}

int DKPropertySet::Export(DKStream* stream, bool exportXML) const
{
	if (stream && stream->IsWritable())
	{
		int exported = 0;
		DKObject<DKXMLElement> e = Export(exportXML, &exported);
		if (e)
		{
			DKObject<DKData> buffer = DKXMLDocument(NULL, e).Export(DKStringEncoding::UTF8);
			if (buffer && buffer->Length() > 0)
			{
				stream->Write(buffer->LockShared(), buffer->Length());
				buffer->UnlockShared();
				return exported;
			}
		}
	}
	return -1;
}

DKObject<DKXMLElement> DKPropertySet::Export(bool exportXML, int* numExported) const
{
	DKCriticalSection<DKSpinLock> guard(lock);

	int exported = 0;
	DKObject<DKXMLElement> e = DKObject<DKXMLElement>::New();
	e->name = L"DKPropertySet";

	bool exportFailed = false;

	this->properties.EnumerateForward([&](const PropertyMap::Pair& pair, bool* stop)
	{
		DKObject<DKXMLElement> pnode = NULL;
		if (exportXML)
		{
			DKObject<DKXMLElement> value = pair.value.ExportXML();
			if (value != NULL)
			{
				pnode = DKObject<DKXMLElement>::New();
				pnode->name = L"Property";

				DKObject<DKXMLElement> key = DKObject<DKXMLElement>::New();
				key->name = L"Key";
				DKObject<DKXMLPCData> keyPCData = DKObject<DKXMLPCData>::New();
				keyPCData->value = pair.key;
				key->nodes.Add(keyPCData.SafeCast<DKXMLNode>());

				pnode->nodes.Add(key.SafeCast<DKXMLNode>());
				pnode->nodes.Add(value.SafeCast<DKXMLNode>());
			}
		}
		else
		{
			DKBufferStream stream;
			if (pair.value.ExportStream(&stream))
			{
				const DKBuffer* buffer = stream.BufferObject();
				if (buffer && buffer->Length() > 0)
				{
					DKObject<DKBuffer> compressed = buffer->Compress(DKCompressor::Deflate);
					if (compressed)
					{
						DKObject<DKXMLCData> value = DKObject<DKXMLCData>::New();
						if (compressed->Base64Encode(value->value))
						{
							pnode = DKObject<DKXMLElement>::New();
							pnode->name = L"Property";

							DKObject<DKXMLElement> key = DKObject<DKXMLElement>::New();
							key->name = L"Key";
							DKObject<DKXMLPCData> keyPCData = DKObject<DKXMLPCData>::New();
							keyPCData->value = pair.key;
							key->nodes.Add(keyPCData.SafeCast<DKXMLNode>());

							pnode->nodes.Add(key.SafeCast<DKXMLNode>());
							pnode->nodes.Add(value.SafeCast<DKXMLNode>());
						}
					}
				}
			}
		}
		if (pnode != NULL)
		{
			e->nodes.Add(pnode.SafeCast<DKXMLNode>());
			exported++;
		}
		else
		{
			exportFailed = true;
			*stop = true;					
		}
	});

	if (exportFailed)
	{
		DKLog("DKPropertySet::Export Error.(DKVariant exportXML failed.\n");
		return NULL;
	}

	if (numExported)
		*numExported = exported;

	return e;
}

bool DKPropertySet::HasValue(const DKString& key) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	return properties.Find(key) != NULL;
}

const DKVariant& DKPropertySet::Value(const DKString& key) const
{
	DKCriticalSection<DKSpinLock> guard(lock);

	const PropertyMap::Pair* p = properties.Find(key);
	if (p)
	{
		return p->value;
	}
	static DKVariant empty;
	return empty;
}

bool DKPropertySet::SetInitialValue(const DKString& key, const DKVariant& value)
{
	lock.Lock();
	bool result = properties.Insert(key, value);
	lock.Unlock();

	if (result)
	{
		auto p = insertionCallbacks.Find(key);
		if (p)
			p->value.PostInvocation(key, value);
	}

	return result;
}

void DKPropertySet::SetValue(const DKString& key, const DKVariant& value)
{
	lock.Lock();

	DKVariant oldValue;
	bool modification = false;

	const PropertyMap::Pair* p = properties.Find(key);
	if (p)
	{
		oldValue = p->value;
		modification = true;
	}
	properties.Update(key, value);

	lock.Unlock();

	if (modification)
	{
		auto p = modificationCallbacks.Find(key);
		if (p)
			p->value.PostInvocation(key, oldValue, value);
	}
	else
	{
		auto p = insertionCallbacks.Find(key);
		if (p)
			p->value.PostInvocation(key, value);
	}
}

void DKPropertySet::Remove(const DKString& key)
{
	lock.Lock();

	DKVariant oldValue;
	bool deletion = false;

	const PropertyMap::Pair* p = properties.Find(key);
	if (p)
	{
		oldValue = p->value;
		deletion = true;

		properties.Remove(key);
	}

	lock.Unlock();

	if (deletion)
	{
			auto p = deletionCallbacks.Find(key);
			if (p)
				p->value.PostInvocation(key, oldValue);
	}
}

size_t DKPropertySet::NumberOfEntries(void) const
{
	return properties.Count();
}

DKPropertySet& DKPropertySet::DefaultSet(void)
{
	static DKPropertySet	p;
	return p;
}

void DKPropertySet::SetCallback(const DKString& key, InsertionCallback* insertion, ModificationCallback* modification, DeletionCallback* deletion, DKRunLoop* runLoop, void* context)
{
	if (context)
	{
		if (insertion)
			insertionCallbacks.Value(key).SetCallback(insertion, runLoop, context);
		else if (insertionCallbacks.Find(key))
			insertionCallbacks.Value(key).Remove(context);
		if (modification)
			modificationCallbacks.Value(key).SetCallback(modification, runLoop, context);
		else if (modificationCallbacks.Find(key))
			modificationCallbacks.Value(key).Remove(context);
		if (deletion)
			deletionCallbacks.Value(key).SetCallback(deletion, runLoop, context);
		else if (deletionCallbacks.Find(key))
			deletionCallbacks.Value(key).Remove(context);
	}
}

void DKPropertySet::RemoveCallback(const DKString& key, void* context)
{
	if (context)
	{
		auto p1 = insertionCallbacks.Find(key);
		if (p1)
			p1->value.Remove(context);
		auto p2 = modificationCallbacks.Find(key);
		if (p2)
			p2->value.Remove(context);
		auto p3 = deletionCallbacks.Find(key);
		if (p3)
			p3->value.Remove(context);
	}
}

void DKPropertySet::RemoveCallback(void* context)
{
	if (context)
	{
		insertionCallbacks.EnumerateForward([context](InsertionCallbackMap::Pair& pair)
		{
			pair.value.Remove(context);
		});
		modificationCallbacks.EnumerateForward([context](ModificationCallbackMap::Pair& pair)
		{
			pair.value.Remove(context);
		});
		deletionCallbacks.EnumerateForward([context](DeletionCallbackMap::Pair& pair)
		{
			pair.value.Remove(context);
		});
	}
}

void DKPropertySet::EnumerateForward(const Enumerator* e) const
{
	if (e)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		properties.EnumerateForward([e](const PropertyMap::Pair& pair) {e->Invoke(pair.key, pair.value);} );
	}
}

void DKPropertySet::EnumerateBackward(const Enumerator* e) const
{
	if (e)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		properties.EnumerateBackward([e](const PropertyMap::Pair& pair) {e->Invoke(pair.key, pair.value);} );
	}	
}

