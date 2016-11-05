//
//  File: DKVariant.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKVariant.h"

namespace DKGL
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

		// utility functions for StructuredData
		using StructElem = DKVariant::StructElem;
		using VStructuredData = DKVariant::VStructuredData;
		static bool IsValidStructElement(StructElem e)
		{
			switch (e)
			{
			case StructElem::Arithmetic1:
			case StructElem::Arithmetic2:
			case StructElem::Arithmetic4:
			case StructElem::Arithmetic8:
			case StructElem::Bypass1:
			case StructElem::Bypass2:
			case StructElem::Bypass4:
			case StructElem::Bypass8:
				return true;
			}
			return false;
		}
		static bool ValidateStructElementLayout(const VStructuredData& sd)
		{
			for (StructElem e : sd.layout)
			{
				if (!IsValidStructElement(e))
					return false;
			}
			return true;
		}
		static DKString StructElementToString(StructElem e)
		{
			switch (e)
			{
			case StructElem::Arithmetic1:	return "a1"; break;
			case StructElem::Arithmetic2:	return "a2"; break;
			case StructElem::Arithmetic4:	return "a4"; break;
			case StructElem::Arithmetic8:	return "a8"; break;
			case StructElem::Bypass1:		return "b1"; break;
			case StructElem::Bypass2:		return "b2"; break;
			case StructElem::Bypass4:		return "b4"; break;
			case StructElem::Bypass8:		return "b8"; break;
			}
			return DKString::empty;
		}
		static bool ConvertStringToStructElement(const DKString& s, StructElem& out)
		{
			if (s.Length() >= 2)
			{
				DKString::CharT v1 = s[0];
				DKString::CharT v2 = s[1];
				if (v1 == DKString::CharT('a') || v1 == DKString::CharT('b'))
				{
					if (v2 == DKString::CharT('1') ||
						v2 == DKString::CharT('2') ||
						v2 == DKString::CharT('4') ||
						v2 == DKString::CharT('8'))
						return true;
				}
			}
			return false;
		}
		static bool ConvertStructuredDataByteOrder(VStructuredData& sd, DKByteOrder bo)
		{
			size_t len = sd.data.Length();
			if (bo != DKRuntimeByteOrder() && sd.elementSize > 0 && sd.elementSize <= len && sd.layout.Count() > 0)
			{
				uint8_t* p = reinterpret_cast<uint8_t*>(sd.data.LockExclusive());
				if (p)
				{
					size_t pos = 0;
					while (pos < len)
					{
						size_t n = 0;
						uint8_t* p2 = &(p[pos]);
						for (StructElem e : sd.layout)
						{
							uint8_t fieldSize = static_cast<uint8_t>(e) & 0x0f;
							n += fieldSize;
							if (n >= sd.elementSize)
								break;

							if ((static_cast<uint8_t>(e) & 0xf0) == 0)
							{
								switch (fieldSize)
								{
								case 1:
									reinterpret_cast<uint8_t*>(p2)[0] = DKSwitchIntegralByteOrder(reinterpret_cast<uint8_t*>(p2)[0]);
									break;
								case 2:
									reinterpret_cast<uint16_t*>(p2)[0] = DKSwitchIntegralByteOrder(reinterpret_cast<uint16_t*>(p2)[0]);
									break;
								case 4:
									reinterpret_cast<uint32_t*>(p2)[0] = DKSwitchIntegralByteOrder(reinterpret_cast<uint32_t*>(p2)[0]);
									break;
								case 8:
									reinterpret_cast<uint64_t*>(p2)[0] = DKSwitchIntegralByteOrder(reinterpret_cast<uint64_t*>(p2)[0]);
									break;
								}
							}

							p2 += fieldSize;
						}
						pos += sd.elementSize;
					}
					sd.data.UnlockExclusive();
					return true;
				}
				return false;
			}
			return true;
		}
	}
}

using namespace DKGL;
using namespace DKGL::Private;


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

DKVariant::DKVariant(const VStructuredData& v)
	: valueType(TypeUndefined)
{
	memset(vblock, 0, sizeof(vblock));
	this->SetStructuredData(v);
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

DKVariant::DKVariant(const DKXmlElement* e)
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
		VariantBlock<VInteger, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeFloat:
		VariantBlock<VFloat, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector2:
		VariantBlock<VVector2, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector3:
		VariantBlock<VVector3, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeVector4:
		VariantBlock<VVector4, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix2:
		VariantBlock<VMatrix2, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix3:
		VariantBlock<VMatrix3, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeMatrix4:
		VariantBlock<VMatrix4, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeQuaternion:
		VariantBlock<VQuaternion, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeRational:
		VariantBlock<VRational, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeString:
		VariantBlock<VString, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeData:
		VariantBlock<VData, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeStructData:
		VariantBlock<VStructuredData, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeDateTime:
		VariantBlock<VDateTime, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypeArray:
		VariantBlock<VArray, sizeof(vblock)>::Dealloc(vblock);
		break;
	case TypePairs:
		VariantBlock<VPairs, sizeof(vblock)>::Dealloc(vblock);
		break;
	}
	memset(vblock, 0, sizeof(vblock));

	valueType = t;
	switch (valueType)
	{
	case TypeInteger:
		VariantBlock<VInteger, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeFloat:
		VariantBlock<VFloat, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector2:
		VariantBlock<VVector2, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector3:
		VariantBlock<VVector3, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeVector4:
		VariantBlock<VVector4, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix2:
		VariantBlock<VMatrix2, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix3:
		VariantBlock<VMatrix3, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeMatrix4:
		VariantBlock<VMatrix4, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeQuaternion:
		VariantBlock<VQuaternion, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeRational:
		VariantBlock<VRational, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeString:
		VariantBlock<VString, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeData:
		VariantBlock<VData, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeStructData:
		VariantBlock<VStructuredData, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeDateTime:
		VariantBlock<VDateTime, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypeArray:
		VariantBlock<VArray, sizeof(vblock)>::Alloc(vblock);
		break;
	case TypePairs:
		VariantBlock<VPairs, sizeof(vblock)>::Alloc(vblock);
		break;
	}
	return *this;
}

DKVariant::Type DKVariant::ValueType(void) const
{
	return valueType;
}

#define DKVARIANT_XML_ELEMENT	L"DKVariant"

DKObject<DKXmlElement> DKVariant::ExportXML(void) const
{
	DKObject<DKXmlElement> e = DKObject<DKXmlElement>::New();
	e->name = DKVARIANT_XML_ELEMENT;
	DKXmlAttribute attrType;
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
	case TypeStructData:		attrType.value = L"structdata";		break;
	case TypeArray:				attrType.value = L"array";			break;
	case TypePairs:				attrType.value = L"pairs";			break;
	default:					attrType.value = L"undefined";		break;
	}

	e->attributes.Add(attrType);

	if (valueType == TypeArray)
	{
		for (const DKVariant& var : this->Array())
		{
			DKObject<DKXmlElement> node = var.ExportXML();
			if (node != NULL)
				e->nodes.Add(node.SafeCast<DKXmlNode>());
		}
	}
	else if (valueType == TypePairs)
	{
		this->Pairs().EnumerateForward([&](const VPairs::Pair& pair)
		{
			DKObject<DKXmlElement> value = pair.value.ExportXML();
			if (value != NULL)
			{
				DKObject<DKXmlElement> node = DKObject<DKXmlElement>::New();
				DKObject<DKXmlElement> key = DKObject<DKXmlElement>::New();
				DKObject<DKXmlPCData> pcdata = DKObject<DKXmlPCData>::New();

				node->name = L"Node";
				key->name = L"Key";
				pcdata->value = pair.key;

				key->nodes.Add(pcdata.SafeCast<DKXmlNode>());
				node->nodes.Add(key.SafeCast<DKXmlNode>());
				node->nodes.Add(value.SafeCast<DKXmlNode>());

				e->nodes.Add(node.SafeCast<DKXmlNode>());
			}
		});
	}
	else if (valueType == TypeData)
	{
		DKObject<DKXmlCData> cdata = DKObject<DKXmlCData>::New();
		DKObject<DKBuffer> compressed = this->Data().Compress(DKCompressor::Deflate);
		if (compressed)
			compressed->Base64Encode(cdata->value);
		e->nodes.Add(cdata.SafeCast<DKXmlCData>());
	}
	else if (valueType == TypeStructData)
	{
		const VStructuredData& stData = this->StructuredData();
		DKXmlAttribute byteorder, elementSize;
		byteorder.name = "byteorder";
		byteorder.value = (DKRuntimeByteOrder() == DKByteOrder::BigEndian) ? "BE" : "LE";
		elementSize.name = "elementSize";
		elementSize.value = DKString::Format("%llu", stData.elementSize);

		DKObject<DKXmlElement> layout = DKObject<DKXmlElement>::New();
		layout->name = "layout";
		layout->attributes.Add(elementSize);
		layout->attributes.Add(byteorder);

		if (stData.layout.Count())
		{
			if (ValidateStructElementLayout(stData))
			{
				DKObject<DKXmlPCData> layoutData = DKObject<DKXmlPCData>::New();
				layoutData->value = StructElementToString(stData.layout.Value(0));

				size_t numElements = stData.layout.Count();
				for (size_t i = 1; i < numElements; ++i)
				{
					layoutData->value.Append(DKString::Format(", %ls",
						(const wchar_t*)StructElementToString(stData.layout.Value(i))));
				}
				layout->nodes.Add(layoutData.SafeCast<DKXmlNode>());
			}
			else
			{
				DKLog("Warning: DKVariant::VStructuredData.layout is invalid!\n");
			}
		}
		e->nodes.Add(layout.SafeCast<DKXmlNode>());

		DKObject<DKXmlCData> cdata = DKObject<DKXmlCData>::New();
		DKObject<DKBuffer> compressed = stData.data.Compress(DKCompressor::Deflate);
		if (compressed)
		{
			compressed->Base64Encode(cdata->value);
		}
		e->nodes.Add(cdata.SafeCast<DKXmlNode>());
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
		if (data.Length() > 0)
		{
			DKObject<DKXmlPCData> pcdata = DKObject<DKXmlPCData>::New();
			pcdata->value = data;

			e->nodes.Add(pcdata.SafeCast<DKXmlNode>());
		}
	}

	return e;
}

bool DKVariant::ImportXML(const DKXmlElement* e)
{
	if (e->name.CompareNoCase(DKVARIANT_XML_ELEMENT) == 0)
	{
		this->SetValueType(TypeUndefined);
		for (int i = 0; i < e->attributes.Count(); ++i)
		{
			const DKXmlAttribute& attr = e->attributes.Value(i);
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
				else if (attr.value.CompareNoCase(L"structdata") == 0)
					this->SetValueType(TypeStructData);
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
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeElement)
				{
					const DKXmlElement* node = e->nodes.Value(i).SafeCast<DKXmlElement>();
					if (node->name.CompareNoCase(DKVARIANT_XML_ELEMENT) == 0)
					{
						va.Add(DKVariant(node));
					}
				}
			}
			this->Array() = static_cast<VArray&&>(va);
		}
		else if (this->ValueType() == TypePairs)
		{
			VPairs vm;
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeElement)
				{
					const DKXmlElement* node = e->nodes.Value(i).SafeCast<DKXmlElement>();
					if (node->name.CompareNoCase(L"Node") == 0)
					{
						bool keyFound = false;
						DKString key = L"";
						DKVariant value;
						for (int k = 0; k < node->nodes.Count(); ++k)
						{
							if (node->nodes.Value(k)->Type() == DKXmlNode::NodeTypeElement)
							{
								const DKXmlElement* node2 = node->nodes.Value(k).SafeCast<DKXmlElement>();
								if (node2->name.CompareNoCase(L"Key") == 0)
								{
									for (int x = 0; x < node2->nodes.Count(); ++x)
									{
										if (node2->nodes.Value(x)->Type() == DKXmlNode::NodeTypeCData)
											key += node2->nodes.Value(x).SafeCast<DKXmlCData>()->value;
										else if (node2->nodes.Value(x)->Type() == DKXmlNode::NodeTypePCData)
											key += node2->nodes.Value(x).SafeCast<DKXmlPCData>()->value;
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
			this->Pairs() = static_cast<VPairs&&>(vm);
		}
		else if (this->ValueType() == TypeData)
		{
			DKStringU8 value;
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeCData)
				{
					value = e->nodes.Value(i).SafeCast<DKXmlCData>()->value;
					break;
				}
			}
			DKObject<VData> compressed = VData::Base64Decode(value);
			if (compressed)
			{
				DKObject<VData> d = compressed->Decompress();
				if (d)
					this->Data() = static_cast<VData&&>(*d);
			}
		}
		else if (this->ValueType() == TypeStructData)
		{
			DKByteOrder dataByteOrder = DKRuntimeByteOrder();
			VStructuredData& stData = this->StructuredData();

			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeElement)
				{
					const DKXmlElement* node = e->nodes.Value(i).SafeCast<DKXmlElement>();
					if (node->name.CompareNoCase(L"layout") == 0)
					{
						for (const DKXmlAttribute& attr : node->attributes)
						{
							if (attr.name.CompareNoCase("elementSize") == 0)
							{
								stData.elementSize = attr.value.ToUnsignedInteger();
							}
							else if (attr.name.CompareNoCase("byteorder") == 0)
							{
								if (attr.value.CompareNoCase("BE") == 0)
									dataByteOrder = DKByteOrder::BigEndian;
								else if (attr.value.CompareNoCase("LE") == 0)
									dataByteOrder = DKByteOrder::LittleEndian;
							}
						}

						DKString layoutData;
						for (int j = 0; j < node->nodes.Count(); ++j)
						{
							if (node->nodes.Value(j)->Type() == DKXmlNode::NodeTypePCData)
							{
								const DKXmlPCData* pcData = node->nodes.Value(j).SafeCast<DKXmlPCData>();
								if (pcData)
								{
									layoutData.Append(pcData->value);
								}
							}
						}
						DKString::StringArray layoutStrArray = layoutData.Split(L",");
						for (DKString& elemStr : layoutStrArray)
						{
							DKString s = elemStr.TrimWhitespaces();
							StructElem el;
							if (ConvertStringToStructElement(s, el))
								stData.layout.Add(el);
							else
							{
								stData.layout.Clear();
								DKLog("Warning: DKVariant::VStructuredData.layout is invalid!\n");
								break;
							}
						}
						break;
					}
				}
			}
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypeCData)
				{
					const DKXmlCData* cData = e->nodes.Value(i).SafeCast<DKXmlCData>();
					if (cData)
					{
						DKObject<DKBuffer> compressed = DKBuffer::Base64Decode(cData->value);
						if (compressed)
						{
							DKObject<DKBuffer> data = compressed->Decompress();
							if (data)
							{
								stData.data = static_cast<DKBuffer&&>(*data); // move!
								if (!ConvertStructuredDataByteOrder(stData, dataByteOrder))
								{
									DKLog("Error: DKVariant::VStructuredData data byte order error!\n");
								}
								break;
							}
						}
					}
				}
			}
		}
		else if (this->ValueType() != TypeUndefined)
		{
			DKString value = L"";
			for (int i = 0; i < e->nodes.Count(); ++i)
			{
				if (e->nodes.Value(i)->Type() == DKXmlNode::NodeTypePCData)
				{
					value.Append(e->nodes.Value(i).SafeCast<DKXmlPCData>()->value);
				}
			}
			if (this->ValueType() == TypeString)
			{
				this->String() = static_cast<DKString&&>(value);
			}
			else if (this->ValueType() == TypeRational)
			{
				DKString::IntegerArray intArray = value.ToIntegerArray(L"/");
				VRational::Integer val[2] = { 0LL, 1LL };
				for (size_t i = 0; i < 2 && i < intArray.Count(); ++i)
					val[i] = intArray.Value(i);
				this->Rational() = VRational(val[0], val[1]);
			}
			else if (this->ValueType() == TypeDateTime)
			{
				if (!DKDateTime::GetDateTime(this->DateTime(), value))
				{
					this->DateTime() = DKDateTime(0, 0);
				}
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
						this->Vector2() = VVector2(0, 0);
					else
						this->Vector2() = VVector2(floatArray.Value(0), floatArray.Value(1));
				}
				else if (this->ValueType() == TypeVector3)
				{
					if (floatArray.Count() < 3)
						this->Vector3() = VVector3(0, 0, 0);
					else
						this->Vector3() = VVector3(floatArray.Value(0), floatArray.Value(1), floatArray.Value(2));
				}
				else if (this->ValueType() == TypeVector4)
				{
					if (floatArray.Count() < 4)
						this->Vector4() = VVector4(0, 0, 0, 0);
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
//  -TypeInteger = 8 (int64)
//  -TypeDateTime = 12 (int64+int32)
//  -TypeFloat = 8 (double, 8bytes)
//  -TypeVector2 = 8 (float x 2)
//  -TypeVector3 = 12 (float x 3)
//  -TypeVector4 = 16 (float x 4)
//  -TypeMatrix2 = 16 (float x 4)
//  -TypeMatrix3 = 36 (float x 9)
//  -TypeMatrix4 = 64 (float x 16)
//  -TypeQuaternion = 16 (float x 4)
//
// Variable length
//  -TypeString
//      Data: length(uint64), utf8-string(uint8[length])
//  -TypeData
//      Data: length(uint64), bytes(uint8[length])
//  -TypeStructData
//      Header: elementSize(uint64), num-Layout(uint64), data-length(uint64)
//      Layout-Data: layout-bytes(uint8[num-Layout])
//      Content-Data: data-bytes(uint8[data-length])
//
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

#pragma pack(push, 4)

static_assert(sizeof(uint32_t) == sizeof(float), "Size mismatch!");
static_assert(sizeof(uint64_t) == sizeof(DKVariant::VFloat), "Size mismatch!");
static_assert(sizeof(uint64_t) == sizeof(DKVariant::VInteger), "Size mismatch!");


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
	case TypeStructData:
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
			uint64_t v = static_cast<uint64_t>(this->Integer());
			if (stream->Write(&v, sizeof(v)) != sizeof(v))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeFloat)
		{
			VFloat value = this->Float();
			uint64_t v = (*reinterpret_cast<uint64_t*>(&value));
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
				int64_t n, d;
			} r = { this->Rational().Numerator(), this->Rational().Denominator() };

			if (stream->Write(&r, sizeof(r)) != sizeof(r))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
		}
		else if (valueType == TypeString)
		{
			DKStringU8 str(this->String());
			uint64_t len = str.Bytes();
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
				int64_t s;
				int32_t ms;
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
			uint64_t len = this->Data().Length();
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
		else if (valueType == TypeStructData)
		{
			const VStructuredData& stData = this->StructuredData();
			struct
			{
				uint64_t elementSize;
				uint64_t numLayouts;
				uint64_t dataLength;
			} stInfo = { stData.elementSize, stData.layout.Count(), stData.data.Length() };
			if (stream->Write(&stInfo, sizeof(stInfo)) != sizeof(stInfo))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			if (stream->Write(stData.layout, stInfo.numLayouts) != stInfo.numLayouts)
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			if (stream->Write(stData.data.LockShared(), stInfo.dataLength) != stInfo.dataLength)
			{
				stData.data.UnlockShared();
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			stData.data.UnlockShared();
		}
		else if (valueType == TypeArray)
		{
			const VArray& a = this->Array();
			uint64_t len = a.Count();
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

			uint64_t len = a.Count();
			if (stream->Write(&len, sizeof(len)) != sizeof(len))
			{
				errorDesc = L"Failed to write to stream.";
				goto FAILED;
			}
			for (size_t i = 0; i < a.Count(); ++i)
			{
				const VPairs::Pair* pair = a.Value(i);
				DKStringU8 key(pair->key);

				uint64_t keyLen = key.Bytes();
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
			uint16_t operator () (uint16_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			uint32_t operator () (uint32_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			uint64_t operator () (uint64_t v) const
			{
				return this->littleEndian ? DKLittleEndianToSystem(v) : DKBigEndianToSystem(v);
			}
			bool littleEndian;
		} byteorder = { littleEndian };

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
			case TypeStructData:
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
					uint64_t v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					this->SetValueType(TypeInteger).Integer() = byteorder(v);
				}
				else if (type == TypeFloat)
				{
					uint64_t v;
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
					uint32_t v[2];
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
					uint32_t v[3];
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
					uint32_t v[4];
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
					uint32_t v[4];
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
					uint32_t v[9];
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
					uint32_t v[16];
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
					uint32_t v[4];
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
						int64_t n, d;
					} r = { 0LL, 0LL };

					if (stream->Read(&r, sizeof(r)) != sizeof(r))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}

					r.n = byteorder(static_cast<uint64_t>(r.n));
					r.d = byteorder(static_cast<uint64_t>(r.d));

					this->SetValueType(TypeRational).Rational() = VRational(r.n, r.d);
				}
				else if (type == TypeString)
				{
					uint64_t v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					uint64_t len = byteorder(v);
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
					this->SetValueType(TypeString).String() = static_cast<DKString&&>(str);
				}
				else if (type == TypeDateTime)
				{
					struct
					{
						uint64_t s;
						uint32_t ms;
					} dt = { 0LL, 0 };

					if (stream->Read(&dt, sizeof(dt)) != sizeof(dt))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}

					dt.s = byteorder(dt.s);
					dt.ms = byteorder(dt.ms);

					this->SetValueType(TypeDateTime).DateTime() = DKDateTime(dt.s, dt.ms);
				}
				else if (type == TypeData)
				{
					uint64_t v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					uint64_t len = byteorder(v);
					if (len > 0)
					{
						if (stream->RemainLength() >= len)
						{
							VData val;
							val.SetContent(0, len);
							void* p = val.LockExclusive();
							if (stream->Read(p, len) != len)
							{
								val.UnlockExclusive();
								errorDesc = L"Failed to read from stream.";
								goto FAILED;
							}
							val.UnlockExclusive();
							this->SetValueType(TypeData).Data() = static_cast<VData&&>(val);
						}
						else
						{
							errorDesc = L"Invalid stream length.";
							goto FAILED;
						}
					}
					else
						this->SetValueType(TypeData).Data().SetContent(0);
				}
				else if (type == TypeStructData)
				{
					struct
					{
						uint64_t elementSize;
						uint64_t numLayouts;
						uint64_t dataLength;
					} stInfo = { 0, 0, 0 };

					if (stream->Read(&stInfo, sizeof(stInfo)) != sizeof(stInfo))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					stInfo.elementSize = byteorder(stInfo.elementSize);
					stInfo.numLayouts = byteorder(stInfo.numLayouts);
					stInfo.dataLength = byteorder(stInfo.dataLength);

					VStructuredData stData;
					stData.elementSize = stInfo.elementSize;
					if (stInfo.numLayouts > 0)
					{
						if (stream->RemainLength() >= stInfo.numLayouts)
						{
							stData.layout.Resize(stInfo.numLayouts);
							if (stream->Read(stData.layout, stInfo.numLayouts) != stInfo.numLayouts)
							{
								errorDesc = L"Failed to read from stream.";
								goto FAILED;
							}
							if (!ValidateStructElementLayout(stData))
							{
								errorDesc = L"Failed to read from stream.";
								goto FAILED;
							}
						}
						else
						{
							errorDesc = L"Invalid stream length.";
							goto FAILED;
						}
					}
					if (stInfo.dataLength > 0)
					{
						if (stream->RemainLength() >= stInfo.dataLength)
						{
							stData.data.SetContent(0, stInfo.dataLength);
							void* p = stData.data.LockExclusive();
							if (stream->Read(p, stInfo.dataLength) != stInfo.dataLength)
							{
								stData.data.UnlockExclusive();
								errorDesc = L"Failed to read from stream.";
								goto FAILED;
							}
							stData.data.UnlockExclusive();
						}
						else
						{
							errorDesc = L"Invalid stream length.";
							goto FAILED;
						}
					}
					if (ConvertStructuredDataByteOrder(stData, littleEndian ? DKByteOrder::LittleEndian : DKByteOrder::BigEndian))
					{
						this->SetValueType(TypeStructData).StructuredData() = static_cast<VStructuredData&&>(stData);
					}
					else
					{
						errorDesc = L"Converting byte order failed.";
						goto FAILED;
					}
				}
				else if (type == TypeArray)
				{
					uint64_t v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					uint64_t len = byteorder(v);
					VArray val;
					if (len > 0)
					{
						val.Reserve(len);
						for (uint64_t i = 0; i < len; ++i)
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
					this->SetValueType(TypeArray).Array() = static_cast<VArray&&>(val);
				}
				else if (type == TypePairs)
				{
					uint64_t v;
					if (stream->Read(&v, sizeof(v)) != sizeof(v))
					{
						errorDesc = L"Failed to read from stream.";
						goto FAILED;
					}
					uint64_t len = byteorder(v);
					VPairs val;
					for (uint64_t i = 0; i < len; ++i)
					{
						uint64_t keyLen;
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
					this->SetValueType(TypePairs).Pairs() = static_cast<VPairs&&>(val);
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
#pragma pack(pop)

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
	return VariantBlock<VInteger, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VInteger& DKVariant::Integer(void) const
{
	return const_cast<DKVariant*>(this)->Integer();
}

DKVariant::VFloat& DKVariant::Float(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeFloat);
	DKASSERT_DEBUG(ValueType() == TypeFloat);
	return VariantBlock<VFloat, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VFloat& DKVariant::Float(void) const
{
	return const_cast<DKVariant*>(this)->Float();
}

DKVariant::VVector2& DKVariant::Vector2(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector2);
	DKASSERT_DEBUG(ValueType() == TypeVector2);
	return VariantBlock<VVector2, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector2& DKVariant::Vector2(void) const
{
	return const_cast<DKVariant*>(this)->Vector2();
}

DKVariant::VVector3& DKVariant::Vector3(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector3);
	DKASSERT_DEBUG(ValueType() == TypeVector3);
	return VariantBlock<VVector3, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector3& DKVariant::Vector3(void) const
{
	return const_cast<DKVariant*>(this)->Vector3();
}

DKVariant::VVector4& DKVariant::Vector4(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeVector4);
	DKASSERT_DEBUG(ValueType() == TypeVector4);
	return VariantBlock<VVector4, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VVector4& DKVariant::Vector4(void) const
{
	return const_cast<DKVariant*>(this)->Vector4();
}

DKVariant::VMatrix2& DKVariant::Matrix2(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix2);
	DKASSERT_DEBUG(ValueType() == TypeMatrix2);
	return VariantBlock<VMatrix2, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix2& DKVariant::Matrix2(void) const
{
	return const_cast<DKVariant*>(this)->Matrix2();
}

DKVariant::VMatrix3& DKVariant::Matrix3(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix3);
	DKASSERT_DEBUG(ValueType() == TypeMatrix3);
	return VariantBlock<VMatrix3, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix3& DKVariant::Matrix3(void) const
{
	return const_cast<DKVariant*>(this)->Matrix3();
}

DKVariant::VMatrix4& DKVariant::Matrix4(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeMatrix4);
	DKASSERT_DEBUG(ValueType() == TypeMatrix4);
	return VariantBlock<VMatrix4, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VMatrix4& DKVariant::Matrix4(void) const
{
	return const_cast<DKVariant*>(this)->Matrix4();
}

DKVariant::VQuaternion& DKVariant::Quaternion(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeQuaternion);
	DKASSERT_DEBUG(ValueType() == TypeQuaternion);
	return VariantBlock<VQuaternion, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VQuaternion& DKVariant::Quaternion(void) const
{
	return const_cast<DKVariant*>(this)->Quaternion();
}

DKVariant::VRational& DKVariant::Rational(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeRational);
	DKASSERT_DEBUG(ValueType() == TypeRational);
	return VariantBlock<VRational, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VRational& DKVariant::Rational(void) const
{
	return const_cast<DKVariant*>(this)->Rational();
}

DKVariant::VString& DKVariant::String(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeString);
	DKASSERT_DEBUG(ValueType() == TypeString);
	return VariantBlock<VString, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VString& DKVariant::String(void) const
{
	return const_cast<DKVariant*>(this)->String();
}

DKVariant::VDateTime& DKVariant::DateTime(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeDateTime);
	DKASSERT_DEBUG(ValueType() == TypeDateTime);
	return VariantBlock<VDateTime, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VDateTime& DKVariant::DateTime(void) const
{
	return const_cast<DKVariant*>(this)->DateTime();
}

DKVariant::VData& DKVariant::Data(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeData);
	DKASSERT_DEBUG(ValueType() == TypeData);
	return VariantBlock<VData, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VData& DKVariant::Data(void) const
{
	return const_cast<DKVariant*>(this)->Data();
}

DKVariant::VStructuredData& DKVariant::StructuredData(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeStructData);
	DKASSERT_DEBUG(ValueType() == TypeStructData);
	return VariantBlock<VStructuredData, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VStructuredData& DKVariant::StructuredData(void) const
{
	return const_cast<DKVariant*>(this)->StructuredData();
}

DKVariant::VArray& DKVariant::Array(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypeArray);
	DKASSERT_DEBUG(ValueType() == TypeArray);
	return VariantBlock<VArray, sizeof(vblock)>::Value(vblock);
}

const DKVariant::VArray& DKVariant::Array(void) const
{
	return const_cast<DKVariant*>(this)->Array();
}

DKVariant::VPairs& DKVariant::Pairs(void)
{
	if (ValueType() == TypeUndefined)	SetValueType(TypePairs);
	DKASSERT_DEBUG(ValueType() == TypePairs);
	return VariantBlock<VPairs, sizeof(vblock)>::Value(vblock);
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

DKVariant& DKVariant::SetStructuredData(const VStructuredData& v)
{
	SetValueType(TypeStructData).StructuredData() = v;
	return *this;
}

DKVariant& DKVariant::SetStructuredData(const void* p, size_t elementSize, size_t count, std::initializer_list<StructElem> layout)
{
	VStructuredData& data = SetValueType(TypeStructData).StructuredData();
	data.data.SetContent(p, elementSize * count);
	data.elementSize = elementSize;
	data.layout = layout;
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
	case TypeStructData:
		this->StructuredData() = v.StructuredData();
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
				size_t length = this->Data().Length();
				const void* ptr1 = this->Data().LockShared();
				const void* ptr2 = v.Data().LockShared();

				result = memcmp(ptr1, ptr2, length) == 0;

				this->Data().UnlockShared();
				v.Data().UnlockShared();
			}
			break;
		case TypeStructData:
			if (true)
			{
				const VStructuredData& s1 = this->StructuredData();
				const VStructuredData& s2 = v.StructuredData();
				if (s1.elementSize == s2.elementSize &&
					s1.layout.Count() == s2.layout.Count() &&
					s1.data.Length() == s2.data.Length())
				{
					result = true;
					for (size_t i = 0; i < s1.layout.Count(); ++i)
					{
						if (s1.layout.Value(i) != s2.layout.Value(i))
						{
							result = false;
							break;
						}
					}
					if (result)
					{
						size_t length = s1.data.Length();
						const void* ptr1 = s1.data.LockShared();
						const void* ptr2 = s2.data.LockShared();

						result = memcmp(ptr1, ptr2, length) == 0;

						s1.data.UnlockShared();
						s2.data.UnlockShared();
					}
				}
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

				this->Pairs().EnumerateForward([&pairs1](const VPairs::Pair& pair) {pairs1.Add(&pair);});
				v.Pairs().EnumerateForward([&pairs2](const VPairs::Pair& pair) {pairs2.Add(&pair);});

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
