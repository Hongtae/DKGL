//
//  File: DKPrimitiveIndex.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKTriangle.h"
#include "DKLine.h"
#include "DKVector3.h"

////////////////////////////////////////////////////////////////////////////////
// DKPrimitiveIndex
// calcualte primitive vertex order with index.
//
// Types of Primitives. (with short description)
//
// points (TypePoints):
//    each vertex become a point.
//    no performance benefits by using index-buffer with type primitive.
//
// lines (TypeLines):
//    each two vertices becomes line segment. (0,1)(2,3)(4,5)...
//    number of vertices should be greater or equal to 2.
//
// line strip (TypeLineStrip):
//    first two vertices becomes line segment,
//    next vertex connected to previous vertex to be line segment.
//    (0,1)(1,2)(2,3)(3,4)...
//    number of vertices should be greater or equal to 2.
//
// line loop (TypeLineLoop):
//    similar to line strip, but last vertex connects to first.
//    this primitive made closed line segments always.
//    (0,1)(1,2)(2,3)(3,4)... (n,0)
//    number of vertices should be greater or equal to 2.
//
// triangles (TypeTriangles):
//    each three vertices make triangle. (0,1,2)(3,4,5)(6,7,8)...
//    number of vertices should be greater or equal to 3.
//
// triangle strip (TypeTriangleStrip):
//    first three vertices make triangle and make next triangle by
//    second, third vertex of previous triangle with next vertex.
//    triangle index are different to even, odd sequence.
//    for even number of sequence: (n, n+1, n+2)
//    for odd number of sequence: (n+1, n, n+2)
//      ex: (0,1,2)(2,1,3)(2,3,4)(4,3,5)(4,5,6)(6,5,7)...
//    number of vertices should be greater or equal to 3.
//
// triangle fan (TypeTriangleFan):
//    first and two vertices of the rest make triangle.
//      (0,1,2)(0,2,3)(0,3,4)(0,4,5)...
//    number of vertices should be greater or equal to 3.
//
// lines adjacency (TypeLinesAdjacency):
//    similar to lines, but each line segment's point has adjacency.
//    adjacency used by geometry shader to control line segment.
//     -       lines: (1,2)(5,6)(9,10)...
//     - adjacencies: (0,3)(4,7)(8,11)...
//    number of vertices should be greater or equal to 4.
//    one line per 4 vertices. (adj-begin-end-adj)
//
// line strip adjacency (TypeLineStripAdjacency):
//    similar to line strip, first line segment begin with second vertex,
//    (first vertex is adjacency) and have same sequence as line strip except
//    last vertex. last vertex is adjacency.
//    (adj-line-line-...-line-adj)
//    number of vertices should be greater or equal to 4,
//    number of indices should be number of vertices + 3.
//    total output lines would be number of vertices - 2.
//
// triangles adjacency (TypeTrianglesAdjacency):
//    similar to triangles, but each vertex has adjacency.
//    triangle vertices is made up of (n, n+2, n+4) for n'th sequences,
//    and (n+1, n+3, n+4) is adjacency, used by geometry shader.
//      triangles:(0,2,4)(6,8,10)(12,14,16)...
//    adjacencies:(1,3,5)(7,9,11)(13,15,17)...
//    adjacency of each line-segment of triangle is below.
//      adjacency for line segment (n, n+2) is n+1.
//      adjacency for line segment (n+2, n+4) is n+3.
//      adjacency for line segment (n+4, n) is n+5.
//    number of vertices should be greater or equal to 6.
//
// triangle strip adjacency (TypeTriangleStripAdjacency):
//    similar to triangle strip, but each line segment of triangle has
//    adjacency. indices of single triangle and multiple triangles are
//    different.
//    for single triangle,
//       triangle: (0,2,4), adjacencies: (1,5,3).
//    for more than two triangles,
//       first-triangle: (0,2,4), adjacencies: (1,5,3)
//       in middle of sequences, (with n > 0 and n < number of triangles - 1)
//       triangle: (2n+2, 2n, 2n+4), adjacencies: (2n-2, 2n+3, 2n+6), if n is odd.
//       triangle: (2n, 2n+2, 2n+4), adjacencies: (2n-2, 2n+6, 2n+3), if n is even.
//       and last triangle is,
//       triangle: (2n+2, 2n, 2n+4), adjacencies: (2n-2, 2n+3, 2n+5), if n is odd.
//       triangle: (2n, 2n+2, 2n+4), adjacencies: (2n-2, 2n+5, 2n+3), if n is even.
//    number of vertices should be greater or equal to 6,
//    and it should be 2(n+2) where n is number of triangles.
//
// Note:
//   Adjacency type will be ignored unless geometry shader used.
//   (OpenGL ES not supported)
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	struct DKPrimitive
	{
		enum Type
		{
			TypeUnknown = 0,
			TypePoints,
			TypeLines,
			TypeLineStrip,
			TypeLineLoop,
			TypeTriangles,
			TypeTriangleStrip,
			TypeTriangleFan,
			TypeLinesAdjacency,
			TypeLineStripAdjacency,
			TypeTrianglesAdjacency,
			TypeTriangleStripAdjacency,
		};
		static DKFoundation::DKString TypeToString(Type p);
		static Type StringToType(const DKFoundation::DKString& str);
	};

	class DKLIB_API DKPrimitiveIndex
	{
	public:
		DKPrimitiveIndex(void);
		~DKPrimitiveIndex(void);

		bool SetIndex(unsigned int* indices, size_t count, DKPrimitive::Type t);

		size_t NumberOfTriangles(void) const;
		size_t NumberOfLines(void) const;
		size_t NumberOfPoints(void) const;
		size_t MaxVertices(void) const;

		bool GetTriangleIndices(unsigned int triangle, unsigned int& index1, unsigned int& index2, unsigned int& index3) const;
		bool GetTriangleAdjacencyIndices(unsigned int triangle, unsigned int& index1, unsigned int& index2, unsigned int& index3, unsigned int& adj12, unsigned int& adj23, unsigned int& adj31) const;
		bool GetLineIndices(unsigned int line, unsigned int& index1, unsigned int& index2) const;
		bool GetLineAdjacencyIndices(unsigned int line, unsigned int& index1, unsigned int& index2, unsigned int& adj1, unsigned int& adj2) const;
		bool GetPointIndex(unsigned int point, unsigned int& index) const;

		bool IsTriangleBased(void) const;
		bool IsLineBased(void) const;
		bool HasAdjacency(void) const;

		DKPrimitive::Type GetType(void) const		{return type;}
		size_t GetIndexCount(void) const			{return indices.Count();}

		size_t GetTriangles(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKTriangle>& output) const;
		size_t GetLines(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKLine>& output) const;
		size_t GetPoints(const DKVector3* positions, size_t start, size_t count, DKFoundation::DKArray<DKVector3>& output) const;
	private:
		DKFoundation::DKArray<unsigned int>	indices;
		DKPrimitive::Type					type;
		size_t								maxVertexIndex;
	};
}
