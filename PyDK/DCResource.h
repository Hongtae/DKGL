#pragma once
#include <Python.h>
#include <DK/DK.h>

using namespace DKFoundation;
using namespace DKFramework;

struct DCResource
{
	PyObject_HEAD
	DKObject<DKResource> resource;
};
