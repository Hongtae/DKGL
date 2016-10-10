#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKGL;
using namespace DKGL;

struct DCResource
{
	PyObject_HEAD
	DKObject<DKResource> resource;
};
