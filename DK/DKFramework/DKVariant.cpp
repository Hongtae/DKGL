//
//  File: DKVariant.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
//

#include "DKVariant.h"

using namespace DKFoundation;

namespace DKFramework
{
	namespace Private
	{
		namespace
		{
			// VariantBlock, if type T is greater than maxSize, allocate buffer.
			// maxSize should be greater or equal to sizeof(void*)
			template <typename T, int maxSize, bool b = (sizeof(T) > maxSize), bool valid = (maxSize >= sizeof(void*))> struct VariantBlock;
			template <typename T, int maxSize> struct VariantBlock<T, maxSize, false, true>
			{
				static T* Alloc(void* p)
				{
					return new (reinterpret_cast<T*>(p)) T();
				}
				static void Dealloc(void* p)
				{
					reinterpret_cast<T*>(p)->~T();
				}
				static T& Value(void* p)
				{
					return reinterpret_cast<T*>(p)[0];
				}
			};
			template <typename T, int maxSize> struct VariantBlock<T, maxSize, true, true>
			{
				static T* Alloc(void* p)
				{
					T** pptr = reinterpret_cast<T**>(p);
					pptr[0] = new (DKMemoryDefaultAllocator::Alloc(sizeof(T))) T();
					return pptr[0];
				}
				static void Dealloc(void* p)
				{
					T** pptr = reinterpret_cast<T**>(p);
					pptr[0]->~T();
					DKMemoryDefaultAllocator::Free(pptr[0]);
				}
				static T& Value(void* p)
				{
					return *reinterpret_cast<T**>(p)[0];
				}
			};
		}
	}
}

using namespace DKFramework;


DKVariant::DKVariant(Type t)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetValueType(t);
}

DKVariant::DKVariant(const DKUniChar8* v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetString(v);
}

DKVariant::DKVariant(const DKUniCharW* v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetString(v);
}

DKVariant::DKVariant(const VInteger& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetInteger(v);
}

DKVariant::DKVariant(const VFloat& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetFloat(v);
}

DKVariant::DKVariant(const VVector2& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetVector2(v);
}

DKVariant::DKVariant(const VVector3& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetVector3(v);
}

DKVariant::DKVariant(const VVector4& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetVector4(v);
}

DKVariant::DKVariant(const VMatrix2& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetMatrix2(v);
}

DKVariant::DKVariant(const VMatrix3& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetMatrix3(v);
}

DKVariant::DKVariant(const VMatrix4& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetMatrix4(v);
}

DKVariant::DKVariant(const VQuaternion& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetQuaternion(v);
}

DKVariant::DKVariant(const VRational& v)
	: valueType(TypeRational)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetRational(v);
}

DKVariant::DKVariant(const VString& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetString(v);
}

DKVariant::DKVariant(const VDateTime& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetDateTime(v);
}

DKVariant::DKVariant(const VData& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetData(v);
}

DKVariant::DKVariant(const VArray& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetArray(v);
}

DKVariant::DKVariant(const VPairs& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetPairs(v);
}

DKVariant::DKVariant(const DKVariant& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetValue(v);
}

DKVariant::DKVariant(DKVariant&& v)
	: valueType(TypeUndefined)
{
	valueType = v.valueType;
	memcpy(vblock, v.vblock, sizeof(vblock));
	memset(v.vblock, 0, sizeof(v.vblock));
	v.valueType = TypeUndefined;
}

DKVariant::DKVariant(const DKXMLElement* e)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->ImportXML(e);
}

DKVariant::~DKVariant(void)
{
	SetValueType(TypeUndefined);
}

DKVariant& DKVariant::SetValueType(Type t)
{
	if (valueType == t)
		return *this;

	switch (valueType)
	{
	case TypeInteger:
		Private::VariantBlock<VInteger, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeFloat:
		Private::VariantBlock<VFloat, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector2:
		Private::VariantBlock<VVector2, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector3:
		Private::VariantBlock<VVector3, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector4:
		Private::VariantBlock<VVector4, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix2:
		Private::VariantBlock<VMatrix2, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix3:
		Private::VariantBlock<VMatrix3, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix4:
		Private::VariantBlock<VMatrix4, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeQuaternion:
		Private::VariantBlock<VQuaternion, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeRational:
		Private::VariantBlock<VRational, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeString:
		Private::VariantBlock<VString, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeData:
		Private::VariantBlock<VData, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeDateTime:
		Private::VariantBlock<VDateTime, sizeof(vblock)>::Dealloc(vblock);
		break;	
	case TypeArray:
		Private::VariantBlock<VArray, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypePairs:
		Private::VariantBlock<VPairs, sizeof(vblock)>::Dealloc(vblock);
		break;
	}
	memset(vblock, 0, sizeof(vblock));

	valueType = t;
	switch (valueType)
	{
	case TypeInteger:
		Private::VariantBlock<VInteger, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeFloat:
		Private::VariantBlock<VFloat, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector2:
		Private::VariantBlock<VVector2, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector3:
		Private::VariantBlock<VVector3, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector4:
		Private::VariantBlock<VVector4, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix2:
		Private::VariantBlock<VMatrix2, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix3:
		Private::VariantBlock<VMatrix3, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix4:
		Private::VariantBlock<VMatrix4, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeQuaternion:
		Private::VariantBlock<VQuaternion, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeRational:
		Private::VariantBlock<VRational, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeString:
		Private::VariantBlock<VString, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeData:
		Private::VariantBlock<VData, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeDateTime:
		Private::VariantBlock<VDateTime, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeArray:
		Private::VariantBlock<VArray, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypePairs:
		Private::VariantBlock<VPairs, sizeof(vblock)>::Alloc(vblock);
		break;
	}
	return *this;
}

DKVariant::Type DKVariant::ValueType(void) const
{
	return valueType;
}

#define DKVARIANT_XML_ELEMENT	L"DKVariant"

DKObject<DKXMLElement> DKVariant::ExportXML(void) const
{
	DKObject<DKXMLElement> e = DKObject<DKXMLElement>::New();
	e->name = DKVARIANT_XML_ELEMENT;
	DKXMLAttribute attrType;
	attrType.name = L"type";

	switch (valueType)
	{
	case TypeInteger:			attrType.value = L"integer";		break;
	case TypeFloat:				attrType.value = L"float";			break;
	case TypeVector2:			attrType.value = L"vector2";		break;
	case TypeVector3:			attrType.value = L"vector3";		break;
	case TypeVector4:			attrType.value = L"vector4";		break;
	case TypeMatrix2:			attrType.value = L"matrix2";		break;
	case TypeMatrix3:			attrType.value = L"matrix3";		break;
	case TypeMatrix4:			attrType.value = L"matrix4";		break;
	case TypeQuaternion:		attrType.value = L"quaternion";		break;
	case TypeRational:			attrType.value = L"rational";		break;
	case TypeString:			attrType.value = L"string";			break;
	case TypeDateTime:			attrType.value = L"datetime";		break;
	case TypeData:				attrType.value = L"data";			break;
	case TypeArray:				attrType.value = L"array";			break;
	case TypePairs:				attrType.value = L"pairs";			break;
	default:					attrType.value = L"undefined";		break;
	}

	e->attributes.Add(attrType);

	if (valueType == TypeArray)
	{
		for (const DKVariant& var : this->Array())
		{
			DKObject<DKXMLElement> node = var.ExportXML();
			if (node != NULL)
				e->nodes.Add(node.SafeCast<DKXMLNode>());
		}
	}
	else if (valueType == TypePairs)
	{
		this->Pairs().EnumerateForward([&](const VPairs::Pair& pair)
		{
			DKObject<DKXMLElement> value = pair.value.ExportXML();
			if (value != NULL)
			{
				DKObject<DKXMLElement> node = DKObject<DKXMLElement>::New();
				DKObject<DKXMLElement> key = DKObject<DKXMLElement>::New();
				DKObject<DKXMLPCData> pcdata = DKObject<DKXMLPCData>::New();

				node->name = L"Node";
				key->name = L"Key";
				pcdata->value = pair.key;

				key->nodes.Add(pcdata.SafeCast<DKXMLNode>());
				node->nodes.Add(key.SafeCast<DKXMLNode>());
				node->nodes.Add(value.SafeCast<DKXMLNode>());

				e->nodes.Add(node.SafeCast<DKXMLNode>());
			}
		});
	}
	else
	{
		DKString data;
		if (valueType == TypeInteger)
		{
			data = DKString::Format("%lld", this->Integer());
		}
		else if (valueType == TypeFloat)
		{
			data = DKString::Format("%.32g", this->Float());
		}
		else if (valueType == TypeVector2)
		{
			data = DKString::Format("%.16g, %.16g", this->Vector2().x, this->Vector2().y);
		}
		else if (valueType == TypeVector3)
		{
			data = DKString::Format("%.16g, %.16g, %.16g", this->Vector3().x, this->Vector3().y, this->Vector3().z);
		}
		else if (valueType == TypeVector4)
		{
			data = DKString::Format("%.16g, %.16g, %.16g, %.16g", this->Vector4().x, this->Vector4().y, this->Vector4().z, this->Vector4().w);
		}
		else if (valueType == TypeMatrix2)
		{
			data = DKString::Format("%.16g, %.16g, %.16g, %.16g",
				this->Matrix2().val[0], this->Matrix2().val[1],
				this->Matrix2().val[2], this->Matrix2().val[3]);
		}
		else if (valueType == TypeMatrix3)
		{
			data = DKString::Format("%.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g",
				this->Matrix3().val[0], this->Matrix3().val[1], this->Matrix3().val[2],
				this->Matrix3().val[3], this->Matrix3().val[4], this->Matrix3().val[5],
				this->Matrix3().val[6], this->Matrix3().val[7], this->Matrix3().val[8]);
		}
		else if (valueType == TypeMatrix4)
		{
			data = DKString::Format("%.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g, %.16g",
				this->Matrix4().val[0x0], this->Matrix4().val[0x1], this->Matrix4().val[0x2], this->Matrix4().val[0x3],
				this->Matrix4().val[0x4], this->Matrix4().val[0x5], this->Matrix4().val[0x6], this->Matrix4().val[0x7],
				this->Matrix4().val[0x8], this->Matrix4().val[0x9], this->Matrix4().val[0xa], this->Matrix4().val[0xb],
				this->Matrix4().val[0xc], this->Matrix4().val[0xd], this->Matrix4().val[0xe], this->Matrix4().val[0xf]);
		}
		else if (valueType == TypeQuaternion)
		{
			data = DKString::Format("%.16g, %.16g, %.16g, %.16g", this->Quaternion().val[0], this->Quaternion().val[1], this->Quaternion().val[2], this->Quaternion().val[3]);
		}
		else if (valueType == TypeRational)
		{
			data = DKString::Format("%lld/%lld", this->Rational().Numerator(), this->Rational().Denominator());
		}
		else if (valueType == TypeString)
		{
			data = this->String();
		}
		else if (valueType == TypeDateTime)
		{
			data = this->DateTime().FormatISO8601();
		}
		else if (valueType == TypeData)
		{
			this->Data().CompressEncode(data);
		}
		if (data.Length() > 0)
		{
			DKObject<DKXMLPCData> pcdata = DKObject<DKXMLPCData>::New();
			pcdata->value = data;

			e->nodes.Add(pcdata.SafeCast<DKXMLNode>());
		}
	}

	return e;
}

bool DKVariant::ImportXML(const DKXMLElement* e)
{
	if (e->name.CompareNoCase(DKVARIANT_XML_ELEMENT) == 0)
	{
		this->SetValueType(TypeUndefined);
		for (int i = 0; i < e->attributes.Count(); ++i)
		{
			const DKXMLAttribute& attr = e->attributes.Value(i);
			if (attr.name.CompareNoCase(L"type") == 0)
			{
				if (attr.value.CompareNoCase(L"integer") == 0)
					this->SetValueType(TypeInteger);
				else if (attr.value.CompareNoCase(L"float") == 0)
					this->SetValueType(TypeFloat);
				else if (attr.value.CompareNoCase(L"vector2") == 0)
					this->SetValueType(TypeVector2);
				else if (attr.value.CompareNoCase(L"vector3") == 0)
					this->SetValueType(TypeVector3);
				else if (attr.value.CompareNoCase(L"vector4") == 0)
					this->SetValueType(TypeVector4);
				else if (attr.value.CompareNoCase(L"matrix2") == 0)
					this->SetValueType(TypeMatrix2);
				else if (attr.value.CompareNoCase(L"matrix3") == 0)
					this->SetValueType(TypeMatrix3);
				else if (attr.value.CompareNoCase(L"matrix4") == 0)
					this->SetValueType(TypeMatrix4);
				else if (attr.value.CompareNoCase(L"quaternion") == 0)
					this->SetValueType(TypeQuaternion);
				else if (attr.value.CompareNoCase(L"rational") == 0)
					this->SetValueType(TypeRational);
				else if (attr.value.CompareNoCase(L"string") == 0)
					this->SetValueType(TypeString);
				else if (attr.value.CompareNoCase(L"datetime") == 0)
					this->SetValueType(TypeDateTime);
				else if (attr.value.CompareNoCase(L"data") == 0)
					this->SetValueType(TypeData);
				else if (attr.value.CompareNoCase(L"array") == 0)
					this->SetValueType(TypeArray);
				else if (attr.value.CompareNoCase(L"pairs") == 0)
					this->SetValueType(TypePairs);

				break;
			}
		}
		if (this->ValueType() == TypeArray)
		{
			VArray va;
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
				{
					const DKXMLElement* node = e->nodes.Value(i).SafeCast<DKXMLElement>();
					if (node->name.CompareNoCase(DKVARIANT_XML_ELEMENT) == 0)
					{
						va.Add(DKVariant(node));
					}
				}
			}
			this->Array() = va;
		}
		else if (this->ValueType() == TypePairs)
		{
			VPairs vm;
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeElement)
				{
					const DKXMLElement* node = e->nodes.Value(i).SafeCast<DKXMLElement>();
					if (node->name.CompareNoCase(L"Node") == 0)
					{
						bool keyFound = false;
						DKString key = L"";
						DKVariant value;
						for (int k = 0; k < node->nodes.Count(); ++k)
						{
							if (node->nodes.Value(k)->Type() == DKXMLNode::NodeTypeElement)
							{
								const DKXMLElement* node2 = node->nodes.Value(k).SafeCast<DKXMLElement>();
								if (node2->name.CompareNoCase(L"Key") == 0)
								{
									for (int x = 0; x < node2->nodes.Count(); ++x)
									{
										if (node2->nodes.Value(x)->Type() == DKXMLNode::NodeTypeCData)
											key += node2->nodes.Value(x).SafeCast<DKXMLCData>()->value;
										else if (node2->nodes.Value(x)->Type() == DKXMLNode::NodeTypePCData)
											key += node2->nodes.Value(x).SafeCast<DKXMLPCData>()->value;
									}
									keyFound = true;
								}
								else if (node2->name.CompareNoCase(DKVARIANT_XML_ELEMENT) == 0)
								{
									value = DKVariant(node2);
								}
							}
						}
						if (keyFound)
							vm.Update(key, value);
					}
				}
			}
			this->Pairs() = vm;
		}
		else if (this->ValueType() != TypeUndefined)
		{
			DKString value = L"";
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypeCData)
				{
					value += e->nodes.Value(i).SafeCast<DKXMLCData>()->value;
				}
				else if (e->nodes.Value(i)->Type() == DKXMLNode::NodeTypePCData)
				{
					value += e->nodes.Value(i).SafeCast<DKXMLPCData>()->value;
				}
			}
			if (this->ValueType() == TypeString)
			{
				this->String() = value;
			}
			else if (this->ValueType() == TypeRational)
			{
				DKString::IntegerArray intArray = value.ToIntegerArray(L"/");
				VRational::Integer val[2] = {0LL, 1LL};
				for (size_t i = 0; i < 2 && i < intArray.Count(); ++i)
					val[i] = intArray.Value(i);
				this->Rational() = VRational(val[0], val[1]);
			}
			else if (this->ValueType() == TypeDateTime)
			{
				if (!DKDateTime::GetDateTime(this->DateTime(), value))
				{
					this->DateTime() = DKDateTime(0,0);
				}
			}
			else if (this->ValueType() == TypeData)
			{
				DKObject<DKBuffer> d = DKBuffer::DecodeDecompress(value);
				if (d)
					this->Data().SetContent(d);
				else
					this->Data().SetContent(0);
			}
			else if (this->ValueType() == TypeInteger)
			{
				this->Integer() = value.ToInteger();
			}
			else if (this->ValueType() == TypeFloat)
			{
				this->Float() = value.ToRealNumber();
			}
			else
			{
				DKString::RealNumberArray floatArray = value.ToRealNumberArray(L",");
				if (this->ValueType() == TypeVector2)
				{
					if (floatArray.Count() < 2)
						this->Vector2() = VVector2(0,0);
					else
						this->Vector2() = VVector2(floatArray.Value(0), floatArray.Value(1));
				}
				else if (this->ValueType() == TypeVector3)
				{
					if (floatArray.Count() < 3)
						this->Vector3() = VVector3(0,0,0);
					else
						this->Vector3() = VVector3(floatArray.Value(0), floatArray.Value(1), floatArray.Value(2));
				}
				else if (this->ValueType() == TypeVector4)
				{
					if (floatArray.Count() < 4)
						this->Vector4() = VVector4(0,0,0,0);
					else
						this->Vector4() = VVector4(floatArray.Value(0), floatArray.Value(1), floatArray.Value(2), floatArray.Value(3));
				}
				else if (this->ValueType() == TypeMatrix2)
				{
					if (floatArray.Count() < 4)
						this->Matrix2().Identity();
					else
					{
						for (int i = 0; i < 4; ++i)
							this->Matrix2().val[i] = floatArray.Value(i);
					}
				}
				else if (this->ValueType() == TypeMatrix3)
				{
					if (floatArray.Count() < 9)
						this->Matrix3().Identity();
					else
					{
						for (int i = 0; i < 9; ++i)
							this->Matrix3().val[i] = floatArray.Value(i);
					}
				}
				else if (this->ValueType() == TypeMatrix4)
				{
					if (floatArray.Count() < 16)
						this->Matrix4().Identity();
					else
					{
						for (int i = 0; i < 16; ++i)
							this->Matrix4().val[i] = floatArray.Value(i);
					}
				}
				else if (this->ValueType() == TypeQuaternion)
				{
					if (floatArray.Count() < 4)
						this->Quaternion().Identity();
					else
						this->Quaternion() = VQuaternion(floatArray.Value(0), floatArray.Value(1), floatArray.Value(2), floatArray.Value(3));
				}
			}
		}
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// DKVariant binary format layout
//   HEADER_STRING(fixed), Version(uint-16), Type(int-32), Data
//
// HEADER_STRING = "DKVariant" + byte_order_char (B/L)
// big-endian = "DKVariantB"
// little-endian = "DKVariantL"
//
// Data Size by Type
// Fixed length (bytes),
//  -TypeUndefined = 0
//  -TypeInteger = 8 (long long)
//  -TypeDateTime = 12 (int64+int32)
//  -TypeFloat = 8 (double, 8bytes)
//  -TypeVector2 = 8 (float x 2)
//  -TypeVector3 = 12 (float x 3)
//  -TypeVector4 = 16 (float x 4)
//  -TypeMatrix2 = 16 (float x 4)
//  -TypeMatrix3 = 36 (float x 9)
//  -TypeMatrix4 = 64 (float x 16)
//  -TypeQuaternion = 16 (float x 4)
// Variable length
//  -TypeString
//      Data: length(uint64), string(utf8)
//  -TypeData
//      Data: size(uint64), bytes
// Containers
//  -TypeArray
//      Data: number of items(uint64), item1, item2, ...
//  -TypePairs
//      Data: number of items(uint64),
//            length of key1(uint64), key1(utf8), item1,
//            length of key2(uint64), key2(utf8), item2, ...
//
////////////////////////////////////////////////////////////////////////////////

#define DKVARIANT_VERSION		12
#define DKVARIANT_HEADER_STRING_BIG_ENDIAN		"DKVariantB"
#define DKVARIANT_HEADER_STRING_LITTLE_ENDIAN	"DKVariantL"


#if __LITTLE_ENDIAN__
#define DKVARIANT_HEADER_STRING		DKVARIANT_HEADER_STRING_LITTLE_ENDIAN
#else
#define DKVARIANT_HEADER_STRING		DKVARIANT_HEADER_STRING_BIG_ENDIAN
#endif

bool DKVariant::ExportStream(DKStream* stream) const
{
	DKString errorDesc = L"Unknown error";
	bool validType = false;

	if (stream == NULL || stream->IsWritable() == false)
	{
		errorDesc = L"Invalid stream.";
		goto FAILED;
	}

	switch (valueType)
	{
	case TypeUndefined:
	case TypeInteger:
	case TypeFloat:
	case TypeVector2:
	case TypeVector3:
	case TypeVector4:
	case TypeMatrix2:
	case TypeMatrix3:
	case TypeMatrix4:
	case TypeQuaternion:
	case TypeRational:
	case TypeString:
	case TypeDateTime:
	case TypeData:
	case TypeArray:
	case TypePairs:
		validType = true;
		break;
	}
	if (validType)
	{
		size_t headerLen = strlen(DKVARIANT_HEADER_STRING);
		if (stream->Write(DKVARIANT_HEADER_STRING, headerLen) != headerLen)
		{
			errorDesc = L"Failed to write to stream.";
			goto FAILED;
		}
		const unsigned short version = DKVARIANT_VERSION;
		if (stream->Write(&version, sizeof(version)) != sizeof(version))
		{
			errorDesc = L"Failed to write to stream.";
			goto FAILED;
		}
		const unsigned int type = static_cast<unsigned int>(valueType);
		if (stream->Write(&type, sizeof(type)) != sizeof(type))
		{
			errorDesc = L"Failed to write to stream.";
			goto FAILED;
		}
		if (valueType == TypeUndefined)
		{
		}
		else if (valueType == TypeInteger)
		{
			unsigned long long v = static_cast<unsigned long long>(this->Integer());
			if (stream->Write(&v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeFloat)
		{
			VFloat value = this->Float();
			unsigned long long v = (*reinterpret_cast<unsigned long long*>(&value));
			if (stream->Write(&v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeVector2)
		{
			VVector2 value = this->Vector2();
			unsigned int v[2];
			for (int i = 0; i < 2; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeVector3)
		{
			VVector3 value = this->Vector3();
			unsigned int v[3];
			for (int i = 0; i < 3; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeVector4)
		{
			VVector4 value = this->Vector4();
			unsigned int v[4];
			for (int i = 0; i < 4; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeMatrix2)
		{
			VMatrix2 value = this->Matrix2();
			unsigned int v[4];
			for (int i = 0; i < 4; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeMatrix3)
		{
			VMatrix3 value = this->Matrix3();
			unsigned int v[9];
			for (int i = 0; i < 9; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeMatrix4)
		{
			VMatrix4 value = this->Matrix4();
			unsigned int v[16];
			for (int i = 0; i < 16; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeQuaternion)
		{
			VQuaternion value = this->Quaternion();
			unsigned int v[4];
			for (int i = 0; i < 4; ++i)
				v[i] = (*reinterpret_cast<unsigned int*>(&value.val[i]));
			if (stream->Write(v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeRational)
		{
			struct
			{
				long long n, d;
			} r = {this->Rational().Numerator(), this->Rational().Denominator()};
			
			if (stream->Write(&r, sizeof(r)) != sizeof(r))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeString)
		{
			DKStringU8 str(this->String());
			unsigned long long len = str.Bytes();
			if (stream->Write(&len, sizeof(len)) != sizeof(len))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			if (len > 0 && stream->Write((const char*)str, len) != len)
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeDateTime)
		{
			struct
			{
				long long s;
				int ms;
			} dt = {
				this->DateTime().SecondsSinceEpoch(),
				this->DateTime().Microsecond()
			};

			if (stream->Write(&dt, sizeof(dt)) != sizeof(dt))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeData)
		{
			const void* ptr = this->Data().LockShared();
			unsigned long long len = this->Data().Length();
			if (stream->Write(&len, sizeof(len)) != sizeof(len))
			{
				this->Data().UnlockShared();
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			if (len > 0 && stream->Write(ptr, len) != len)
			{
				this->Data().UnlockShared();
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			this->Data().UnlockShared();
		}
		else if (valueType == TypeArray)
		{
			const VArray& a = this->Array();
			unsigned long long len = a.Count();
			if (stream->Write(&len, sizeof(len)) != sizeof(len))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}			
			for (size_t i = 0; i < a.Count(); ++i)
			{
				if (a.Value(i).ExportStream(stream) == false)
				{
					errorDesc = L"Failed to extract array into stream.";
					goto FAILED;
				}
			}
		}
		else if (valueType == TypePairs)
		{
			DKArray<const VPairs::Pair*> a;
			a.Reserve(this->Pairs().Count());
			this->Pairs().EnumerateForward([&a](const VPairs::Pair& pair) {a.Add(&pair);});

			unsigned long long len = a.Count();
			if (stream->Write(&len, sizeof(len)) != sizeof(len))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}			
			for (size_t i = 0; i < a.Count(); ++i)
			{
				const VPairs::Pair* pair = a.Value(i);
				DKStringU8 key(pair->key);

				unsigned long long keyLen = key.Bytes();
				if (stream->Write(&keyLen, sizeof(keyLen)) != sizeof(keyLen))		// key length
				{
					errorDesc = L"Failed to write to stream.";
					goto FAILED;
				}
				if (keyLen > 0 && stream->Write((const char*)key, keyLen) != keyLen)	// key (utf8)
				{
					errorDesc = L"Failed to write to stream.";
					goto FAILED;
				}
				if (pair->value.ExportStream(stream) == false)		// value (DKVariant)
				{
					errorDesc = L"Failed to extract array into stream.";
					goto FAILED;
				}
			}
		}
		else
		{
			errorDesc = DKString::Format("Unknown Type: 0x%x.", valueType);
			goto FAILED;
		}
		return true;
	}
	else
	{
		errorDesc = DKString::Format("Unknown Type: 0x%x.", valueType);
		goto FAILED;
	}

FAILED:
	DKLog("DKVariant Error: %ls\n", (const wchar_t*)errorDesc);
	return false;
}

bool DKVariant::ImportStream(DKStream* stream)
{
	size_t headerLen = strlen(DKVARIANT_HEADER_STRING);

	DKString errorDesc = L"Unknown error";

	bool validHeader = false;
	bool littleEndian = false;

	if (stream == NULL || stream->IsReadable() == false)
	{
		errorDesc = L"Invalid stream.";
		goto FAILED;
	}		

	char name[64];
	if (stream->Read(name, headerLen) != headerLen)
	{
		errorDesc = L"Failed to read from stream.";
		goto FAILED;
	}

	if (strncmp(name, DKVARIANT_HEADER_STRING_BIG_ENDIAN, headerLen) == 0)
	{
		validHeader = true;
		littleEndian = false;
	}
	else if (strncmp(name, DKVARIANT_HEADER_STRING_LITTLE_ENDIAN, headerLen) == 0)
	{
		validHeader = true;
		littleEndian = true;
	}
	else
	{
		validHeader = false;
	}

	if (validHeader)
	{
		struct
		{
			unsigned short operator () (unsigned short v) const
			{
				return this->littleEndian ? LITTLE_ENDIAN_TO_SYSTEM_UINT16(v) : BIG_ENDIAN_TO_SYSTEM_UINT16(v);
			}
			unsigned int operator () (unsigned int v) const
			{
				return this->littleEndian ? LITTLE_ENDIAN_TO_SYSTEM_UINT32(v) : BIG_ENDIAN_TO_SYSTEM_UINT32(v);
			}
			unsigned long long operator () (unsigned long long v) const
			{
				return this->littleEndian ? LITTLE_ENDIAN_TO_SYSTEM_UINT64(v) : BIG_ENDIAN_TO_SYSTEM_UINT64(v);
			}
			bool littleEndian;
		} byteorder = {littleEndian};

		unsigned short v;
		if (stream->Read(&v, sizeof(v)) != sizeof(v))
		{
			errorDesc = L"Failed to read from stream.";
			goto FAILED;
		}
		v = byteorder(v);
		if (v <= DKVARIANT_VERSION)
		{
			if (v < DKVARIANT_VERSION)
			{
				DKLog("DKVariant Warning: file is older version:0x%x current-version:0x%0x.\n", static_cast<unsigned int>(v), static_cast<unsigned int>(DKVARIANT_VERSION));
			}

			unsigned int type;
			if (stream->Read(&type, sizeof(type)) != sizeof(type))
			{
				errorDesc = L"Failed to read from stream.";
				goto FAILED;
			}
			type = byteorder(type);

			bool validType = false;
			switch (static_cast<Type>(type))
			{
			case TypeUndefined:
			case TypeInteger:
			case TypeFloat:
			case TypeVector2:
			case TypeVector3:
			case TypeVector4:
			case TypeMatrix2:
			case TypeMatrix3:
			case TypeMatrix4:
			case TypeQuaternion:
			case TypeRational:
			case TypeString:
			case TypeDateTime:
			case TypeData:
			case TypeArray:
			case TypePairs:
				validType = true;
				break;
			}
			if (validType)
			{
				if (type == TypeUndefined)
				{
					SetValueType(TypeUndefined);
				}
				else if (type == TypeInteger)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					this->SetValueType(TypeInteger).Integer() = byteorder(v);
				}
				else if (type == TypeFloat)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					v = byteorder(v);
					this->SetValueType(TypeFloat).Float() = *reinterpret_cast<VFloat*>(&v);
				}
				else if (type == TypeVector2)
				{
					unsigned int v[2];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VVector2 val;
					for (int i = 0; i < 2; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeVector2).Vector2() = val;
				}
				else if (type == TypeVector3)
				{
					unsigned int v[3];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VVector3 val;
					for (int i = 0; i < 3; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeVector3).Vector3() = val;
				}
				else if (type == TypeVector4)
				{
					unsigned int v[4];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VVector4 val;
					for (int i = 0; i < 4; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeVector4).Vector4() = val;
				}
				else if (type == TypeMatrix2)
				{
					unsigned int v[4];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VMatrix2 val;
					for (int i = 0; i < 4; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeMatrix2).Matrix2() = val;
				}
				else if (type == TypeMatrix3)
				{
					unsigned int v[9];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VMatrix3 val;
					for (int i = 0; i < 9; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeMatrix3).Matrix3() = val;
				}
				else if (type == TypeMatrix4)
				{
					unsigned int v[16];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VMatrix4 val;
					for (int i = 0; i < 16; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeMatrix4).Matrix4() = val;
				}
				else if (type == TypeQuaternion)
				{
					unsigned int v[4];
					if (stream->Read(v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					VQuaternion val;
					for (int i = 0; i < 4; ++i)
					{
						v[i] = byteorder(v[i]);
						val.val[i] = *reinterpret_cast<float*>(&v[i]);
					}
					this->SetValueType(TypeQuaternion).Quaternion() = val;
				}
				else if (type == TypeRational)
				{
					struct
					{
						long long n,d;
					} r = {0LL, 0LL};

					if (stream->Read(&r, sizeof(r)) != sizeof(r))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}

					r.n = byteorder(static_cast<unsigned long long>(r.n));
					r.d = byteorder(static_cast<unsigned long long>(r.d));

					this->SetValueType(TypeRational).Rational() = VRational(r.n, r.d);
				}
				else if (type == TypeString)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					unsigned long long len = byteorder(v);
					DKString str = L"";
					if (len > 0)
					{
						void* p = DKMemoryDefaultAllocator::Alloc(len);
						if (stream->Read(p, len) != len)
						{
							DKMemoryDefaultAllocator::Free(p);
							errorDesc = L"Failed to read from stream.";
							goto FAILED;
						}
						str.SetValue((const DKUniChar8*)p, len);
						DKMemoryDefaultAllocator::Free(p);
					}
					this->SetValueType(TypeString).String() = str;
				}
				else if (type == TypeDateTime)
				{
					struct
					{
						long long s;
						int ms;
					} dt = {0LL, 0};

					if (stream->Read(&dt, sizeof(dt)) != sizeof(dt))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}

					dt.s = byteorder(static_cast<unsigned long long>(dt.s));
					dt.ms = byteorder(static_cast<unsigned int>(dt.ms));

					this->SetValueType(TypeDateTime).DateTime() = DKDateTime(dt.s, dt.ms);
				}
				else if (type == TypeData)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					unsigned long long len = byteorder(v);
					if (len > 0)
					{
						DKBuffer& val = this->SetValueType(TypeData).Data();
						val.SetContent(0, len);
						void* p = val.LockExclusive();
						if (stream->Read(p, len) != len)
						{
							val.UnlockExclusive();
							errorDesc = L"Failed to read from stream.";
							goto FAILED;
						}
						val.UnlockExclusive();
					}
					else
						this->SetValueType(TypeData).Data().SetContent(0);
				}
				else if (type == TypeArray)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					unsigned long long len = byteorder(v);
					VArray val;
					if (len > 0)
					{
						val.Reserve(len);
						for (unsigned long long i = 0; i < len; ++i)
						{
							DKVariant v;
							if (v.ImportStream(stream) == false)
							{
								errorDesc = L"Failed to restore array element from stream.";
								goto FAILED;
							}
							val.Add(v);
						}
					}
					this->SetValueType(TypeArray).Array() = val;
				}
				else if (type == TypePairs)
				{
					unsigned long long v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					unsigned long long len = byteorder(v);
					VPairs val;
					for (unsigned long long i = 0; i < len; ++i)
					{
						unsigned long long keyLen;
						if (stream->Read(&keyLen, sizeof(keyLen)) != sizeof(keyLen))
						{
							errorDesc = L"Failed to read from stream.";
							goto FAILED;
						}
						DKString key = L"";
						if (keyLen > 0)
						{
							void* p = DKMemoryDefaultAllocator::Alloc(keyLen);
							if (stream->Read(p, keyLen) != keyLen)
							{
								DKMemoryDefaultAllocator::Free(p);
								errorDesc = L"Failed to read from stream.";
								goto FAILED;
							}
							key.SetValue((const DKUniChar8*)p, keyLen);
							DKMemoryDefaultAllocator::Free(p);
						}
						DKVariant variant;
						if (variant.ImportStream(stream) == false)
						{
							errorDesc = L"Failed to restore map element from stream.";
							goto FAILED;
						}
						val.Update(key, variant);
					}
					this->SetValueType(TypePairs).Pairs() = val;
				}
				else
				{
					errorDesc = DKString::Format("Unknown Type: 0x%x.", valueType);
					goto FAILED;
				}
				return true;
			}
			else
			{
				errorDesc = DKString::Format("Unknown Type: 0x%x.", type);
				goto FAILED;
			}
		}
		else
		{
			errorDesc = DKString::Format("Wrong binary version: 0x%x.", static_cast<unsigned int>(v));
			goto FAILED;
		}
	}
	else
	{
		errorDesc = L"Format is not DKVariant";
		goto FAILED;
	}

FAILED:
	DKLog("DKVariant Error: %ls\n", (const wchar_t*)errorDesc);
	return false;
}

DKVariant& DKVariant::operator = (const DKVariant& v)
{
	return this->SetValue(v);
}

DKVariant& DKVariant::operator = (DKVariant&& v)
{
	if (this != &v)
	{
		this->SetValueType(TypeUndefined);

		this->valueType = v.valueType;
		memcpy(this->vblock, v.vblock, sizeof(this->vblock));
		memset(v.vblock, 0, sizeof(v.vblock));
		v.valueType = TypeUndefined;
	}
	return *this;
}

DKVariant::VInteger& DKVariant::Integer(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeInteger);
	DKASSERT_DEBUG(ValueType() == TypeInteger);
	return Private::VariantBlock<VInteger, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VInteger& DKVariant::Integer(void) const
{
	return const_cast<DKVariant*>(this)->Integer();
}

DKVariant::VFloat& DKVariant::Float(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeFloat);
	DKASSERT_DEBUG(ValueType() == TypeFloat);
	return Private::VariantBlock<VFloat, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VFloat& DKVariant::Float(void) const
{
	return const_cast<DKVariant*>(this)->Float();
}

DKVariant::VVector2& DKVariant::Vector2(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector2);
	DKASSERT_DEBUG(ValueType() == TypeVector2);
	return Private::VariantBlock<VVector2, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector2& DKVariant::Vector2(void) const
{
	return const_cast<DKVariant*>(this)->Vector2();
}

DKVariant::VVector3& DKVariant::Vector3(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector3);
	DKASSERT_DEBUG(ValueType() == TypeVector3);
	return Private::VariantBlock<VVector3, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector3& DKVariant::Vector3(void) const
{
	return const_cast<DKVariant*>(this)->Vector3();
}

DKVariant::VVector4& DKVariant::Vector4(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector4);
	DKASSERT_DEBUG(ValueType() == TypeVector4);
	return Private::VariantBlock<VVector4, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector4& DKVariant::Vector4(void) const
{
	return const_cast<DKVariant*>(this)->Vector4();
}

DKVariant::VMatrix2& DKVariant::Matrix2(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix2);
	DKASSERT_DEBUG(ValueType() == TypeMatrix2);
	return Private::VariantBlock<VMatrix2, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix2& DKVariant::Matrix2(void) const
{
	return const_cast<DKVariant*>(this)->Matrix2();
}

DKVariant::VMatrix3& DKVariant::Matrix3(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix3);
	DKASSERT_DEBUG(ValueType() == TypeMatrix3);
	return Private::VariantBlock<VMatrix3, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix3& DKVariant::Matrix3(void) const
{
	return const_cast<DKVariant*>(this)->Matrix3();
}

DKVariant::VMatrix4& DKVariant::Matrix4(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix4);
	DKASSERT_DEBUG(ValueType() == TypeMatrix4);
	return Private::VariantBlock<VMatrix4, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix4& DKVariant::Matrix4(void) const
{
	return const_cast<DKVariant*>(this)->Matrix4();
}

DKVariant::VQuaternion& DKVariant::Quaternion(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeQuaternion);
	DKASSERT_DEBUG(ValueType() == TypeQuaternion);
	return Private::VariantBlock<VQuaternion, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VQuaternion& DKVariant::Quaternion(void) const
{
	return const_cast<DKVariant*>(this)->Quaternion();
}

DKVariant::VRational& DKVariant::Rational(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeRational);
	DKASSERT_DEBUG(ValueType() == TypeRational);
	return Private::VariantBlock<VRational, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VRational& DKVariant::Rational(void) const
{
	return const_cast<DKVariant*>(this)->Rational();
}

DKVariant::VString& DKVariant::String(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeString);
	DKASSERT_DEBUG(ValueType() == TypeString);
	return Private::VariantBlock<VString, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VString& DKVariant::String(void) const
{
	return const_cast<DKVariant*>(this)->String();
}

DKVariant::VDateTime& DKVariant::DateTime(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeDateTime);
	DKASSERT_DEBUG(ValueType() == TypeDateTime);
	return Private::VariantBlock<VDateTime, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VDateTime& DKVariant::DateTime(void) const
{
	return const_cast<DKVariant*>(this)->DateTime();
}

DKVariant::VData& DKVariant::Data(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeData);
	DKASSERT_DEBUG(ValueType() == TypeData);
	return Private::VariantBlock<VData, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VData& DKVariant::Data(void) const
{
	return const_cast<DKVariant*>(this)->Data();
}

DKVariant::VArray& DKVariant::Array(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeArray);
	DKASSERT_DEBUG(ValueType() == TypeArray);
	return Private::VariantBlock<VArray, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VArray& DKVariant::Array(void) const
{
	return const_cast<DKVariant*>(this)->Array();
}

DKVariant::VPairs& DKVariant::Pairs(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypePairs);
	DKASSERT_DEBUG(ValueType() == TypePairs);
	return Private::VariantBlock<VPairs, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VPairs& DKVariant::Pairs(void) const
{
	return const_cast<DKVariant*>(this)->Pairs();
}

DKVariant& DKVariant::SetInteger(const VInteger& v)
{
	SetValueType(TypeInteger).Integer() = v;
	return *this;
}

DKVariant& DKVariant::SetFloat(const VFloat& v)
{
	SetValueType(TypeFloat).Float() = v;
	return *this;
}

DKVariant& DKVariant::SetVector2(const VVector2& v)
{
	SetValueType(TypeVector2).Vector2() = v;
	return *this;
}

DKVariant& DKVariant::SetVector3(const VVector3& v)
{
	SetValueType(TypeVector3).Vector3() = v;
	return *this;
}

DKVariant& DKVariant::SetVector4(const VVector4& v)
{
	SetValueType(TypeVector4).Vector4() = v;
	return *this;
}

DKVariant& DKVariant::SetMatrix2(const VMatrix2& v)
{
	SetValueType(TypeMatrix2).Matrix2() = v;
	return *this;
}

DKVariant& DKVariant::SetMatrix3(const VMatrix3& v)
{
	SetValueType(TypeMatrix3).Matrix3() = v;
	return *this;
}

DKVariant& DKVariant::SetMatrix4(const VMatrix4& v)
{
	SetValueType(TypeMatrix4).Matrix4() = v;
	return *this;
}

DKVariant& DKVariant::SetQuaternion(const VQuaternion& v)
{
	SetValueType(TypeQuaternion).Quaternion() = v;
	return *this;
}

DKVariant& DKVariant::SetRational(const VRational& v)
{
	SetValueType(TypeRational).Rational() = v;
	return *this;
}

DKVariant& DKVariant::SetString(const VString& v)
{
	SetValueType(TypeString).String() = v;
	return *this;
}

DKVariant& DKVariant::SetDateTime(const VDateTime& v)
{
	SetValueType(TypeDateTime).DateTime() = v;
	return *this;
}

DKVariant& DKVariant::SetData(const VData& v)
{
	SetValueType(TypeData).Data() = v;
	return *this;
}

DKVariant& DKVariant::SetData(const void* p, size_t s)
{
	SetValueType(TypeData).Data().SetContent(p, s);
	return *this;
}

DKVariant& DKVariant::SetArray(const VArray& v)
{
	SetValueType(TypeArray).Array() = v;
	return *this;
}

DKVariant& DKVariant::SetPairs(const VPairs& v)
{
	SetValueType(TypePairs).Pairs() = v;
	return *this;
}

DKVariant& DKVariant::SetValue(const DKVariant& v)
{
	SetValueType(v.ValueType());

	switch (valueType)
	{
	case TypeInteger:
		this->Integer() = v.Integer();
		break;
	case TypeFloat:
		this->Float() = v.Float();
		break;
	case TypeVector2:
		this->Vector2() = v.Vector2();
		break;
	case TypeVector3:
		this->Vector3() = v.Vector3();
		break;
	case TypeVector4:
		this->Vector4() = v.Vector4();
		break;
	case TypeMatrix2:
		this->Matrix2() = v.Matrix2();
		break;
	case TypeMatrix3:
		this->Matrix3() = v.Matrix3();
		break;
	case TypeMatrix4:
		this->Matrix4() = v.Matrix4();
		break;
	case TypeQuaternion:
		this->Quaternion() = v.Quaternion();
		break;
	case TypeRational:
		this->Rational() = v.Rational();
		break;
	case TypeString:
		this->String() = v.String();
		break;
	case TypeDateTime:
		this->DateTime() = v.DateTime();
		break;
	case TypeData:
		this->Data() = v.Data();
		break;
	case TypeArray:
		this->Array() = v.Array();
		break;
	case TypePairs:
		this->Pairs() = v.Pairs();
		break;
	}
	return *this;
}

bool DKVariant::IsEqual(const DKVariant& v) const
{
	bool result = false;
	if (valueType == v.valueType)
	{
		switch (valueType)
		{
		case TypeUndefined:
			result = true;
			break;
		case TypeInteger:
			result = this->Integer() == v.Integer();
			break;
		case TypeFloat:
			result = this->Float() == v.Float();
			break;
		case TypeVector2:
			result = this->Vector2() == v.Vector2();
			break;
		case TypeVector3:
			result = this->Vector3() == v.Vector3();
			break;
		case TypeVector4:
			result = this->Vector4() == v.Vector4();
			break;
		case TypeMatrix2:
			result = this->Matrix2() == v.Matrix2();
			break;
		case TypeMatrix3:
			result = this->Matrix3() == v.Matrix3();
			break;
		case TypeMatrix4:
			result = this->Matrix4() == v.Matrix4();
			break;
		case TypeQuaternion:
			result = this->Quaternion() == v.Quaternion();
			break;
		case TypeString:
			result = this->String() == v.String();
			break;
		case TypeDateTime:
			result = this->DateTime() == v.DateTime();
			break;
		case TypeData:
			if (this->Data().Length() == v.Data().Length())
			{
				size_t count = this->Data().Length();
				const void* ptr1 = this->Data().LockShared();
				const void* ptr2 = v.Data().LockShared();

				result = memcmp(ptr1, ptr2, count) == 0;

				this->Data().UnlockShared();
				v.Data().UnlockShared();
			}
			break;
		case TypeArray:
			if (this->Array().Count() == v.Array().Count())
			{
				result = true;
				size_t count = this->Array().Count();

				for (size_t i = 0; i < count; ++i)
				{
					if (this->Array().Value(i).IsEqual(v.Array().Value(i)) == false)
					{
						result = false;
						break;
					}
				}
			}
			break;
		case TypePairs:
			if (this->Pairs().Count() == v.Pairs().Count())
			{
				DKArray<const VPairs::Pair*> pairs1;
				DKArray<const VPairs::Pair*> pairs2;

				pairs1.Reserve(this->Pairs().Count());
				pairs2.Reserve(v.Pairs().Count());

				this->Pairs().EnumerateForward([&pairs1](const VPairs::Pair& pair){pairs1.Add(&pair);});
				v.Pairs().EnumerateForward([&pairs2](const VPairs::Pair& pair){pairs2.Add(&pair);});

				if (pairs1.Count() == pairs2.Count())
				{
					result = true;
					size_t count = pairs1.Count();

					for (size_t i = 0; i < count; ++i)
					{
						const VPairs::Pair* p1 = pairs1.Value(i);
						const VPairs::Pair* p2 = pairs2.Value(i);

						if (p1->key != p2->key || p1->value.IsEqual(p2->value) == false)
						{
							result = false;
							break;
						}
					}
				}
			}
			break;
		default:
			result = false;
			break;
		}
	}
	return result;
}
