//
//  File: DKResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
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
	class DKLIB_API DKResource
	{
	public:
		DKResource(void);
		virtual ~DKResource(void);

		virtual void SetName(const DKFoundation::DKString& name);
		const DKFoundation::DKString& Name(void) const;
		virtual void SetUUID(const DKFoundation::DKUUID& uuid);
		const DKFoundation::DKUUID& UUID(void) const;

		virtual DKFoundation::DKObject<DKSerializer> Serializer(void);
		virtual DKFoundation::DKObject<DKFoundation::DKData> Serialize(DKSerializer::SerializeForm) const;
		virtual DKFoundation::DKObject<DKFoundation::DKXMLElement> SerializeXML(DKSerializer::SerializeForm) const;
		virtual bool Deserialize(const DKFoundation::DKData*, DKResourceLoader*);
		virtual bool Deserialize(const DKFoundation::DKXMLElement*, DKResourceLoader*);

		virtual bool Validate(void); // resource validation

		DKVariant::VPairs metadata;

	protected:
		DKFoundation::DKAllocator& Allocator(void);

	private:
		DKFoundation::DKString objectName;
		DKFoundation::DKUUID objectUUID;
		DKFoundation::DKAllocator* allocator;

		DKResource(const DKResource&);
		DKResource& operator = (const DKResource&);

		friend class DKResourceLoader;
	};
}
