//
//  File: DKXMLParser.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#define DKLIB_EXTDEPS_LIBXML
#include <string.h>
#include <memory.h>
#include "../lib/ExtDeps.h"
#include "DKXMLParser.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKDataStream.h"

namespace DKFoundation
{
	namespace Private
	{
		namespace
		{
			inline DKString xmlCharToIGString(const xmlChar* c)
			{
				return DKString((const DKUniChar8*)c);
			}
			inline DKString xmlCharToIGString(const xmlChar* c, size_t len)
			{
				return DKString((const DKUniChar8*)c, len);
			}
			void internalSubset(void *ctx, const xmlChar *name, const xmlChar *ExternalID, const xmlChar *SystemID)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnInternalSubsetDeclaration(xmlCharToIGString(name), xmlCharToIGString(ExternalID), xmlCharToIGString(SystemID));
			}
			int isStandalone(void *ctx)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.isStandalone()\n");
				return 0;
			}
			int hasInternalSubset(void *ctx)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.hasInternalSubset()\n");
				return 0;
			}
			int hasExternalSubset(void *ctx)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.hasExternalSubset()\n");
				return 0;
			}
			xmlParserInputPtr resolveEntity(void *ctx, const xmlChar *publicId, const xmlChar *systemId)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				/* xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx; */
				DKString text = DKString(L"SAX.resolveEntity(");
				if (publicId != NULL)
					text += DKString::Format("%ls", (const wchar_t*)xmlCharToIGString(publicId));
				else
					text += DKString(L" ");
				if (systemId != NULL)
					text += DKString::Format(", %ls)\n", (const wchar_t*)xmlCharToIGString(systemId));
				else
					text += DKString(L", )\n");
				/*********
				if (systemId != NULL) {
				return(xmlNewInputFromFile(ctxt, (char *) systemId));
				}
				*********/
				DKLog(text);
				return NULL;
			}
			xmlEntityPtr getEntity(void *ctx, const xmlChar *name)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKLog("SAX.getEntity(%ls)\n", (const wchar_t*)xmlCharToIGString(name));
				//return NULL;
				if (parser->customEntityStorage.Count() > 0)
				{
					for (int i = 0; i < parser->customEntityStorage.Count(); i++)
					{
						if (strcmp((const char*)((xmlEntityPtr)parser->customEntityStorage.Value(i))->name,
							(const char*)name) == 0)
							return (xmlEntityPtr)(parser->customEntityStorage.Value(i));
					}
				}
				return NULL;
			}
			void entityDecl(void *ctx, const xmlChar *name, int type, const xmlChar *publicId, const xmlChar *systemId, xmlChar *content)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				const xmlChar *nullstr = BAD_CAST "(null)";
				/* not all libraries handle printing null pointers nicely */
				if (publicId == NULL)
					publicId = nullstr;
				if (systemId == NULL)
					systemId = nullstr;
				if (content == NULL)
					content = (xmlChar *)nullstr;

				DKLog("SAX.entityDecl(%ls, %d, %ls, %ls, %ls)\n",
					(const wchar_t*)xmlCharToIGString(name),
					type, 
					(const wchar_t*)xmlCharToIGString(publicId),
					(const wchar_t*)xmlCharToIGString(systemId),
					(const wchar_t*)xmlCharToIGString(content));

				xmlEntityPtr e = xmlNewEntity(NULL, name, type, publicId, systemId, content);
				if (e)
				{
					parser->customEntityStorage.Add(e);
				}
			}
			void notationDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKLog("SAX.notationDecl(%ls, %ls, %ls)\n",
					(const wchar_t*)xmlCharToIGString(name),
					(const wchar_t*)xmlCharToIGString(publicId),
					(const wchar_t*)xmlCharToIGString(systemId));
			}
			void attributeDecl(void *ctx, const xmlChar * elem, const xmlChar * name, int type, int def, const xmlChar * defaultValue, xmlEnumerationPtr tree)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKXMLParser::AttributeDecl::Type			attributeType;		// attribute type
				DKXMLParser::AttributeDecl::ValueType		defaultValueType;	// default value type
				switch (type)
				{
				case XML_ATTRIBUTE_CDATA:			attributeType = DKXMLParser::AttributeDecl::TypeCData;			break;
				case XML_ATTRIBUTE_ID:				attributeType = DKXMLParser::AttributeDecl::TypeID;				break;
				case XML_ATTRIBUTE_IDREF:			attributeType = DKXMLParser::AttributeDecl::TypeIDRef;			break;
				case XML_ATTRIBUTE_IDREFS:			attributeType = DKXMLParser::AttributeDecl::TypeIDRefs;			break;
				case XML_ATTRIBUTE_ENTITY:			attributeType = DKXMLParser::AttributeDecl::TypeEntity;			break;
				case XML_ATTRIBUTE_ENTITIES:		attributeType = DKXMLParser::AttributeDecl::TypeEntities;		break;
				case XML_ATTRIBUTE_NMTOKEN:			attributeType = DKXMLParser::AttributeDecl::TypeNMToken;		break;
				case XML_ATTRIBUTE_NMTOKENS:		attributeType = DKXMLParser::AttributeDecl::TypeNMTokens;		break;
				case XML_ATTRIBUTE_ENUMERATION:		attributeType = DKXMLParser::AttributeDecl::TypeEnumeration;	break;
				case XML_ATTRIBUTE_NOTATION:		attributeType = DKXMLParser::AttributeDecl::TypeNotation;		break;
				}

				DKString defString; // type of default value.
				switch (def)
				{
				case XML_ATTRIBUTE_NONE:			defaultValueType = DKXMLParser::AttributeDecl::ValueTypeNone;		break;
				case XML_ATTRIBUTE_REQUIRED:		defaultValueType = DKXMLParser::AttributeDecl::ValueTypeRequired;	break;
				case XML_ATTRIBUTE_IMPLIED:			defaultValueType = DKXMLParser::AttributeDecl::ValueTypeImplied;	break;
				case XML_ATTRIBUTE_FIXED:			defaultValueType = DKXMLParser::AttributeDecl::ValueTypeFixed;		break;
				}

				DKArray<DKString>	enumList;
				for (xmlEnumerationPtr p = tree; p != NULL; p = p->next)
					enumList.Add(DKString((const DKUniChar8*)p->name));

				DKXMLParser::AttributeDecl	attr;
				attr.name = xmlCharToIGString(name);
				attr.element = xmlCharToIGString(elem);
				attr.type = attributeType;
				attr.defaultValueType = defaultValueType;
				attr.defaultValue = xmlCharToIGString(defaultValue);

				parser->OnAttributeDeclaration(attr, enumList);

				xmlFreeEnumeration(tree);
			}
			void ConvertElementDeclContent(xmlElementContentPtr from, DKXMLParser::ElementContentDecl& to)
			{
				if (from == NULL)
				{
					to.type = DKXMLParser::ElementContentDecl::TypeUndefined;
					return;
				}

				to.name = xmlCharToIGString(from->name);
				to.prefix = xmlCharToIGString(from->prefix);

				switch (from->type)
				{
				case XML_ELEMENT_CONTENT_PCDATA:	to.type = DKXMLParser::ElementContentDecl::TypePCData;			break;
				case XML_ELEMENT_CONTENT_ELEMENT:	to.type = DKXMLParser::ElementContentDecl::TypeElement;			break;
				case XML_ELEMENT_CONTENT_SEQ:		to.type = DKXMLParser::ElementContentDecl::TypeSequence;		break;
				case XML_ELEMENT_CONTENT_OR:		to.type = DKXMLParser::ElementContentDecl::TypeAlternative;		break;
				default:
					to.type = DKXMLParser::ElementContentDecl::TypeUndefined;
					return;
				}
				switch (from->ocur)
				{
				case XML_ELEMENT_CONTENT_ONCE:		to.occurrence = DKXMLParser::ElementContentDecl::OccurrenceOnce;		break;
				case XML_ELEMENT_CONTENT_OPT:		to.occurrence = DKXMLParser::ElementContentDecl::OccurrenceOnceOrNone;	break;
				case XML_ELEMENT_CONTENT_MULT:		to.occurrence = DKXMLParser::ElementContentDecl::OccurrenceNoneOrMore;	break;
				case XML_ELEMENT_CONTENT_PLUS:		to.occurrence = DKXMLParser::ElementContentDecl::OccurrenceOnceOrMore;	break;
				}

				if (from->c1)
				{
					DKXMLParser::ElementContentDecl decl;
					ConvertElementDeclContent(from->c1, decl);
					if (decl.type != DKXMLParser::ElementContentDecl::TypeUndefined)
					{
						if (decl.type == to.type && decl.occurrence == DKXMLParser::ElementContentDecl::OccurrenceOnce)
							to.contents.Add(decl.contents);
						else
							to.contents.Add(decl);
					}
				}
				if (from->c2)
				{
					DKXMLParser::ElementContentDecl decl;
					ConvertElementDeclContent(from->c2, decl);
					if (decl.type != DKXMLParser::ElementContentDecl::TypeUndefined)
					{
						if (decl.type == to.type && decl.occurrence == DKXMLParser::ElementContentDecl::OccurrenceOnce)
							to.contents.Add(decl.contents);
						else
							to.contents.Add(decl);
					}
				}
			}
			void elementDecl(void *ctx, const xmlChar *name, int type, xmlElementContentPtr content)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				// 'content' contains the value, parsed by parser as tree-structured.
				// the value includes conditions between element and element.
				//
				// if element is <!ELEMENT ele (a | b | c)>,
				// structured value becomes to 'A or (B or C)'
				// For given element is '(a | b | c | d)',
				// structured value become to 'A or (B or (C or D))'

				DKXMLParser::ElementDecl		elem;
				DKXMLParser::ElementContentDecl	ec;

				elem.name = DKString((const DKUniChar8*)name);
				switch (type)
				{
				case XML_ELEMENT_TYPE_EMPTY:		elem.type = DKXMLParser::ElementDecl::TypeEmpty;		break;
				case XML_ELEMENT_TYPE_ANY:			elem.type = DKXMLParser::ElementDecl::TypeAny;			break;
				case XML_ELEMENT_TYPE_MIXED:		elem.type = DKXMLParser::ElementDecl::TypeMixed;		break;
				case XML_ELEMENT_TYPE_ELEMENT:		elem.type = DKXMLParser::ElementDecl::TypeElement;		break;
				default:							elem.type = DKXMLParser::ElementDecl::TypeUndefined;	break;
				}
				ConvertElementDeclContent(content, ec);
				parser->OnElementDeclaration(elem, ec);
			}
			void unparsedEntityDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId, const xmlChar *notationName)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				const xmlChar *nullstr = BAD_CAST "(null)";

				if (publicId == NULL)
					publicId = nullstr;
				if (systemId == NULL)
					systemId = nullstr;
				if (notationName == NULL)
					notationName = nullstr;

				DKLog("SAX.unparsedEntityDecl(%ls, %ls, %ls, %ls)\n", 
					(const wchar_t*)xmlCharToIGString(name),
					(const wchar_t*)xmlCharToIGString(publicId),
					(const wchar_t*)xmlCharToIGString(systemId),
					(const wchar_t*)xmlCharToIGString(notationName));
			}
			void setDocumentLocator(void *ctx, xmlSAXLocatorPtr loc)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				//DKLog("SAX.setDocumentLocator()\n");
			}
			void startDocument(void *ctx)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnStartDocument();
			}
			void endDocument(void *ctx)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnEndDocument();
			}
			void startElement(void *ctx, const xmlChar *name, const xmlChar **atts)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKXMLParser::Element element;
				element.name = DKString((const DKUniChar8*)name);

				DKArray<DKXMLParser::Attribute>	attr;
				if (atts != NULL)
				{
					for (int i = 0; atts[i] != NULL; i++)
					{
						DKXMLParser::Attribute at;
						at.name = xmlCharToIGString(atts[i]);
						i++;
						at.value = xmlCharToIGString(atts[i]);

						attr.Add(at);
					}					 
				}

				parser->OnStartElement(element, DKArray<DKXMLParser::Namespace>(), attr);
			}
			void endElement(void *ctx, const xmlChar *name)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKXMLParser::Element element;
				element.name = xmlCharToIGString(name);
				parser->OnEndElement(element);
			}
			void reference(void *ctx, const xmlChar *name)
			{
				// 엔티티가 characters 로 출력되고 나서 호출된다.
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.reference(%ls)\n", (const wchar_t*)xmlCharToIGString(name));
			}
			void characters(void *ctx, const xmlChar *ch, int len)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnCharacters(reinterpret_cast<const char*>(ch), len);
			}
			void ignorableWhitespace(void *ctx, const xmlChar *ch, int len)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.ignorableWhitespace: %d bytes\n", len);
			}
			void processingInstruction(void *ctx, const xmlChar *target, const xmlChar *data)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnProcessingInstruction(xmlCharToIGString(target), xmlCharToIGString(data));
			}
			void comment(void *ctx, const xmlChar *value)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnComment(xmlCharToIGString(value));
			}
			void XMLCDECL warning(void *ctx, const char *msg, ...)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnWarning(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			void XMLCDECL error(void *ctx, const char *msg, ...)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnError(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			void XMLCDECL fatalError(void *ctx, const char *msg, ...)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnFatalError(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			xmlEntityPtr getParameterEntity(void *ctx, const xmlChar *name)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				DKLog("SAX.getParameterEntity(%ls)\n", (const wchar_t*)xmlCharToIGString(name));
				return NULL;
			}
			void cdataBlock(void *ctx, const xmlChar *value, int len)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnCharacterDataBlock(reinterpret_cast<const char*>(value), len);
			}
			void externalSubset(void *ctx, const xmlChar *name, const xmlChar *ExternalID, const xmlChar *SystemID)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);
				parser->OnExternalSubsetDeclaration(xmlCharToIGString(name), xmlCharToIGString(ExternalID), xmlCharToIGString(SystemID));
			}
			void startElementNs(void *ctx,
				const xmlChar *localname,
				const xmlChar *prefix,
				const xmlChar *URI,
				int nb_namespaces,
				const xmlChar **namespaces,		// pointer to the array of prefix/URI pairs namespace definitions
				int nb_attributes,
				int nb_defaulted,
				const xmlChar **attributes)		// pointer to the array of (localname/prefix/URI/value/end)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKXMLParser::Element	element;
				element.name = xmlCharToIGString(localname);
				element.prefix = xmlCharToIGString(prefix);
				element.URI = xmlCharToIGString(URI);

				DKArray<DKXMLParser::Namespace>	ns;
				if (nb_namespaces)
				{
					ns.Resize(nb_namespaces);
					for (int i = 0; i < nb_namespaces; i++)
					{
						ns.Value(i).prefix = xmlCharToIGString(namespaces[i*2]);
						ns.Value(i).URI = xmlCharToIGString(namespaces[i*2+1]);
					}
				}
				DKArray<DKXMLParser::Attribute> attrs;
				if (attributes)
				{
					attrs.Resize(nb_attributes);
					for (int i = 0; i < nb_attributes; i++)
					{
						attrs.Value(i).name = xmlCharToIGString(attributes[i*5]);
						attrs.Value(i).prefix = xmlCharToIGString(attributes[i*5+1]);
						attrs.Value(i).URI = xmlCharToIGString(attributes[i*5+2]);
						attrs.Value(i).value = xmlCharToIGString(attributes[i*5+3], (size_t)((const char*)attributes[i*5+4] - (const char*)attributes[i*5+3]));
					}
				}
				parser->OnStartElement(element, ns, attrs);
			}
			void endElementNs(void *ctx, const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI)
			{
				DKXMLParser* parser = reinterpret_cast<DKXMLParser*>(ctx);

				DKXMLParser::Element	element;
				element.name = xmlCharToIGString(localname);
				element.prefix = xmlCharToIGString(prefix);
				element.URI = xmlCharToIGString(URI);

				parser->OnEndElement(element);
			}

			xmlSAXHandler emptySAXHandler = {
				NULL, /* internalSubset */
				NULL, /* isStandalone */
				NULL, /* hasInternalSubset */
				NULL, /* hasExternalSubset */
				NULL, /* resolveEntity */
				NULL, /* getEntity */
				NULL, /* entityDecl */
				NULL, /* notationDecl */
				NULL, /* attributeDecl */
				NULL, /* elementDecl */
				NULL, /* unparsedEntityDecl */
				NULL, /* setDocumentLocator */
				NULL, /* startDocument */
				NULL, /* endDocument */
				NULL, /* startElement */
				NULL, /* endElement */
				NULL, /* reference */
				NULL, /* characters */
				NULL, /* ignorableWhitespace */
				NULL, /* processingInstruction */
				NULL, /* comment */
				NULL, /* xmlParserWarning */
				NULL, /* xmlParserError */
				NULL, /* xmlParserError */
				NULL, /* getParameterEntity */
				NULL, /* cdataBlock; */
				NULL, /* externalSubset; */
				1,
				NULL,
				NULL, /* startElementNs */
				NULL, /* endElementNs */
				NULL  /* xmlStructuredErrorFunc */
			};

			xmlSAXHandler defaultSAX2Handler = {
				internalSubset,
				isStandalone,
				hasInternalSubset,
				hasExternalSubset,
				resolveEntity,
				getEntity,
				entityDecl,
				notationDecl,
				attributeDecl,
				elementDecl,
				unparsedEntityDecl,
				setDocumentLocator,
				startDocument,
				endDocument,
				startElement, // use startElementNs for SAX2
				endElement,   // use endElementNs for SAX2
				reference,
				characters,
				ignorableWhitespace,
				processingInstruction,
				comment,
				warning,
				error,
				fatalError,
				getParameterEntity,
				cdataBlock,
				externalSubset,
				XML_SAX2_MAGIC,
				NULL,
				startElementNs, // called after element tag has been processed.
				endElementNs,   // called after tag has been processed.
				NULL			/* xmlStructuredErrorFunc */
			};
		}
	}
}

using namespace DKFoundation;

DKXMLParser::DKXMLParser(void)
{

}

DKXMLParser::~DKXMLParser(void)
{
	ClearCustomEntityStorage();
}

bool DKXMLParser::BeginHTML(const DKString& url)
{
	if (url.Length() == 0)
		return false;

	DKStringU8 filenameUTF8(url);
	if (filenameUTF8.Bytes() == 0)
		return false;

	ClearCustomEntityStorage();

	htmlDocPtr doc = htmlSAXParseFile((const char*)filenameUTF8, 0, &Private::defaultSAX2Handler, this);
	if (doc == NULL)
	{
		return true;
	}
	if (doc != NULL)
	{
		//DKLog("htmlSAXParseFile returned non-NULL\n");
		xmlFreeDoc(doc);
	}
	return false;
}

bool DKXMLParser::BeginHTML(const DKData* data)
{
	bool result = false;
	if (data)
	{
		const void* buffer = data->LockShared();
		size_t bufferSize = data->Length();

		if (buffer && bufferSize > 0)
		{
			ClearCustomEntityStorage();

			if (((const char*)buffer)[bufferSize -1])		// non-null terminated
			{
				char* text = (char*)DKMemoryHeapAlloc(bufferSize + 4);
				memset(text, 0, bufferSize+4);
				memcpy(text, buffer, bufferSize);

				htmlDocPtr doc = htmlSAXParseDoc((xmlChar*)text, 0, &Private::defaultSAX2Handler, this);

				DKMemoryHeapFree(text);

				if (doc == NULL)
				{
					result = true;
				}
				else
				{
					//DKLog("htmlSAXParseDoc returned non-NULL\n");
					xmlFreeDoc(doc);
					result = false;
				}
			}
			else		// null terminated buffer
			{
				htmlDocPtr doc = htmlSAXParseDoc((xmlChar*)buffer, 0, &Private::defaultSAX2Handler, this);

				if (doc == NULL)
				{
					result = true;
				}
				else
				{
					//DKLog("htmlSAXParseDoc returned non-NULL\n");
					xmlFreeDoc(doc);
					result = false;
				}
			}
		}
		data->UnlockShared();
	}
	return result;
}

bool DKXMLParser::BeginHTML(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
		if (ds)
			return BeginHTML(ds->DataSource());
		return BeginHTML(DKBuffer::Create(stream));
	}
	return false;
}

bool DKXMLParser::BeginXML(const DKString& url)
{
	if (url.Length() == 0)
		return false;

	DKStringU8 filenameUTF8(url);
	if (filenameUTF8.Bytes() == 0)
		return false;

	ClearCustomEntityStorage();

	int ret = xmlSAXUserParseFile(&Private::defaultSAX2Handler, this, (const char*)filenameUTF8);
	if (ret == XML_ERR_OK)
	{
		return true;
	}
	//DKLog("xmlSAXUserParseFile returns error: %d\n", ret);
	return false;
}

bool DKXMLParser::BeginXML(const DKData* data)
{
	if (data)
	{
		const char* buffer = reinterpret_cast<const char*>(data->LockShared());
		size_t bufferSize = data->Length();

		bool result = false;
		if (buffer && bufferSize > 0)
		{
			ClearCustomEntityStorage();
			int ret = xmlSAXUserParseMemory(&Private::defaultSAX2Handler, this, buffer, bufferSize);
			if (ret == XML_ERR_OK)
			{
				result = true;
			}
			else
			{
				//DKLog("xmlSAXUserParseMemory returns error: %d\n", ret);
				result = false;
			}			
		}
		data->UnlockShared();

		return result;
	}
	return false;
}

bool DKXMLParser::BeginXML(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
		if (ds)
			return BeginXML(ds->DataSource());
		return BeginXML(DKBuffer::Create(stream));
	}
	return false;
}

void DKXMLParser::OnProcessingInstruction(const DKString& target, const DKString& data)
{
	DKString txt(L"[DKXMLParser::OnProcessingInstruction]");
	txt += DKString(L" Target:") + target;
	txt += DKString(L" Data:") + data;
	txt += L"\n";
	DKLog(txt);
}

void DKXMLParser::OnInternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
{
	DKString txt(L"[DKXMLParser::OnInternalSubset]");
	txt += DKString(L" Name:") + name;
	txt += DKString(L" ExternalID:") + externalID;
	txt += DKString(L" SystemID:") + systemID;
	txt += L"\n";
	DKLog(txt);
}

void DKXMLParser::OnExternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
{
	DKString txt(L"[DKXMLParser::OnExternalSubset]");
	txt += DKString(L" Name:") + name;
	txt += DKString(L" ExternalID:") + externalID;
	txt += DKString(L" SystemID:") + systemID;
	txt += L"\n";
	DKLog(txt);
}
void DKXMLParser::OnEntityDeclaration(const DKString& name, int type, const DKString& publicID, const DKString& systemID, const DKString& content)
{

}

void DKXMLParser::OnAttributeDeclaration(const AttributeDecl& attr, const DKArray<DKString>& enumeratedValues)
{
	DKString attType;
	DKString defType;
	switch (attr.type)
	{
	case AttributeDecl::TypeCData:				attType = L"CDATA";			break;
	case AttributeDecl::TypeID:					attType = L"ID";			break;
	case AttributeDecl::TypeIDRef:				attType = L"IDRef";			break;
	case AttributeDecl::TypeIDRefs:				attType = L"IDRefs";		break;
	case AttributeDecl::TypeEntity:				attType = L"Entity";		break;
	case AttributeDecl::TypeEntities:			attType = L"Entities";		break;
	case AttributeDecl::TypeNMToken:			attType = L"NMToken";		break;
	case AttributeDecl::TypeNMTokens:			attType = L"NMTokens";		break;
	case AttributeDecl::TypeEnumeration:		attType = L"Enumeration";	break;
	case AttributeDecl::TypeNotation:			attType = L"Notation";		break;
	}
	switch (attr.defaultValueType)
	{
	case AttributeDecl::ValueTypeNone:			defType = L"None";			break;
	case AttributeDecl::ValueTypeRequired:		defType = L"Required";		break;
	case AttributeDecl::ValueTypeImplied:		defType = L"Implied";		break;
	case AttributeDecl::ValueTypeFixed:			defType = L"Fixed";			break;
	}

	DKString txt(L"[DKXMLParser::OnAttributeDeclaration]");
	txt += DKString(L" name:") + attr.name;
	txt += DKString(L" element:") + attr.element;
	txt += DKString(L" type:") + attType;
	txt += DKString(L" default:") + attr.defaultValue + L"(" + defType + L")";
	txt += DKString::Format(" enumValueCount: %d", enumeratedValues.Count());
	txt += L"\n";
	DKLog(txt);
}

DKString DKXMLParser::FormatElementContent(const DKXMLParser::ElementContentDecl& content)
{
	if (content.type == DKXMLParser::ElementContentDecl::TypeUndefined)
		return L"";

	DKString ret = L"";

	if (content.prefix.Length() > 0)
		ret += content.prefix + L":";

	switch (content.type)
	{
	case DKXMLParser::ElementContentDecl::TypePCData:			ret += L"#PCDATA";		break;
	case DKXMLParser::ElementContentDecl::TypeElement:			ret += content.name;	break;
	case DKXMLParser::ElementContentDecl::TypeSequence:			// (A,B,..)
	case DKXMLParser::ElementContentDecl::TypeAlternative:		// (A|B|..)
		break;
	}

	if (content.type == DKXMLParser::ElementContentDecl::TypeSequence ||
		content.type == DKXMLParser::ElementContentDecl::TypeAlternative)
	{
		ret += L"(";
		for (int i = 0; i < content.contents.Count(); i++)
		{
			if ( i > 0)
			{
				if (content.type == DKXMLParser::ElementContentDecl::TypeSequence)
					ret += L",";
				else
					ret += L"|";
			}
			ret += FormatElementContent(content.contents.Value(i));
		}
		ret += L")";
	}
	switch (content.occurrence)
	{
	case DKXMLParser::ElementContentDecl::OccurrenceNoneOrMore:		ret += L"*";	break;		// *
	case DKXMLParser::ElementContentDecl::OccurrenceOnceOrMore:		ret += L"+";	break;		// +
	case DKXMLParser::ElementContentDecl::OccurrenceOnceOrNone:		ret += L"?";	break;		// ?
	case DKXMLParser::ElementContentDecl::OccurrenceOnce:
		break;
	}
	return ret;
}

void DKXMLParser::OnElementDeclaration(const ElementDecl& elem, const ElementContentDecl& content)
{
	DKString txt(L"[DKXMLParser::OnElementDeclaration]");
	txt += DKString(L" name:") + elem.name;
	txt += DKString(L" type:");
	switch (elem.type)
	{
	case ElementDecl::TypeEmpty:	txt += L"EMPTY";		break;
	case ElementDecl::TypeAny:		txt += L"ANY";			break;
	case ElementDecl::TypeMixed:	txt += L"MIXED";		break;
	case ElementDecl::TypeElement:	txt += L"ELEMENT";		break;
	default:						txt += L"UNDEFINED";	break;
	}
	if (content.type != ElementContentDecl::TypeUndefined)
	{
		txt += DKString(L"Content:");
		txt += FormatElementContent(content);
	}
	txt += L"\n";
	DKLog(txt);
}

void DKXMLParser::OnStartDocument(void)
{
	DKLog("[DKXMLParser::OnStartDocument]\n");
}

void DKXMLParser::OnEndDocument(void)
{
	DKLog("[DKXMLParser::OnEndDocument]\n");
}

void DKXMLParser::OnStartElement(const Element& element, const DKArray<Namespace>& namespaces, const DKArray<Attribute>& attributes)
{
	DKString txt(L"[DKXMLParser::OnStartElement]");
	txt += DKString(L" Name:") + element.name;
	txt += DKString(L" Prefix:") + element.prefix;
	txt += DKString(L" URI:") + element.URI;
	txt += DKString::Format(" Namespaces: %d", namespaces.Count());
	txt += DKString::Format(" Attributes: %d", attributes.Count());
	txt += L"\n";
	for (int i = 0; i < attributes.Count(); i++)
	{
		txt += attributes.Value(i).name + L"=\"" + attributes.Value(i).value + L"\"\n";
	}
	DKLog(txt);
}

void DKXMLParser::OnEndElement(const Element& element)
{
	DKString txt(L"[DKXMLParser::OnEndElement]");
	txt += DKString(L" Name:") + element.name;
	txt += DKString(L" Prefix:") + element.prefix;
	txt += DKString(L" URI:") + element.URI;
	txt += L"\n";
	DKLog(txt);
}

void DKXMLParser::OnComment(const DKString& comment)
{
	DKLog("[DKXMLParser::OnComment] Comment: %ls\n", (const wchar_t*)comment);
}

void DKXMLParser::OnCharacters(const char* ch, size_t len)
{
	DKLog("[DKXMLParser::OnCharacters] %llu bytes\n", len);
}

void DKXMLParser::OnCharacterDataBlock(const char* ch, size_t len)
{
	DKLog("[DKXMLParser::OnCDataBlock] %llu bytes\n", len);
}

void DKXMLParser::OnWarning(const DKString& mesg)
{
	DKLog("[DKXMLParser::OnWarning] Warning: %ls\n", (const wchar_t*)mesg);
}

void DKXMLParser::OnError(const DKString& mesg)
{
	DKLog("[DKXMLParser::OnError] Error: %ls\n", (const wchar_t*)mesg);
}

void DKXMLParser::OnFatalError(const DKString& mesg)
{
	DKLog("[DKXMLParser::OnFatalError] FatalError: %ls\n", (const wchar_t*)mesg);
}

void DKXMLParser::ClearCustomEntityStorage(void)
{
	if (customEntityStorage.Count() > 0)
	{
		for (int i = 0; i < customEntityStorage.Count(); i++)
			xmlFree((xmlEntityPtr)customEntityStorage.Value(i));
	}
	customEntityStorage.Clear();
}
