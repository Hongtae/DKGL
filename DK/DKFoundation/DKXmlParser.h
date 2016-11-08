//
//  File: DKXmlParser.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKString.h"
#include "DKBuffer.h"
#include "DKStream.h"
#include "DKArray.h"

////////////////////////////////////////////////////////////////////////////////
// DKXmlParser
// a SAX parser, You need subclass to define behaviors while parsing.
// this class provides parsing DTD.
////////////////////////////////////////////////////////////////////////////////

namespace DKGL
{
	class DKGL_API DKXmlParser
	{
	public:
		DKXmlParser(void);
		virtual ~DKXmlParser(void);

		bool BeginHtml(const DKString& url);
		bool BeginHtml(const DKData* buffer);
		bool BeginHtml(DKStream* stream);
		bool BeginXml(const DKString& url);
		bool BeginXml(const DKData* buffer);
		bool BeginXml(DKStream* stream);

		struct Namespace
		{
			DKString	prefix;
			DKString	URI;
		};
		struct Attribute
		{
			DKString	name;
			DKString	prefix;
			DKString	URI;
			DKString	value;
		};
		struct Element
		{
			DKString	name;
			DKString	prefix;
			DKString	URI;
		};
		struct ElementDecl
		{
			enum Type
			{
				TypeUndefined,
				TypeEmpty,
				TypeAny,
				TypeMixed,				// #PCDATA
				TypeElement,
			};
			DKString	name;
			Type		type;
		};
		struct ElementContentDecl
		{
			enum Type
			{
				TypeUndefined,
				TypePCData,
				TypeElement,
				TypeSequence,			// (A,B,..)
				TypeAlternative,		// (A|B|..)
			};
			enum Occurrence
			{
				OccurrenceNoneOrMore,		// *
				OccurrenceOnceOrMore,		// +
				OccurrenceOnceOrNone,		// ?
				OccurrenceOnce,				//
			};
			Type			type;
			DKString		name;
			DKString		prefix;		// namespace
			Occurrence		occurrence;
			DKArray<ElementContentDecl>	contents;
		};
		struct AttributeDecl
		{
			enum Type
			{
				TypeCData,
				TypeID,
				TypeIDRef,
				TypeIDRefs,
				TypeEntity,
				TypeEntities,
				TypeNMToken,
				TypeNMTokens,
				TypeEnumeration,
				TypeNotation,
			};
			enum ValueType
			{
				ValueTypeNone,
				ValueTypeRequired,
				ValueTypeImplied,
				ValueTypeFixed,
			};
			DKString	element;
			DKString	name;
			Type		type;
			DKString	defaultValue;
			ValueType	defaultValueType;
		};
	public:
		static DKString FormatElementContent(const DKXmlParser::ElementContentDecl& content);

		// following virtual functions should be overridden.
		virtual void OnProcessingInstruction(const DKString& target, const DKString& data);
		virtual void OnInternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID);
		virtual void OnExternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID);
		virtual void OnEntityDeclaration(const DKString& name, int type, const DKString& publicID, const DKString& systemID, const DKString& content);
		virtual void OnAttributeDeclaration(const AttributeDecl& attr, const DKArray<DKString>& enumeratedValues);
		virtual void OnElementDeclaration(const ElementDecl& elem, const ElementContentDecl& content);
		virtual void OnStartDocument(void);
		virtual void OnEndDocument(void);
		virtual void OnStartElement(const Element& element, const DKArray<Namespace>& namespaces, const DKArray<Attribute>& attributes);
		virtual void OnEndElement(const Element& element);
		virtual void OnComment(const DKString& comment);
		virtual void OnCharacters(const char* ch, size_t len);         // UTF-8 string.
		virtual void OnCharacterDataBlock(const char* ch, size_t len); // UTF-8 string.
		virtual void OnWarning(const DKString& mesg);
		virtual void OnError(const DKString& mesg);
		virtual void OnFatalError(const DKString& mesg);

		DKArray<void*> customEntityStorage;
	private:
		void ClearCustomEntityStorage(void);
	};
}

