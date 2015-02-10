//
//  File: DKPrimitiveIndex.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#include "DKPrimitiveIndex.h"

using namespace DKFoundation;
using namespace DKFramework;


DKString DKPrimitive::TypeToString(Type p)
{
	switch (p)
	{
	case TypePoints:					return L"Points";
	case TypeLines:						return L"Lines";
	case TypeLineStrip:					return L"LineStrip";
	case TypeLineLoop:					return L"LineLoop";
	case TypeTriangles:					return L"Triangles";
	case TypeTriangleStrip:				return L"TriangleStrip";
	case TypeTriangleFan:				return L"TriangleFan";
	case TypeLinesAdjacency:			return L"LinesAdjacency";
	case TypeLineStripAdjacency:		return L"LineStripAdjacency";
	case TypeTrianglesAdjacency:		return L"TrianglesAdjacency";
	case TypeTriangleStripAdjacency:	return L"TriangleStripAdjacency";
	}
	return L"Unknown";
}

DKPrimitive::Type DKPrimitive::StringToType(const DKString& str)
{
	if (!str.CompareNoCase(TypeToString(TypePoints)))
		return TypePoints;
	if (!str.CompareNoCase(TypeToString(TypeLines)))
		return TypeLines;
	if (!str.CompareNoCase(TypeToString(TypeLineStrip)))
		return TypeLineStrip;
	if (!str.CompareNoCase(TypeToString(TypeLineLoop)))
		return TypeLineLoop;
	if (!str.CompareNoCase(TypeToString(TypeTriangles)))
		return TypeTriangles;
	if (!str.CompareNoCase(TypeToString(TypeTriangleStrip)))
		return TypeTriangleStrip;
	if (!str.CompareNoCase(TypeToString(TypeTriangleFan)))
		return TypeTriangleFan;
	if (!str.CompareNoCase(TypeToString(TypeLinesAdjacency)))
		return TypeLinesAdjacency;
	if (!str.CompareNoCase(TypeToString(TypeLineStripAdjacency)))
		return TypeLineStripAdjacency;
	if (!str.CompareNoCase(TypeToString(TypeTrianglesAdjacency)))
		return TypeTrianglesAdjacency;
	if (!str.CompareNoCase(TypeToString(TypeTriangleStripAdjacency)))
		return TypeTriangleStripAdjacency;
	return TypeUnknown;
}

DKPrimitiveIndex::DKPrimitiveIndex(void)
	: type(DKPrimitive::TypeUnknown)
	, maxVertexIndex(0)
{

}

DKPrimitiveIndex::~DKPrimitiveIndex(void)
{

}

bool DKPrimitiveIndex::SetIndex(unsigned int* indices, size_t count, DKPrimitive::Type t)
{
	if (indices == NULL || count == 0)
		return NULL;

	switch (t)
	{
	case DKPrimitive::TypePoints:
	case DKPrimitive::TypeLines:
	case DKPrimitive::TypeLineStrip:
	case DKPrimitive::TypeLineLoop:
	case DKPrimitive::TypeTriangles:
	case DKPrimitive::TypeTriangleStrip:
	case DKPrimitive::TypeTriangleFan:
	case DKPrimitive::TypeLinesAdjacency:
	case DKPrimitive::TypeLineStripAdjacency:
	case DKPrimitive::TypeTrianglesAdjacency:
	case DKPrimitive::TypeTriangleStripAdjacency:
		break;
	default:
		return false;
	}

	maxVertexIndex = 0;

	this->indices.Add(indices, count);
	this->type = t;

	for (size_t i = 0; i < this->indices.Count(); ++i)
	{
		if (this->indices.Value(i) > maxVertexIndex)
			maxVertexIndex = this->indices.Value(i);
	}
	return true;
}

size_t DKPrimitiveIndex::MaxVertices(void) const
{
	return maxVertexIndex;
}

size_t DKPrimitiveIndex::NumberOfTriangles(void) const
{
	size_t numberOfIndices = this->indices.Count();

	switch (this->type)
	{
	case DKPrimitive::TypeTriangles:
		return numberOfIndices / 3;
		break;
	case DKPrimitive::TypeTriangleStrip:
		if (numberOfIndices >= 3)
			return numberOfIndices - 2;
		break;
	case DKPrimitive::TypeTriangleFan:
		if (numberOfIndices >= 3)
			return (numberOfIndices - 1) / 2;
		break;
	case DKPrimitive::TypeTrianglesAdjacency:
		if (numberOfIndices >= 6)
			return numberOfIndices / 6;
		break;
	case DKPrimitive::TypeTriangleStripAdjacency:
		if (numberOfIndices >= 6)
			return (numberOfIndices / 2) - 2;
		break;
	}
	return 0;
}

size_t DKPrimitiveIndex::NumberOfLines(void) const
{
	size_t numberOfIndices = this->indices.Count();

	switch (this->type)
	{
	case DKPrimitive::TypeLines:
		return numberOfIndices / 2;
		break;
	case DKPrimitive::TypeLineStrip:
		if (numberOfIndices >= 2)
			return numberOfIndices - 1;
		break;
	case DKPrimitive::TypeLineLoop:
		if (numberOfIndices >= 2)
			return numberOfIndices;
		break;
	case DKPrimitive::TypeLinesAdjacency:
		if (numberOfIndices >= 4)
			return numberOfIndices / 4;
		break;
	case DKPrimitive::TypeLineStripAdjacency:
		if (numberOfIndices >= 4)
			return numberOfIndices - 3;
		break;
	}
	return 0;
}

size_t DKPrimitiveIndex::NumberOfPoints(void) const
{
	if (this->type == DKPrimitive::TypePoints)
		return this->indices.Count();

	return 0;
}

size_t DKPrimitiveIndex::GetTriangles(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKTriangle>& output) const
{
	DKArray<DKTriangle> triangles;
	triangles.Reserve(count);

	while (count > 0)
	{
		unsigned int indices[3];
		if (GetTriangleIndices(start++, indices[0], indices[1], indices[2]))
		{
			triangles.Add(DKTriangle(positions[indices[0]], positions[indices[1]], positions[indices[2]]));			
			count--;
		}
		else
			break;
	}
	output.Add(triangles);
	return triangles.Count();
}

size_t DKPrimitiveIndex::GetLines(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKLine>& output) const
{
	DKArray<DKLine> lines;
	lines.Reserve(count);

	while (count > 0)
	{
		unsigned int indices[2];
		if (GetLineIndices(start++, indices[0], indices[1]))
		{
			lines.Add(DKLine(positions[indices[0]], positions[indices[1]]));
			count--;
		}
		else
			break;
	}
	output.Add(lines);
	return lines.Count();
}

size_t DKPrimitiveIndex::GetPoints(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKVector3>& output) const
{
	DKArray<DKVector3> points;
	points.Reserve(count);

	while (count > 0)
	{
		unsigned int index;
		if (GetPointIndex(start++, index))
		{
			points.Add(positions[index]);
			count--;
		}
		else
			break;
	}
	output.Add(points);
	return points.Count();
}

bool DKPrimitiveIndex::GetTriangleIndices(unsigned int triangle, unsigned int& index1, unsigned int& index2, unsigned int& index3) const
{
	if (indices.Count() < 3)
		return false;

	switch (type)
	{
	case DKPrimitive::TypeTriangles:
		if (indices.Count() > triangle * 3 + 2)
		{
			index1 = indices.Value(triangle * 3);
			index2 = indices.Value(triangle * 3 + 1);
			index3 = indices.Value(triangle * 3 + 2);
			return true;
		}
		break;
	case DKPrimitive::TypeTriangleStrip:
		if (indices.Count() > triangle + 2)
		{
			if (triangle % 2 == 0)
			{
				index2 = indices.Value(triangle);
				index1 = indices.Value(triangle + 1);
			}
			else
			{
				index1 = indices.Value(triangle);
				index2 = indices.Value(triangle + 1);
			}
			index3 = indices.Value(triangle + 2);
			return true;
		}
		break;
	case DKPrimitive::TypeTriangleFan:
		if (indices.Count() > triangle + 2)
		{
			index1 = indices.Value(0);
			index2 = indices.Value(triangle + 1);
			index3 = indices.Value(triangle + 2);
			return true;
		}
		break;
	case DKPrimitive::TypeTrianglesAdjacency:
	case DKPrimitive::TypeTriangleStripAdjacency:
		{
			unsigned int adj[3];
			return GetTriangleAdjacencyIndices(triangle, index1, index2, index3, adj[0], adj[1], adj[2]);
		}
		break;
	}
	return false;
}

bool DKPrimitiveIndex::GetTriangleAdjacencyIndices(unsigned int triangle, unsigned int& index1, unsigned int& index2, unsigned int& index3, unsigned int& adj12, unsigned int& adj23, unsigned int& adj31) const
{
	if (indices.Count() < 6)
		return false;

	switch (type)
	{
	case DKPrimitive::TypeTrianglesAdjacency:
		if (indices.Count() > triangle * 6 + 5)
		{
			// 0,2,4 : triangle
			// 1,3,5 : adjacency
			index1 = indices.Value(triangle * 6 + 0);
			index2 = indices.Value(triangle * 6 + 2);
			index3 = indices.Value(triangle * 6 + 4);
			adj12 = indices.Value(triangle * 6 + 1);
			adj23 = indices.Value(triangle * 6 + 3);
			adj31 = indices.Value(triangle * 6 + 5);
			return true;
		}
		break;
	case DKPrimitive::TypeTriangleStripAdjacency:
		if (indices.Count() > triangle + 2)
		{
			if (triangle == 0)			// first triangle
			{
				index1 = indices.Value(0);
				index2 = indices.Value(2);
				index3 = indices.Value(4);
				if (indices.Count() == 3)  // one triangle
				{
					adj12 = indices.Value(1);
					adj23 = indices.Value(5);
					adj31 = indices.Value(3);
				}
				else
				{
					adj12 = indices.Value(1);
					adj23 = indices.Value(6);
					adj31 = indices.Value(3);
				}
			}
			else
			{
				if (triangle % 2 == 0) // even number of triangle index
				{
					index1 = indices.Value(triangle * 2);
					index2 = indices.Value(triangle * 2 + 2);
					index3 = indices.Value(triangle * 2 + 4);
				}
				else                   // odd number of triangle index
				{
					index1 = indices.Value(triangle * 2 + 2);
					index2 = indices.Value(triangle * 2);
					index3 = indices.Value(triangle * 2 + 4);
				}
				if (indices.Count() == triangle + 3) // last triangle
				{
					if (triangle % 2 == 0) // even
					{
						adj12 = indices.Value(triangle * 2 - 2);
						adj23 = indices.Value(triangle * 2 + 5);
						adj31 = indices.Value(triangle * 2 + 3);
					}
					else                   // odd
					{
						adj12 = indices.Value(triangle * 2 - 2);
						adj23 = indices.Value(triangle * 2 + 3);
						adj31 = indices.Value(triangle * 2 + 5);
					}
				}
				else // other triangles
				{
					if (triangle % 2 == 0) // even
					{
						adj12 = indices.Value(triangle * 2 - 2);
						adj23 = indices.Value(triangle * 2 + 6);
						adj31 = indices.Value(triangle * 2 + 3);
					}
					else                   // odd
					{
						adj12 = indices.Value(triangle * 2 - 2);
						adj23 = indices.Value(triangle * 2 + 3);
						adj31 = indices.Value(triangle * 2 + 6);
					}
				}
			}
			return true;
		}
		break;
	}
	return false;
}

bool DKPrimitiveIndex::GetLineIndices(unsigned int line, unsigned int& index1, unsigned int& index2) const
{
	if (indices.Count() < 2)
		return false;

	switch (type)
	{
	case DKPrimitive::TypeLines:
		if (indices.Count() > line * 2)
		{
			index1 = indices.Value(line * 2);
			index2 = indices.Value(line * 2 + 1);
			return true;
		}
		break;
	case DKPrimitive::TypeLineStrip:
		if (indices.Count() > line + 1)
		{
			index1 = indices.Value(line);
			index2 = indices.Value(line + 1);
			return true;
		}
		break;
	case DKPrimitive::TypeLineLoop:
		if (indices.Count() >= line)
		{
			if (indices.Count() == line) // last loop, connected to begin
			{
				index1 = indices.Value(indices.Count() - 1);
				index2 = indices.Value(0);
			}
			else
			{
				index1 = indices.Value(line);
				index2 = indices.Value(line + 1);
			}
			return true;
		}
		break;
	case DKPrimitive::TypeLinesAdjacency:
	case DKPrimitive::TypeLineStripAdjacency:
		{
			unsigned int adj[2];
			return GetLineAdjacencyIndices(line, index1, index2, adj[0], adj[1]);
		}
		break;
	}
	return false;
}

bool DKPrimitiveIndex::GetLineAdjacencyIndices(unsigned int line, unsigned int& index1, unsigned int& index2, unsigned int& adj1, unsigned int& adj2) const
{
	if (indices.Count() < 4)
		return false;

	switch (type)
	{
	case DKPrimitive::TypeLinesAdjacency:
		if (indices.Count() > line * 4 + 3)
		{
			adj1 = indices.Value(line * 4);
			index1 = indices.Value(line * 4 + 1);
			index2 = indices.Value(line * 4 + 2);
			adj2 = indices.Value(line * 4 + 3);
			return true;
		}
		break;
	case DKPrimitive::TypeLineStripAdjacency:
		if (indices.Count() > line + 3)
		{
			adj1 = indices.Value(line);
			index1 = indices.Value(line + 1);
			index2 = indices.Value(line + 2);
			adj2 = indices.Value(line + 3);
			return true;
		}
		break;
	}
	return false;
}

bool DKPrimitiveIndex::GetPointIndex(unsigned int point, unsigned int& index) const
{
	if (type != DKPrimitive::TypePoints)
		return false;

	if (indices.Count() > index)
	{
		index = indices[index];
		return true;
	}

	return false;
}

bool DKPrimitiveIndex::IsTriangleBased(void) const
{
	switch (type)
	{
	case DKPrimitive::TypeTriangles:
	case DKPrimitive::TypeTriangleStrip:
	case DKPrimitive::TypeTriangleFan:
	case DKPrimitive::TypeTrianglesAdjacency:
	case DKPrimitive::TypeTriangleStripAdjacency:
		return true;
		break;
	}
	return false;
}

bool DKPrimitiveIndex::IsLineBased(void) const
{
	switch (type)
	{
	case DKPrimitive::TypeLines:
	case DKPrimitive::TypeLineStrip:
	case DKPrimitive::TypeLineLoop:
	case DKPrimitive::TypeLinesAdjacency:
	case DKPrimitive::TypeLineStripAdjacency:
		return true;
		break;
	}
	return false;
}

bool DKPrimitiveIndex::HasAdjacency(void) const
{
	switch (type)
	{
	case DKPrimitive::TypeLinesAdjacency:
	case DKPrimitive::TypeLineStripAdjacency:
	case DKPrimitive::TypeTrianglesAdjacency:
	case DKPrimitive::TypeTriangleStripAdjacency:
		return true;
		break;
	}
	return false;
}
