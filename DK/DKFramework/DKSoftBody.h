//
//  File: DKSoftBody.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKCollisionObject.h"

namespace DKFramework
{
	/// NOT IMPLEMENTED.
	class DKGL_API DKSoftBody : public DKCollisionObject
	{
	public:
		DKSoftBody(void);
		~DKSoftBody(void);

		DKCollisionShape* CollisionShape(void)				{return NULL;}
		const DKCollisionShape* CollisionShape(void) const	{return NULL;}

		DKObject<DKSerializer> Serializer(void) override;

	protected:
		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
	};
}
