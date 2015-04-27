#pragma once
#include "DCConstraint.h"

struct DCGeneric6DofConstraint
{
	DCConstraint base;
	DKGeneric6DofConstraint* constraint;
};
