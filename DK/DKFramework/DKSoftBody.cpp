//
//  File: DKSoftBody.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "Private/BulletUtils.h"
#include "DKSoftBody.h"

using namespace DKGL;


DKSoftBody::DKSoftBody(void)
: DKCollisionObject(ObjectType::SoftBody, new btSoftBody(NULL))
{
	DKERROR_THROW("Not Implemented");
}

DKSoftBody::~DKSoftBody(void)
{
}

DKObject<DKModel> DKSoftBody::Clone(UUIDObjectMap&) const
{
	DKERROR_THROW("Not Implemented");
	return NULL;
}

DKObject<DKSerializer> DKSoftBody::Serializer(void)
{
	DKERROR_THROW("Not Implemented");
	return NULL;
}
