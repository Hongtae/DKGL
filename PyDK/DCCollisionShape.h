#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKFoundation;
using namespace DKFramework;

struct DCCollisionShape
{
	PyObject_HEAD
	DKObject<DKCollisionShape> shape;
};
