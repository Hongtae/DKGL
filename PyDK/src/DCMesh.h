#pragma once
#include "DCModel.h"

struct DCMesh
{
	DCModel base;
	DKMesh* mesh;
	PyObject* material;
	PyObject* textures;
	PyObject* samplers;
	void UpdateMaterial(void);
	void UpdateTextures(void);
};
