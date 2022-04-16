//
//  File: DKPropertySet.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2022 Hongtae Kim. All rights reserved.
//

#include "DKPropertySet.h"

using namespace DKFramework;

DKPropertySet::DKPropertySet()
	: dataSet(DKVariant::TypePairs)
{
}

DKPropertySet::~DKPropertySet()
{
}

int DKPropertySet::Import(const DKString& url, bool overwrite)
{
	DKObject<DKXmlDocument> doc = DKXmlDocument::Open(DKXmlDocument::TypeXML, url);
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
	prop.dataSet.Pairs().EnumerateForward([&](const PropertyMap::Pair& pair)
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

int DKPropertySet::Import(const DKXmlElement* e, bool overwrite)
{
	if (e && e->name.CompareNoCase(L"DKPropertySet") == 0)
	{
		int imported = 0;
		for (int i = 0; i < e->nodes.Count(); i++)
		{
			if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeElement)
			{
				const DKXmlElement* pnode = e->nodes.Value(i).SafeCast<DKXmlElement>();
				if (pnode->name.CompareNoCase(L"Property") == 0)
				{			
					bool keyFound = false;
					DKString key = L"";
					DKVariant value;
					for (int k = 0; k < pnode->nodes.Count(); ++k)
					{
						if (pnode->nodes.Value(k)->Type() == DKXmlNode::NodeTypeElement)
						{
							const DKXmlElement* node = pnode->nodes.Value(k).SafeCast<DKXmlElement>();
							if (node->name.CompareNoCase(L"Key") == 0)
							{
								for (int x = 0; x < node->nodes.Count(); ++x)
								{
									if (node->nodes.Value(x)->Type() == DKXmlNode::NodeTypeCData)
										key += node->nodes.Value(x).SafeCast<DKXmlCData>()->value;
									else if (node->nodes.Value(x)->Type() == DKXmlNode::NodeTypePCData)
										key += node->nodes.Value(x).SafeCast<DKXmlPCData>()->value;
								}
								keyFound = true;
							}
							else if (node->name.CompareNoCase(L"DKVariant") == 0)	// XML-DKVariant node
							{
								value.ImportXML(node);
							}
						}
						else if (pnode->nodes.Value(k)->Type() == DKXmlNode::NodeTypeCData)		// DKVariant-binary
						{
							DKObject<DKBuffer> compressed = DKBuffer::Base64Decode(pnode->nodes.Value(k).SafeCast<DKXmlCData>()->value);
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
	DKObject<DKXmlDocument> doc = DKXmlDocument::Open(DKXmlDocument::TypeXML, stream);
	if (doc)
	{
		return Import(doc->RootElement(), overwrite);
	}
	return -1;
}

int DKPropertySet::Export(const DKString& file, bool exportXML) const
{
	int exported = 0;
	DKObject<DKXmlElement> e = Export(exportXML, &exported);
	if (e)
	{
		DKObject<DKData> buffer = DKXmlDocument(NULL, e).Export(DKStringEncoding::UTF8);
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
		DKObject<DKXmlElement> e = Export(exportXML, &exported);
		if (e)
		{
			DKObject<DKData> buffer = DKXmlDocument(NULL, e).Export(DKStringEncoding::UTF8);
			if (buffer && buffer->Length() > 0)
			{
				stream->Write(buffer->Contents(), buffer->Length());
				return exported;
			}
		}
	}
	return -1;
}

DKObject<DKXmlElement> DKPropertySet::Export(bool exportXML, int* numExported) const
{
	DKCriticalSection<DKSpinLock> guard(lock);

	int exported = 0;
	DKObject<DKXmlElement> e = DKObject<DKXmlElement>::New();
	e->name = L"DKPropertySet";

	bool exportFailed = false;

	this->dataSet.Pairs().EnumerateForward([&](const PropertyMap::Pair& pair, bool* stop)
	{
		DKObject<DKXmlElement> pnode = NULL;
		if (exportXML)
		{
			DKObject<DKXmlElement> value = pair.value.ExportXML();
			if (value != NULL)
			{
				pnode = DKObject<DKXmlElement>::New();
				pnode->name = L"Property";

				DKObject<DKXmlElement> key = DKObject<DKXmlElement>::New();
				key->name = L"Key";
				DKObject<DKXmlPCData> keyPCData = DKObject<DKXmlPCData>::New();
				keyPCData->value = pair.key;
				key->nodes.Add(keyPCData.SafeCast<DKXmlNode>());

				pnode->nodes.Add(key.SafeCast<DKXmlNode>());
				pnode->nodes.Add(value.SafeCast<DKXmlNode>());
			}
		}
		else
		{
			DKBufferStream stream;
			if (pair.value.ExportStream(&stream))
			{
				const DKBuffer* buffer = stream.Buffer();
				if (buffer && buffer->Length() > 0)
				{
					DKObject<DKBuffer> compressed = buffer->Compress(DKCompressor::Default);
					if (compressed)
					{
						DKObject<DKXmlCData> value = DKObject<DKXmlCData>::New();
						if (compressed->Base64Encode(value->value))
						{
							pnode = DKObject<DKXmlElement>::New();
							pnode->name = L"Property";

							DKObject<DKXmlElement> key = DKObject<DKXmlElement>::New();
							key->name = L"Key";
							DKObject<DKXmlPCData> keyPCData = DKObject<DKXmlPCData>::New();
							keyPCData->value = pair.key;
							key->nodes.Add(keyPCData.SafeCast<DKXmlNode>());

							pnode->nodes.Add(key.SafeCast<DKXmlNode>());
							pnode->nodes.Add(value.SafeCast<DKXmlNode>());
						}
					}
				}
			}
		}
		if (pnode != NULL)
		{
			e->nodes.Add(pnode.SafeCast<DKXmlNode>());
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
	return dataSet.Pairs().Find(key) != NULL;
}

const DKVariant& DKPropertySet::Value(const DKString& key) const
{
	DKCriticalSection<DKSpinLock> guard(lock);

	const PropertyMap::Pair* p = dataSet.Pairs().Find(key);
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
	bool result = dataSet.Pairs().Insert(key, value);
	lock.Unlock();

	if (result)
	{
		CallbackObservers(key, insertionCallbacks, value);
	}

	return result;
}

void DKPropertySet::SetValue(const DKString& key, const DKVariant& value)
{
	lock.Lock();

	DKVariant oldValue;
	bool modification = false;

	const PropertyMap::Pair* p = dataSet.Pairs().Find(key);
	if (p)
	{
		oldValue = p->value;
		modification = true;
	}
	dataSet.Pairs().Update(key, value);

	lock.Unlock();

	if (modification)
	{
		CallbackObservers(key, modificationCallbacks, oldValue, value);
	}
	else
	{
		CallbackObservers(key, insertionCallbacks, value);
	}
}

void DKPropertySet::ReplaceValue(const DKString& key, Replacer* replacer)
{
	lock.Lock();

	DKVariant oldValue;
	bool modification = false;
	bool removed = false;

	const PropertyMap::Pair* p = dataSet.Pairs().Find(key);
	if (p)
	{
		oldValue = p->value;
		modification = true;
	}
	DKVariant value = replacer->Invoke(oldValue);
	removed = value.ValueType() == DKVariant::TypeUndefined;
	if (removed)
		dataSet.Pairs().Remove(key);
	else
		dataSet.Pairs().Update(key, value);
	lock.Unlock();

	if (modification)
	{
		if (removed)
			CallbackObservers(key, deletionCallbacks, oldValue);
		else
			CallbackObservers(key, modificationCallbacks, oldValue, value);
	}
	else if (!removed)
	{
		CallbackObservers(key, insertionCallbacks, value);
	}
}

void DKPropertySet::Remove(const DKString& key)
{
	lock.Lock();

	DKVariant oldValue;
	bool deletion = false;

	const PropertyMap::Pair* p = dataSet.Pairs().Find(key);
	if (p)
	{
		oldValue = p->value;
		deletion = true;

		dataSet.Pairs().Remove(key);
	}

	lock.Unlock();

	if (deletion)
	{
		CallbackObservers(key, deletionCallbacks, oldValue);
	}
}

size_t DKPropertySet::NumberOfEntries() const
{
	return dataSet.Pairs().Count();
}

bool DKPropertySet::LookUpValueForKeyPath(const DKString& path, DKVariant::ConstKeyPathEnumerator* callback) const
{
	DKCriticalSection<DKSpinLock> guard(lock);
	return dataSet.FindObjectAtKeyPath(path, callback);
}

DKPropertySet& DKPropertySet::DefaultSet()
{
	static DKPropertySet ps;
	return ps;
}

DKPropertySet& DKPropertySet::SystemConfig()
{
	static DKPropertySet ps;
	return ps;
}

void DKPropertySet::AddObserver(ObserverContext context,
								const DKString& key,
								InsertionCallback* insertion,
								ModificationCallback* modification,
								DeletionCallback* deletion)
{
	if (context)
	{
		DKCriticalSection<DKSpinLock> guard(callbackLock);

		if (insertion)
			insertionCallbacks.Value(key).Update(context, insertion);
		else if (insertionCallbacks.Find(key))
			insertionCallbacks.Value(key).Remove(context);
		if (modification)
			modificationCallbacks.Value(key).Update(context, modification);
		else if (modificationCallbacks.Find(key))
			modificationCallbacks.Value(key).Remove(context);
		if (deletion)
			deletionCallbacks.Value(key).Update(context, deletion);
		else if (deletionCallbacks.Find(key))
			deletionCallbacks.Value(key).Remove(context);
	}
}

void DKPropertySet::RemoveObserver(ObserverContext context, const DKString& key)
{
	if (context)
	{
		DKCriticalSection<DKSpinLock> guard(callbackLock);

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

void DKPropertySet::RemoveObserver(ObserverContext context)
{
	if (context)
	{
		DKCriticalSection<DKSpinLock> guard(callbackLock);

		insertionCallbacks.EnumerateForward([context](decltype(insertionCallbacks)::Pair& pair)
		{
			pair.value.Remove(context);
		});
		modificationCallbacks.EnumerateForward([context](decltype(modificationCallbacks)::Pair& pair)
		{
			pair.value.Remove(context);
		});
		deletionCallbacks.EnumerateForward([context](decltype(deletionCallbacks)::Pair& pair)
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
		dataSet.Pairs().EnumerateForward([e](const PropertyMap::Pair& pair)
		{
			e->Invoke(pair.key, pair.value);
		});
	}
}

void DKPropertySet::EnumerateBackward(const Enumerator* e) const
{
	if (e)
	{
		DKCriticalSection<DKSpinLock> guard(lock);
		dataSet.Pairs().EnumerateBackward([e](const PropertyMap::Pair& pair)
		{
			e->Invoke(pair.key, pair.value);
		});
	}
}

template <typename T, typename... Args>
void DKPropertySet::CallbackObservers(const DKString& key, const DKMap<DKString, ObserverMap<DKObject<T>>>& target, Args&&... args) const
{
	callbackLock.Lock();
	DKArray<DKObject<T>> callbacks;
	if (auto p = target.Find(key); p)
	{
		callbacks.Reserve(p->value.Count());
		p->value.EnumerateForward([&callbacks](const typename decltype(p->value)::Pair& pair) {
			callbacks.Add(pair.value);
		});
	}
	callbackLock.Unlock();
	for (T* cb : callbacks)
		cb->Invoke(key, std::forward<Args>(args)...);
}
