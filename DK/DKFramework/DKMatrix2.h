//
//  File: DKMatrix2.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"

#pragma pack(push, 4)
namespace DKFramework
{
	class DKVector2;
	/// @brief 2x2 matrix
	///
	/// @note
	///   This matrix order is Row-major.
	///   transform of Vector2 V is V' = V * Matrix
	class DKGL_API DKMatrix2
	{
	public:
		DKMatrix2();
		DKMatrix2(const DKVector2& row1, const DKVector2& row2);
		DKMatrix2(float e11, float e12, float e21, float e22);

		DKMatrix2& Zero();
		DKMatrix2& Identity();
		bool IsIdentity() const;
		bool IsDiagonal() const;

		DKMatrix2& Inverse();
		DKMatrix2& Transpose();
		DKMatrix2& Multiply(const DKMatrix2& m);

		float Determinant() const;
		bool GetInverseMatrix(DKMatrix2& matOut, float *pDeterminant) const;

		DKMatrix2 operator * (const DKMatrix2& m) const;
		DKMatrix2 operator + (const DKMatrix2& m) const;
		DKMatrix2 operator - (const DKMatrix2& m) const;
		DKMatrix2 operator * (float f) const;
		DKMatrix2 operator / (float f) const;

		DKMatrix2& operator *= (const DKMatrix2& m);
		DKMatrix2& operator += (const DKMatrix2& m);
		DKMatrix2& operator -= (const DKMatrix2& m);
		DKMatrix2& operator *= (float f);
		DKMatrix2& operator /= (float f);

		bool operator == (const DKMatrix2& m) const;
		bool operator != (const DKMatrix2& m) const;

		DKVector2 Row1() const;
		DKVector2 Row2() const;
		DKVector2 Column1() const;
		DKVector2 Column2() const;

		union
		{
			struct
			{
				float _11, _12;
				float _21, _22;
			};
			struct
			{
				float m[2][2];
			};
			float val[4];
		};

		static const DKMatrix2 identity;
	};
}
#pragma pack(pop)
