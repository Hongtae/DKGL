#pragma once
#include "DCConvexShape.h"

struct DCPolyhedralConvexShape
{
	DCConvexShape base;
	DKPolyhedralConvexShape* shape;
};
