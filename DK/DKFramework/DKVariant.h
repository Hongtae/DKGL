//
//  File: DKVariant.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

////////////////////////////////////////////////////////////////////////////////
// DKVariant
// a variant class.
// an object can have following types.
//   - Integer (int64)
//   - Float (double)
//   - Vector2 (DKVector2)
//   - Vector3 (DKVector3)
//   - Vector4 (DKVector4)
//   - Matrix2 (DKMatrix2)
//   - Matrix3 (DKMatrix3)
//   - Matrix4 (DKMatrix4)
//   - Quaternion (DKQuaternion)
//   - Rational (DKRational)
//   - String (DKString)
//   - DateTime (DKDateTime)
//   - Data (DKBuffer)
//   - Array (DKArray<DKVariant>)
//   - Pairs (DKMap<DKString, DKVariant>)
//   - Undefined (no value, initial type)
//
// DKVariant object can be serialized with XML or binary.
////////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKLIB_API DKVariant
	{
	public:
		enum Type : int
		{
			TypeUndefined	= 0,
			TypeInteger		= 'intg',
			TypeFloat		= 'flot',
			TypeVector2		= 'vec2',
			TypeVector3		= 'vec3',
			TypeVector4		= 'vec4',
			TypeMatrix2		= 'mat2',
			TypeMatrix3		= 'mat3',
			TypeMatrix4		= 'mat4',
			TypeQuaternion	= 'quat',
			TypeRational	= 'ratl',
			TypeString		= 'strn',
			TypeDateTime	= 'time',
			TypeData		= 'data',
			TypeArray		= 'arry',
			TypePairs		= 'pair',
		};
		typedef long long VInteger;
		typedef double VFloat;
		typedef DKVector2 VVector2;
		typedef DKVector3 VVector3;
		typedef DKVector4 VVector4;
		typedef DKMatrix2 VMatrix2;
		typedef DKMatrix3 VMatrix3;
		typedef DKMatrix4 VMatrix4;
		typedef DKQuaternion VQuaternion;
		typedef DKFoundation::DKRational VRational;
		typedef DKFoundation::DKString VString;
		typedef DKFoundation::DKDateTime VDateTime;
		typedef DKFoundation::DKBuffer VData;
		typedef DKFoundation::DKArray<DKVariant> VArray;
		typedef DKFoundation::DKMap<VString, DKVariant> VPairs;

		DKVariant(Type t = TypeUndefined);

		DKVariant(const DKFoundation::DKUniChar8* v);
		DKVariant(const DKFoundation::DKUniCharW* v);
		DKVariant(const VInteger& v);
		DKVariant(const VFloat& v);
		DKVariant(const VVector2& v);
		DKVariant(const VVector3& v);
		DKVariant(const VVector4& v);
		DKVariant(const VMatrix2& v);
		DKVariant(const VMatrix3& v);
		DKVariant(const VMatrix4& v);
		DKVariant(const VQuaternion& v);
		DKVariant(const VRational& v);
		DKVariant(const VString& v);
		DKVariant(const VDateTime& v);
		DKVariant(const VData& v);
		DKVariant(const VArray& v);
		DKVariant(const VPairs& v);
		DKVariant(const DKVariant& v);
		DKVariant(DKVariant&& v);

		explicit DKVariant(const DKFoundation::DKXMLElement* e);
		~DKVariant(void);

		DKVariant& SetValueType(Type t);
		Type ValueType(void) const;

		// XML input/output
		DKFoundation::DKObject<DKFoundation::DKXMLElement> ExportXML(void) const;
		bool ImportXML(const DKFoundation::DKXMLElement* e);

		// Binary-stream input/output
		bool ExportStream(DKFoundation::DKStream* stream) const;
		bool ImportStream(DKFoundation::DKStream* stream);

		DKVariant& SetInteger(const VInteger& v);
		DKVariant& SetFloat(const VFloat& v);
		DKVariant& SetVector2(const VVector2& v);
		DKVariant& SetVector3(const VVector3& v);
		DKVariant& SetVector4(const VVector4& v);
		DKVariant& SetMatrix2(const VMatrix2& v);
		DKVariant& SetMatrix3(const VMatrix3& v);
		DKVariant& SetMatrix4(const VMatrix4& v);
		DKVariant& SetQuaternion(const VQuaternion& v);
		DKVariant& SetRational(const VRational& v);
		DKVariant& SetString(const VString& v);
		DKVariant& SetDateTime(const VDateTime& v);
		DKVariant& SetData(const VData& v);
		DKVariant& SetData(const void* p, size_t s);
		DKVariant& SetArray(const VArray& v);
		DKVariant& SetPairs(const VPairs& v);
		DKVariant& SetValue(const DKVariant& v);
		DKVariant& SetValue(DKVariant&& v);

		DKVariant& operator = (const DKVariant& v);
		DKVariant& operator = (DKVariant&& v);

		VInteger& Integer(void);
		const VInteger& Integer(void) const;
		VFloat& Float(void);
		const VFloat& Float(void) const;
		VVector2& Vector2(void);
		const VVector2& Vector2(void) const;
		VVector3& Vector3(void);
		const VVector3& Vector3(void) const;
		VVector4& Vector4(void);
		const VVector4& Vector4(void) const;
		VMatrix2& Matrix2(void);
		const VMatrix2& Matrix2(void) const;
		VMatrix3& Matrix3(void);
		const VMatrix3& Matrix3(void) const;
		VMatrix4& Matrix4(void);
		const VMatrix4& Matrix4(void) const;
		VQuaternion& Quaternion(void);
		const VQuaternion& Quaternion(void) const;
		VRational& Rational(void);
		const VRational& Rational(void) const;
		VString& String(void);
		const VString& String(void) const;
		VDateTime& DateTime(void);
		const VDateTime& DateTime(void) const;
		VData& Data(void);
		const VData& Data(void) const;
		VArray& Array(void);
		const VArray& Array(void) const;
		VPairs& Pairs(void);
		const VPairs& Pairs(void) const;

		bool IsEqual(const DKVariant& v) const;

	private:
		// data block size chosen from biggest value of minSize, sizeof(void*)
		template <int minSize> struct VBlock
		{
			enum {Size = (sizeof(void*) > minSize) ? sizeof(void*) : minSize};
		};
		unsigned char vblock[VBlock<16>::Size]; // minSize is 16, vblock will be greater or equal to 16.
		Type valueType;
	};
}
