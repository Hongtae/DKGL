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
	///   - RationalNumber (DKRationalNumber)
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
			TypeUndefined		= 0,
			TypeInteger			= 'intg',
			TypeFloat			= 'flot',
			TypeVector2			= 'vec2',
			TypeVector3			= 'vec3',
			TypeVector4			= 'vec4',
			TypeMatrix2			= 'mat2',
			TypeMatrix3			= 'mat3',
			TypeMatrix4			= 'mat4',
			TypeQuaternion		= 'quat',
			TypeRationalNumber	= 'ratl',
			TypeString			= 'strn',
			TypeDateTime		= 'time',
			TypeData			= 'data',
			TypeStructData		= 'stdt',
			TypeArray			= 'arry',
			TypePairs			= 'pair',
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
		typedef DKRationalNumber VRationalNumber;
		typedef DKString VString;
		typedef DKDateTime VDateTime;
		typedef DKData VData;
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
			DKObject<DKData> data;		///< data object
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
		DKVariant(const VRationalNumber& v);
		DKVariant(const VString& v);
		DKVariant(const VDateTime& v);
		DKVariant(const VData& v);
		DKVariant(const VStructuredData& v);
		DKVariant(const VArray& v);
		DKVariant(const VPairs& v);
		DKVariant(const DKVariant& v);
		DKVariant(DKVariant&& v);

		explicit DKVariant(const DKXmlElement* e);
		~DKVariant();

		DKVariant& SetValueType(Type t);
		Type ValueType() const;

		DKObject<DKXmlElement> ExportXML() const; ///< generate XML
		bool ImportXML(const DKXmlElement* e); ///< import from XML

		bool ExportStream(DKStream* stream, DKByteOrder byteOrder = DKByteOrder::Unknown) const; ///< generate binary data
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
		DKVariant& SetRationalNumber(const VRationalNumber& v);
		DKVariant& SetString(const VString& v);
		DKVariant& SetDateTime(const VDateTime& v);
		DKVariant& SetData(const VData& v);
		DKVariant& SetData(const VData* v);
		DKVariant& SetData(const void* p, size_t s);
		DKVariant& SetStructuredData(const VStructuredData& v);
		DKVariant& SetStructuredData(const void* p, size_t elementSize, size_t count, std::initializer_list<StructElem> layout);
		DKVariant& SetArray(const VArray& v);
		DKVariant& SetPairs(const VPairs& v);
		DKVariant& SetValue(const DKVariant& v);
		DKVariant& SetValue(DKVariant&& v);

		DKVariant& operator = (const DKVariant& v);
		DKVariant& operator = (DKVariant&& v);

		VInteger& Integer();
		const VInteger& Integer() const;
		VFloat& Float();
		const VFloat& Float() const;
		VVector2& Vector2();
		const VVector2& Vector2() const;
		VVector3& Vector3();
		const VVector3& Vector3() const;
		VVector4& Vector4();
		const VVector4& Vector4() const;
		VMatrix2& Matrix2();
		const VMatrix2& Matrix2() const;
		VMatrix3& Matrix3();
		const VMatrix3& Matrix3() const;
		VMatrix4& Matrix4();
		const VMatrix4& Matrix4() const;
		VQuaternion& Quaternion();
		const VQuaternion& Quaternion() const;
		VRationalNumber& RationalNumber();
		const VRationalNumber& RationalNumber() const;
		VString& String();
		const VString& String() const;
		VDateTime& DateTime();
		const VDateTime& DateTime() const;
		VData& Data();
		const VData& Data() const;
		VStructuredData& StructuredData();
		const VStructuredData& StructuredData() const;
		VArray& Array();
		const VArray& Array() const;
		VPairs& Pairs();
		const VPairs& Pairs() const;

		bool IsEqual(const DKVariant& v) const;

		using KeyPathEnumerator = DKFunctionSignature<bool (DKVariant&)>;
		using ConstKeyPathEnumerator = DKFunctionSignature<bool (const DKVariant&)>;

		/// @brief Find descendants with key-path
		/// A parent node object must be DKVariant::TypePair type and key-Path
		/// is dot-notation of ancestors-descendants relationship.
		///  ex: key-path: "Group.Department.Name"
		/// @note
		///  For the same path, the object with the shortest depth is first called with the callback function.
		/// @param callback A callback function that is invoked when a matching object is found. If this callback function returns false, it continues to search for another matching object.
		bool FindObjectAtKeyPath(const DKString& path, KeyPathEnumerator* callback);
		bool FindObjectAtKeyPath(const DKString& path, ConstKeyPathEnumerator* callback) const;

		/// Insert new value at given key-path.
		/// If this function succeeded, FindObjectAtKeyPath() is able to locate new value.
		/// This function will create intermediate pairs (TypePairs) as a container.
		/// @return new value's address, NULL if item cannot be created with given path.
		DKVariant* NewValueAtKeyPath(const DKString& path, const DKVariant& value);
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
