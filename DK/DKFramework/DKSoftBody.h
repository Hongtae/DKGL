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
		DKSoftBody();
		~DKSoftBody();

		DKCollisionShape* CollisionShape()				{return NULL;}
		const DKCollisionShape* CollisionShape() const	{return NULL;}

		DKObject<DKSerializer> Serializer() override;

	protected:
		DKObject<DKModel> Clone(UUIDObjectMap&) const override;
	};
}
