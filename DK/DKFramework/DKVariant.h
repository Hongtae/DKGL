//
//  File: DKVariant.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVector2.h"
#include "DKVector3.h"
#include "DKVector4.h"
#include "DKMatrix2.h"
#include "DKMatrix3.h"
#include "DKMatrix4.h"
#include "DKQuaternion.h"

#pragma pack(push, 4)
namespace DKFramework
{
	/// @brief a variant (value) class.
	/// @details
	/// an object can have following types.
	///   - Integer (int64)
	///   - Float (double)
	///   - Vector2 (DKVector2)
	///   - Vector3 (DKVector3)
	///   - Vector4 (DKVector4)
	///   - Matrix2 (DKMatrix2)
	///   - Matrix3 (DKMatrix3)
	///   - Matrix4 (DKMatrix4)
	///   - Quaternion (DKQuaternion)
	///   - Rational (DKRational)
	///   - String (DKString)
	///   - DateTime (DKDateTime)
	///   - Data (DKBuffer)
	///   - StructuredData (Data with structure layout)
	///   - Array (DKArray<DKVariant>)
	///   - Pairs (DKMap<DKString, DKVariant>)
	///   - Undefined (no value, initial type)
	///
	/// DKVariant object can be serialized with XML or binary.
	class DKGL_API DKVariant
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
			TypeStructData	= 'stdt',
			TypeArray		= 'arry',
			TypePairs		= 'pair',
		};

		typedef int64_t VInteger;
		typedef double VFloat;
		typedef DKVector2 VVector2;
		typedef DKVector3 VVector3;
		typedef DKVector4 VVector4;
		typedef DKMatrix2 VMatrix2;
		typedef DKMatrix3 VMatrix3;
		typedef DKMatrix4 VMatrix4;
		typedef DKQuaternion VQuaternion;
		typedef DKRational VRational;
		typedef DKString VString;
		typedef DKDateTime VDateTime;
		typedef DKBuffer VData;
		typedef DKArray<DKVariant> VArray;
		typedef DKMap<VString, DKVariant> VPairs;

		/// structured data layout.
		/// Arithmetic types are byte-ordered automatically.
		/// Bypass (Raw) types are not byte-ordered. (treat as 'char*' buffer)
		enum class StructElem : uint8_t
		{
			Arithmetic1 = 0x01,	///< int8_t, uint8_t
			Arithmetic2 = 0x02,	///< int16_t, uint16_t
			Arithmetic4 = 0x04,	///< int32_t, uint32_t, float
			Arithmetic8 = 0x08,	///< int64_t, uint64_t, double

			Bypass1 = 0xff,		///< 1 byte item, (same as Arithmetic1)
			Bypass2 = 0xfe,		///< 2 bytes item
			Bypass4 = 0xfc,		///< 4 bytes item
			Bypass8 = 0xf8,		///< 8 bytes item
		};
		/// automatic-byte-order matching data.
		/// layout should be set properly. (describe item layout without padding)
		struct VStructuredData
		{
			DKBuffer data;	///< buffer object
			size_t elementSize;			///< element size (include alignment padding)
			DKArray<StructElem> layout; ///< structured-element layout
		};

		DKVariant(Type t = TypeUndefined);

		DKVariant(const DKUniChar8* v);
		DKVariant(const DKUniCharW* v);
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
		DKVariant(const VStructuredData& v);
		DKVariant(const VArray& v);
		DKVariant(const VPairs& v);
		DKVariant(const DKVariant& v);
		DKVariant(DKVariant&& v);

		explicit DKVariant(const DKXmlElement* e);
		~DKVariant(void);

		DKVariant& SetValueType(Type t);
		Type ValueType(void) const;

		DKObject<DKXmlElement> ExportXML(void) const; ///< generate XML
		bool ImportXML(const DKXmlElement* e); ///< import from XML

		bool ExportStream(DKStream* stream) const; ///< generate binary data
		bool ImportStream(DKStream* stream); ///< import from binary data

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
		DKVariant& SetStructuredData(const VStructuredData& v);
		DKVariant& SetStructuredData(const void* p, size_t elementSize, size_t count, std::initializer_list<StructElem> layout);
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
		VStructuredData& StructuredData(void);
		const VStructuredData& StructuredData(void) const;
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
#pragma pack(pop)
