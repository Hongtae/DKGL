//
//  File: DKResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKSerializer.h"
#include "DKVariant.h"

////////////////////////////////////////////////////////////////////////////////
// DKResource
// Resource for system, you can transfer resource data via file of network.
// You can use DKResourceLoader, DKSerializer to serialize object.
// A lots of serializable objects inherited from DKResource.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKResourceLoader;
	class DKGL_API DKResource
	{
	public:
		DKResource(void);
		virtual ~DKResource(void);

		virtual void SetName(const DKString& name);
		const DKString& Name(void) const;
		virtual void SetUUID(const DKUuid& uuid);
		const DKUuid& UUID(void) const;

		virtual DKObject<DKSerializer> Serializer(void);
		virtual DKObject<DKData> Serialize(DKSerializer::SerializeForm) const;
		virtual DKObject<DKXmlElement> SerializeXML(DKSerializer::SerializeForm) const;
		virtual bool Deserialize(const DKData*, DKResourceLoader*);
		virtual bool Deserialize(const DKXmlElement*, DKResourceLoader*);

		virtual bool Validate(void); // resource validation

		DKVariant::VPairs metadata;

	protected:
		DKAllocator& Allocator(void);

	private:
		DKString objectName;
		DKUuid objectUUID;
		DKAllocator* allocator;

		DKResource(const DKResource&);
		DKResource& operator = (const DKResource&);

		friend class DKResourceLoader;
	};
}
