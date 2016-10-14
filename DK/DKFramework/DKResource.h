//
//  File: DKResource.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKResourceLoader;
	class DKGL_API DKResource
	{
	public:
		DKResource(void);
		virtual ~DKResource(void);

		virtual void SetName(const DKString& name);
		const DKString& Name(void) const;
		virtual void SetUUID(const DKUUID& uuid);
		const DKUUID& UUID(void) const;

		virtual DKObject<DKSerializer> Serializer(void);
		virtual DKObject<DKData> Serialize(DKSerializer::SerializeForm) const;
		virtual DKObject<DKXMLElement> SerializeXML(DKSerializer::SerializeForm) const;
		virtual bool Deserialize(const DKData*, DKResourceLoader*);
		virtual bool Deserialize(const DKXMLElement*, DKResourceLoader*);

		virtual bool Validate(void); // resource validation

		DKVariant::VPairs metadata;

	protected:
		DKAllocator& Allocator(void);

	private:
		DKString objectName;
		DKUUID objectUUID;
		DKAllocator* allocator;

		DKResource(const DKResource&);
		DKResource& operator = (const DKResource&);

		friend class DKResourceLoader;
	};
}
