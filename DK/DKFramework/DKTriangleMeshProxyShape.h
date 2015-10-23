//
//  File: DKTriangleMeshProxyShape.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2015 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKConcaveShape.h"
#include "DKTriangleMesh.h"

////////////////////////////////////////////////////////////////////////////////
// DKTriangleMeshProxyShape
// This class is proxy shape of dynamic triangle mesh.
// This class is not serializable. An instance can be created on Rumtime only.
//
// NOT IMPLEMENTED YET.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKTriangleMeshProxyShape : public DKConcaveShape
	{
	public:
		DKTriangleMeshProxyShape(void);
		~DKTriangleMeshProxyShape(void);


		virtual DKTriangleMesh* TriangleMesh(void) = 0;
	};
}
