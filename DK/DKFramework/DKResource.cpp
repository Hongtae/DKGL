//
//  File: DKResource.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "../lib/OpenGL.h"
#include "DKResource.h"
#include "DKSerializer.h"


using namespace DKFoundation;
using namespace DKFramework;

DKResource::DKResource(void)
: allocator(NULL)
, objectName("")
, objectUUID(DKUuid::Create())
{
}

DKResource::~DKResource(void)
{
}

void DKResource::SetName(const DKString& name)
{
	objectName = name;
}

const DKString& DKResource::Name(void) const
{
	return objectName;
}

void DKResource::SetUUID(const DKFoundation::DKUuid& uuid)
{
	DKASSERT_DEBUG(uuid.IsValid());
	this->objectUUID = uuid;
}

const DKUuid& DKResource::UUID(void) const
{
	return objectUUID;
}

bool DKResource::Validate(void)
{
	return false;
}

DKObject<DKSerializer> DKResource::Serializer(void)
{
	class LocalSerializer : public DKSerializer
	{
	public:
		DKSerializer* Init(DKResource* p)
		{
			if (p == NULL)
				return NULL;
			this->target = p;

			this->SetResourceClass(L"DKResource");

			this->Bind(L"name",
				DKFunction(this, &LocalSerializer::GetName),
				DKFunction(this, &LocalSerializer::SetName),
				DKFunction(this, &LocalSerializer::CheckName),
				NULL);

			this->Bind(L"uuid",
				DKFunction(this, &LocalSerializer::GetUUID),
				DKFunction(this, &LocalSerializer::SetUUID),
				DKFunction(this, &LocalSerializer::CheckUUID),
				DKFunction(this, &LocalSerializer::ResetUUID)->Invocation());

			this->Bind(L"metadata",
				DKFunction(this, &LocalSerializer::GetMetadata),
				DKFunction(this, &LocalSerializer::SetMetadata),
				DKFunction(this, &LocalSerializer::CheckMetadata),
				DKFunction(this, &LocalSerializer::ResetMetadata)->Invocation());

			return this;
		}
	private:
		void GetName(DKVariant& v) const
		{
			v = (const DKVariant::VString&)target->Name();
		}
		void SetName(DKVariant& v)
		{
			target->SetName(v.String());
		}
		bool CheckName(const DKVariant& v) const
		{
			return v.ValueType() == DKVariant::TypeString;
		}
		void GetUUID(DKVariant& v) const
		{
			v = (const DKVariant::VString&)target->UUID().String();
		}
		void SetUUID(DKVariant& v)
		{
			target->objectUUID = (const DKString&)v.String();
		}
		bool CheckUUID(const DKVariant& v) const
		{
			if (v.ValueType() == DKVariant::TypeString)
				return DKUuid(v.String()).IsValid();
			return false;
		}
		void ResetUUID(void)
		{
			target->objectUUID = DKUuid::Create();
		}
		void GetMetadata(DKVariant& v) const
		{
			v.SetValueType(DKVariant::TypeUndefined);
			if (target->metadata.Count() > 0)
				v = target->metadata;
		}
		void SetMetadata(DKVariant& v)
		{
			if (v.ValueType() == DKVariant::TypePairs)
				target->metadata = v.Pairs();
		}
		bool CheckMetadata(const DKVariant& v)
		{
			return v.ValueType() == DKVariant::TypePairs;
		}
		void ResetMetadata(void)
		{
			target->metadata.Clear();
		}
		DKObject<DKResource> target;
	};
	return DKObject<LocalSerializer>::New()->Init(this);
}

DKObject<DKData> DKResource::Serialize(DKSerializer::SerializeForm sf) const
{
	DKObject<DKSerializer> s = const_cast<DKResource*>(this)->Serializer();
	if (s && s->ResourceClass().Compare(L"DKResource") != 0)
	{
		DKBufferStream stream;
		if (s->Serialize(sf, &stream) > 0)
			return stream.DataSource();
	}
	else if (sf == DKSerializer::SerializeFormXML || sf == DKSerializer::SerializeFormBinXML)
	{
		DKObject<DKXMLElement> e = this->SerializeXML(sf);
		if (e)
		{
			return DKXMLDocument(e).Export(DKStringEncoding::UTF8);
		}
	}
	return NULL;
}

DKObject<DKXMLElement> DKResource::SerializeXML(DKSerializer::SerializeForm sf) const
{
	DKObject<DKSerializer> s = const_cast<DKResource*>(this)->Serializer();
	if (s && s->ResourceClass().Compare(L"DKResource") != 0)
		return s->SerializeXML(sf);
	return NULL;
}

bool DKResource::Deserialize(const DKData* d, DKResourceLoader* p)
{
	DKObject<DKSerializer> s = this->Serializer();
	if (s && s->ResourceClass().Compare(L"DKResource") != 0)
		return s->Deserialize(d, p);
	else
	{
		DKObject<DKXMLDocument> doc = DKXMLDocument::Open(DKXMLDocument::TypeXML, d);
		if (doc)
			return Deserialize(doc->RootElement(), p);
	}
	return false;
}

bool DKResource::Deserialize(const DKXMLElement* e, DKResourceLoader* p)
{
	DKObject<DKSerializer> s = this->Serializer();
	if (s && s->ResourceClass().Compare(L"DKResource") != 0)
		return s->Deserialize(e, p);
	return false;
}

DKAllocator& DKResource::Allocator(void)
{
	if (this->allocator)
		return *this->allocator;
	return DKAllocator::DefaultAllocator();
}
