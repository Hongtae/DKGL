//
//  File: DKXMLDocument.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKinclude.h"
#include "DKObject.h"
#include "DKString.h"
#include "DKData.h"
#include "DKXMLParser.h"

////////////////////////////////////////////////////////////////////////////////
// DKXMLDocument
// XML DOM class, provides parse and generate DOM of XML, HTML.
// this class uses DKXMLParser internally. (see DKXMLParser.h)
// this class provides DOM includes DTD.
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	class DKLIB_API DKXMLDocument
	{
	public:
		enum Type
		{
			TypeXML,
			TypeHTML,
		};
		struct Namespace
		{
			DKString	prefix;
			DKString	URI;
		};
		struct Attribute
		{
			DKObject<Namespace>	ns;
			DKString			name;
			DKString			value;
		};
		class Node
		{
		public:
			virtual ~Node(void) {}
			enum NodeType
			{
				NodeTypeUnknown = 0,
				NodeTypeInstruction,
				NodeTypeDocTypeDecl,
				NodeTypeElementDecl,
				NodeTypeAttributeDecl,
				NodeTypeComment,
				NodeTypeElement,
				NodeTypeCData,  // CDATA section strings ignored by parser.
				NodeTypePCData, // Parsed Character Data. (some symbols will be translated.)
			};
			NodeType			Type(void) const;
			virtual DKString	Export(void) const = 0;
		protected:
			Node(NodeType t) : type(t) {}
		private:
			const NodeType	type;
		};
		struct CData : public Node
		{
			CData(void) : Node(NodeTypeCData) {}
			DKStringU8 value;		// UTF-8 string
			DKString Export(void) const;
		};
		struct PCData : public Node
		{
			PCData(void) : Node(NodeTypePCData) {}
			DKString value;
			DKString Export(void) const;
		};
		struct Comment : public Node
		{
			Comment(void) : Node(NodeTypeComment) {}
			DKString value;
			DKString Export(void) const;
		};
		struct Element : public Node
		{
			Element(void) : Node(NodeTypeElement) {}
			DKObject<Namespace>				ns;
			DKString						name;
			DKArray<Attribute>				attributes;
			DKArray<Namespace> 				namespaces;
			DKArray<DKObject<Node>>		nodes;
			DKString Export(void) const;
			DKString Export(DKArray<Namespace>& writtenNS) const;
		};
		struct Instruction : public Node
		{
			Instruction(void) : Node(NodeTypeInstruction) {}
			DKString						target;
			DKString						data;
			DKString Export(void) const;
		};
		struct ElementDecl : public Node		// DTD Element
		{
			ElementDecl(void) : Node(NodeTypeElementDecl) {}
			DKXMLParser::ElementDecl		decl;
			DKXMLParser::ElementContentDecl	contents;
			DKString Export(void) const;
		};
		struct AttributeDecl : public Node	// DTD
		{
			AttributeDecl(void) : Node(NodeTypeAttributeDecl) {}
			DKXMLParser::AttributeDecl		decl;
			DKArray<DKString>				enumeratedValues;
			DKString Export(void) const;
		};
		struct DocTypeDecl : public Node
		{
			DocTypeDecl(void) : Node(NodeTypeDocTypeDecl) {}
			DKString						name;
			DKString						externalID;
			DKString						systemID;
			DKArray<DKObject<Node>>		nodes;
			DKString Export(void) const;
		};
		DKXMLDocument(void);
		DKXMLDocument(DocTypeDecl* dtd, Element* root);
		DKXMLDocument(Element* root);
		~DKXMLDocument(void);

		// open and create object with URL or file.
		static DKObject<DKXMLDocument> Open(Type t, const DKString& fileOrURL, DKString* desc = NULL);

		// When reading HTML from buffer, they should be encoded with UTF-8.
		// becouse XML has encoding information, but HTML does not.
		static DKObject<DKXMLDocument> Open(Type t, const DKData* buffer, DKString* desc = NULL);
		static DKObject<DKXMLDocument> Open(Type t, DKStream* stream, DKString* desc = NULL);

		DKObject<DKData> Export(DKStringEncoding e) const;
		size_t Export(DKStringEncoding e, DKStream* output) const;

		////////////////////////////////////////////////////////////////////////////////
		Element*			RootElement(void);
		const Element*		RootElement(void) const;
		void				SetRootElement(Element* e);				
		// DTD subset
		void				SetDTD(DocTypeDecl* d);				
		DocTypeDecl*		DTD(void);
		const DocTypeDecl*	DTD(void) const;

	private:
		DKArray<DKObject<Node>>		nodes; // all nodes of DOM.
		class DocumentBuilder;
	};

	typedef DKXMLDocument::Namespace		DKXMLNamespace;
	typedef DKXMLDocument::Attribute		DKXMLAttribute;
	typedef DKXMLDocument::Node				DKXMLNode;
	typedef DKXMLDocument::CData			DKXMLCData;
	typedef DKXMLDocument::PCData			DKXMLPCData;
	typedef DKXMLDocument::Comment			DKXMLComment;
	typedef DKXMLDocument::Element			DKXMLElement;
	typedef DKXMLDocument::Instruction		DKXMLInstruction;
	typedef DKXMLDocument::ElementDecl		DKXMLElementDecl;
	typedef DKXMLDocument::AttributeDecl	DKXMLAttributeDecl;
	typedef DKXMLDocument::DocTypeDecl		DKXMLDocTypeDecl;
}
