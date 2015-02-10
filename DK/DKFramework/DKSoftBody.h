//
//  File: DKSoftBody.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2012-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKCollisionObject.h"

////////////////////////////////////////////////////////////////////////////////
// DKSoftBody
// soft body is used to simulate cloth and volumetric soft bodies.
//
// This class is not implemented yet. DON'T USE.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	// NOT IMPLEMENTED.
	class DKLIB_API DKSoftBody : public DKCollisionObject
	{
	public:
		DKSoftBody(void);
		~DKSoftBody(void);

		DKCollisionShape* CollisionShape(void)				{return NULL;}
		const DKCollisionShape* CollisionShape(void) const	{return NULL;}

		DKFoundation::DKObject<DKSerializer> Serializer(void) override;

	protected:
		DKFoundation::DKObject<DKModel> Clone(UUIDObjectMap&) const override;
	};
}
