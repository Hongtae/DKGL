#pragma once
#include "DCMesh.h"

struct DCStaticMesh
{
	DCMesh base;
	DKStaticMesh* mesh;
	PyObject* vertexBuffers;
	PyObject* indexBuffer;
	void UpdateVertexBuffers(void);
	void UpdateIndexBuffer(void);
};
