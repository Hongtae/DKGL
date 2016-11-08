//
//  File: DKXmlParser.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include <string.h>
#include <memory.h>
#include "../Libs/Inc_libxml2.h"
#include "DKXmlParser.h"
#include "DKLog.h"
#include "DKString.h"
#include "DKDataStream.h"

namespace DKGL
{
	namespace Private
	{
		namespace
		{
			inline DKString xmlCharToDKString(const xmlChar* c)
			{
				return DKString((const DKUniChar8*)c);
			}
			inline DKString xmlCharToDKString(const xmlChar* c, size_t len)
			{
				return DKString((const DKUniChar8*)c, len);
			}
			void internalSubset(void *ctx, const xmlChar *name, const xmlChar *ExternalID, const xmlChar *SystemID)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnInternalSubsetDeclaration(xmlCharToDKString(name), xmlCharToDKString(ExternalID), xmlCharToDKString(SystemID));
			}
			int isStandalone(void *ctx)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.isStandalone()\n");
				return 0;
			}
			int hasInternalSubset(void *ctx)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.hasInternalSubset()\n");
				return 0;
			}
			int hasExternalSubset(void *ctx)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.hasExternalSubset()\n");
				return 0;
			}
			xmlParserInputPtr resolveEntity(void *ctx, const xmlChar *publicId, const xmlChar *systemId)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				/* xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx; */
				DKString text = DKString(L"SAX.resolveEntity(");
				if (publicId != NULL)
					text += DKString::Format("%ls", (const wchar_t*)xmlCharToDKString(publicId));
				else
					text += DKString(L" ");
				if (systemId != NULL)
					text += DKString::Format(", %ls)\n", (const wchar_t*)xmlCharToDKString(systemId));
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
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKLog("SAX.getEntity(%ls)\n", (const wchar_t*)xmlCharToDKString(name));
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
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				const xmlChar *nullstr = BAD_CAST "(null)";
				/* not all libraries handle printing null pointers nicely */
				if (publicId == NULL)
					publicId = nullstr;
				if (systemId == NULL)
					systemId = nullstr;
				if (content == NULL)
					content = (xmlChar *)nullstr;

				DKLog("SAX.entityDecl(%ls, %d, %ls, %ls, %ls)\n",
					(const wchar_t*)xmlCharToDKString(name),
					type, 
					(const wchar_t*)xmlCharToDKString(publicId),
					(const wchar_t*)xmlCharToDKString(systemId),
					(const wchar_t*)xmlCharToDKString(content));

				xmlEntityPtr e = xmlNewEntity(NULL, name, type, publicId, systemId, content);
				if (e)
				{
					parser->customEntityStorage.Add(e);
				}
			}
			void notationDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKLog("SAX.notationDecl(%ls, %ls, %ls)\n",
					(const wchar_t*)xmlCharToDKString(name),
					(const wchar_t*)xmlCharToDKString(publicId),
					(const wchar_t*)xmlCharToDKString(systemId));
			}
			void attributeDecl(void *ctx, const xmlChar * elem, const xmlChar * name, int type, int def, const xmlChar * defaultValue, xmlEnumerationPtr tree)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKXmlParser::AttributeDecl::Type			attributeType;		// attribute type
				DKXmlParser::AttributeDecl::ValueType		defaultValueType;	// default value type
				switch (type)
				{
				case XML_ATTRIBUTE_CDATA:			attributeType = DKXmlParser::AttributeDecl::TypeCData;			break;
				case XML_ATTRIBUTE_ID:				attributeType = DKXmlParser::AttributeDecl::TypeID;				break;
				case XML_ATTRIBUTE_IDREF:			attributeType = DKXmlParser::AttributeDecl::TypeIDRef;			break;
				case XML_ATTRIBUTE_IDREFS:			attributeType = DKXmlParser::AttributeDecl::TypeIDRefs;			break;
				case XML_ATTRIBUTE_ENTITY:			attributeType = DKXmlParser::AttributeDecl::TypeEntity;			break;
				case XML_ATTRIBUTE_ENTITIES:		attributeType = DKXmlParser::AttributeDecl::TypeEntities;		break;
				case XML_ATTRIBUTE_NMTOKEN:			attributeType = DKXmlParser::AttributeDecl::TypeNMToken;		break;
				case XML_ATTRIBUTE_NMTOKENS:		attributeType = DKXmlParser::AttributeDecl::TypeNMTokens;		break;
				case XML_ATTRIBUTE_ENUMERATION:		attributeType = DKXmlParser::AttributeDecl::TypeEnumeration;	break;
				case XML_ATTRIBUTE_NOTATION:		attributeType = DKXmlParser::AttributeDecl::TypeNotation;		break;
				}

				DKString defString; // type of default value.
				switch (def)
				{
				case XML_ATTRIBUTE_NONE:			defaultValueType = DKXmlParser::AttributeDecl::ValueTypeNone;		break;
				case XML_ATTRIBUTE_REQUIRED:		defaultValueType = DKXmlParser::AttributeDecl::ValueTypeRequired;	break;
				case XML_ATTRIBUTE_IMPLIED:			defaultValueType = DKXmlParser::AttributeDecl::ValueTypeImplied;	break;
				case XML_ATTRIBUTE_FIXED:			defaultValueType = DKXmlParser::AttributeDecl::ValueTypeFixed;		break;
				}

				DKArray<DKString>	enumList;
				for (xmlEnumerationPtr p = tree; p != NULL; p = p->next)
					enumList.Add(DKString((const DKUniChar8*)p->name));

				DKXmlParser::AttributeDecl	attr;
				attr.name = xmlCharToDKString(name);
				attr.element = xmlCharToDKString(elem);
				attr.type = attributeType;
				attr.defaultValueType = defaultValueType;
				attr.defaultValue = xmlCharToDKString(defaultValue);

				parser->OnAttributeDeclaration(attr, enumList);

				xmlFreeEnumeration(tree);
			}
			void ConvertElementDeclContent(xmlElementContentPtr from, DKXmlParser::ElementContentDecl& to)
			{
				if (from == NULL)
				{
					to.type = DKXmlParser::ElementContentDecl::TypeUndefined;
					return;
				}

				to.name = xmlCharToDKString(from->name);
				to.prefix = xmlCharToDKString(from->prefix);

				switch (from->type)
				{
				case XML_ELEMENT_CONTENT_PCDATA:	to.type = DKXmlParser::ElementContentDecl::TypePCData;			break;
				case XML_ELEMENT_CONTENT_ELEMENT:	to.type = DKXmlParser::ElementContentDecl::TypeElement;			break;
				case XML_ELEMENT_CONTENT_SEQ:		to.type = DKXmlParser::ElementContentDecl::TypeSequence;		break;
				case XML_ELEMENT_CONTENT_OR:		to.type = DKXmlParser::ElementContentDecl::TypeAlternative;		break;
				default:
					to.type = DKXmlParser::ElementContentDecl::TypeUndefined;
					return;
				}
				switch (from->ocur)
				{
				case XML_ELEMENT_CONTENT_ONCE:		to.occurrence = DKXmlParser::ElementContentDecl::OccurrenceOnce;		break;
				case XML_ELEMENT_CONTENT_OPT:		to.occurrence = DKXmlParser::ElementContentDecl::OccurrenceOnceOrNone;	break;
				case XML_ELEMENT_CONTENT_MULT:		to.occurrence = DKXmlParser::ElementContentDecl::OccurrenceNoneOrMore;	break;
				case XML_ELEMENT_CONTENT_PLUS:		to.occurrence = DKXmlParser::ElementContentDecl::OccurrenceOnceOrMore;	break;
				}

				if (from->c1)
				{
					DKXmlParser::ElementContentDecl decl;
					ConvertElementDeclContent(from->c1, decl);
					if (decl.type != DKXmlParser::ElementContentDecl::TypeUndefined)
					{
						if (decl.type == to.type && decl.occurrence == DKXmlParser::ElementContentDecl::OccurrenceOnce)
							to.contents.Add(decl.contents);
						else
							to.contents.Add(decl);
					}
				}
				if (from->c2)
				{
					DKXmlParser::ElementContentDecl decl;
					ConvertElementDeclContent(from->c2, decl);
					if (decl.type != DKXmlParser::ElementContentDecl::TypeUndefined)
					{
						if (decl.type == to.type && decl.occurrence == DKXmlParser::ElementContentDecl::OccurrenceOnce)
							to.contents.Add(decl.contents);
						else
							to.contents.Add(decl);
					}
				}
			}
			void elementDecl(void *ctx, const xmlChar *name, int type, xmlElementContentPtr content)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				// 'content' contains the value, parsed by parser as tree-structured.
				// the value includes conditions between element and element.
				//
				// if element is <!ELEMENT ele (a | b | c)>,
				// structured value becomes to 'A or (B or C)'
				// For given element is '(a | b | c | d)',
				// structured value become to 'A or (B or (C or D))'

				DKXmlParser::ElementDecl		elem;
				DKXmlParser::ElementContentDecl	ec;

				elem.name = DKString((const DKUniChar8*)name);
				switch (type)
				{
				case XML_ELEMENT_TYPE_EMPTY:		elem.type = DKXmlParser::ElementDecl::TypeEmpty;		break;
				case XML_ELEMENT_TYPE_ANY:			elem.type = DKXmlParser::ElementDecl::TypeAny;			break;
				case XML_ELEMENT_TYPE_MIXED:		elem.type = DKXmlParser::ElementDecl::TypeMixed;		break;
				case XML_ELEMENT_TYPE_ELEMENT:		elem.type = DKXmlParser::ElementDecl::TypeElement;		break;
				default:							elem.type = DKXmlParser::ElementDecl::TypeUndefined;	break;
				}
				ConvertElementDeclContent(content, ec);
				parser->OnElementDeclaration(elem, ec);
			}
			void unparsedEntityDecl(void *ctx, const xmlChar *name, const xmlChar *publicId, const xmlChar *systemId, const xmlChar *notationName)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				const xmlChar *nullstr = BAD_CAST "(null)";

				if (publicId == NULL)
					publicId = nullstr;
				if (systemId == NULL)
					systemId = nullstr;
				if (notationName == NULL)
					notationName = nullstr;

				DKLog("SAX.unparsedEntityDecl(%ls, %ls, %ls, %ls)\n", 
					(const wchar_t*)xmlCharToDKString(name),
					(const wchar_t*)xmlCharToDKString(publicId),
					(const wchar_t*)xmlCharToDKString(systemId),
					(const wchar_t*)xmlCharToDKString(notationName));
			}
			void setDocumentLocator(void *ctx, xmlSAXLocatorPtr loc)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				//DKLog("SAX.setDocumentLocator()\n");
			}
			void startDocument(void *ctx)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnStartDocument();
			}
			void endDocument(void *ctx)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnEndDocument();
			}
			void startElement(void *ctx, const xmlChar *name, const xmlChar **atts)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKXmlParser::Element element;
				element.name = DKString((const DKUniChar8*)name);

				DKArray<DKXmlParser::Attribute>	attr;
				if (atts != NULL)
				{
					for (int i = 0; atts[i] != NULL; i++)
					{
						DKXmlParser::Attribute at;
						at.name = xmlCharToDKString(atts[i]);
						i++;
						at.value = xmlCharToDKString(atts[i]);

						attr.Add(at);
					}					 
				}

				parser->OnStartElement(element, DKArray<DKXmlParser::Namespace>(), attr);
			}
			void endElement(void *ctx, const xmlChar *name)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKXmlParser::Element element;
				element.name = xmlCharToDKString(name);
				parser->OnEndElement(element);
			}
			void reference(void *ctx, const xmlChar *name)
			{
				// called after entity had printed as characters.
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.reference(%ls)\n", (const wchar_t*)xmlCharToDKString(name));
			}
			void characters(void *ctx, const xmlChar *ch, int len)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnCharacters(reinterpret_cast<const char*>(ch), len);
			}
			void ignorableWhitespace(void *ctx, const xmlChar *ch, int len)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.ignorableWhitespace: %d bytes\n", len);
			}
			void processingInstruction(void *ctx, const xmlChar *target, const xmlChar *data)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnProcessingInstruction(xmlCharToDKString(target), xmlCharToDKString(data));
			}
			void comment(void *ctx, const xmlChar *value)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnComment(xmlCharToDKString(value));
			}
			void XMLCDECL warning(void *ctx, const char *msg, ...)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnWarning(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			void XMLCDECL error(void *ctx, const char *msg, ...)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnError(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			void XMLCDECL fatalError(void *ctx, const char *msg, ...)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				va_list ap;
				va_start(ap, msg);
				parser->OnFatalError(DKString::FormatV(msg, ap));
				va_end(ap);
			}
			xmlEntityPtr getParameterEntity(void *ctx, const xmlChar *name)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				DKLog("SAX.getParameterEntity(%ls)\n", (const wchar_t*)xmlCharToDKString(name));
				return NULL;
			}
			void cdataBlock(void *ctx, const xmlChar *value, int len)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnCharacterDataBlock(reinterpret_cast<const char*>(value), len);
			}
			void externalSubset(void *ctx, const xmlChar *name, const xmlChar *ExternalID, const xmlChar *SystemID)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);
				parser->OnExternalSubsetDeclaration(xmlCharToDKString(name), xmlCharToDKString(ExternalID), xmlCharToDKString(SystemID));
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
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKXmlParser::Element	element;
				element.name = xmlCharToDKString(localname);
				element.prefix = xmlCharToDKString(prefix);
				element.URI = xmlCharToDKString(URI);

				DKArray<DKXmlParser::Namespace>	ns;
				if (nb_namespaces)
				{
					ns.Resize(nb_namespaces);
					for (int i = 0; i < nb_namespaces; i++)
					{
						ns.Value(i).prefix = xmlCharToDKString(namespaces[i*2]);
						ns.Value(i).URI = xmlCharToDKString(namespaces[i*2+1]);
					}
				}
				DKArray<DKXmlParser::Attribute> attrs;
				if (attributes)
				{
					attrs.Resize(nb_attributes);
					for (int i = 0; i < nb_attributes; i++)
					{
						attrs.Value(i).name = xmlCharToDKString(attributes[i*5]);
						attrs.Value(i).prefix = xmlCharToDKString(attributes[i*5+1]);
						attrs.Value(i).URI = xmlCharToDKString(attributes[i*5+2]);
						attrs.Value(i).value = xmlCharToDKString(attributes[i*5+3], (size_t)((const char*)attributes[i*5+4] - (const char*)attributes[i*5+3]));
					}
				}
				parser->OnStartElement(element, ns, attrs);
			}
			void endElementNs(void *ctx, const xmlChar *localname, const xmlChar *prefix, const xmlChar *URI)
			{
				DKXmlParser* parser = reinterpret_cast<DKXmlParser*>(ctx);

				DKXmlParser::Element	element;
				element.name = xmlCharToDKString(localname);
				element.prefix = xmlCharToDKString(prefix);
				element.URI = xmlCharToDKString(URI);

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

using namespace DKGL;

DKXmlParser::DKXmlParser(void)
{

}

DKXmlParser::~DKXmlParser(void)
{
	ClearCustomEntityStorage();
}

bool DKXmlParser::BeginHtml(const DKString& url)
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

bool DKXmlParser::BeginHtml(const DKData* data)
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
				char* text = (char*)DKMemoryDefaultAllocator::Alloc(bufferSize + 4);
				memset(text, 0, bufferSize+4);
				memcpy(text, buffer, bufferSize);

				htmlDocPtr doc = htmlSAXParseDoc((xmlChar*)text, 0, &Private::defaultSAX2Handler, this);

				DKMemoryDefaultAllocator::Free(text);

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

bool DKXmlParser::BeginHtml(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
		if (ds)
			return BeginHtml(ds->DataSource());
		return BeginHtml(DKBuffer::Create(stream));
	}
	return false;
}

bool DKXmlParser::BeginXml(const DKString& url)
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

bool DKXmlParser::BeginXml(const DKData* data)
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

bool DKXmlParser::BeginXml(DKStream* stream)
{
	if (stream && stream->IsReadable())
	{
		DKObject<DKDataStream> ds = DKObject<DKStream>(stream).SafeCast<DKDataStream>();
		if (ds)
			return BeginXml(ds->DataSource());
		return BeginXml(DKBuffer::Create(stream));
	}
	return false;
}

void DKXmlParser::OnProcessingInstruction(const DKString& target, const DKString& data)
{
	DKString txt(L"[DKXmlParser::OnProcessingInstruction]");
	txt += DKString(L" Target:") + target;
	txt += DKString(L" Data:") + data;
	txt += L"\n";
	DKLog(txt);
}

void DKXmlParser::OnInternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
{
	DKString txt(L"[DKXmlParser::OnInternalSubset]");
	txt += DKString(L" Name:") + name;
	txt += DKString(L" ExternalID:") + externalID;
	txt += DKString(L" SystemID:") + systemID;
	txt += L"\n";
	DKLog(txt);
}

void DKXmlParser::OnExternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
{
	DKString txt(L"[DKXmlParser::OnExternalSubset]");
	txt += DKString(L" Name:") + name;
	txt += DKString(L" ExternalID:") + externalID;
	txt += DKString(L" SystemID:") + systemID;
	txt += L"\n";
	DKLog(txt);
}
void DKXmlParser::OnEntityDeclaration(const DKString& name, int type, const DKString& publicID, const DKString& systemID, const DKString& content)
{

}

void DKXmlParser::OnAttributeDeclaration(const AttributeDecl& attr, const DKArray<DKString>& enumeratedValues)
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

	DKString txt(L"[DKXmlParser::OnAttributeDeclaration]");
	txt += DKString(L" name:") + attr.name;
	txt += DKString(L" element:") + attr.element;
	txt += DKString(L" type:") + attType;
	txt += DKString(L" default:") + attr.defaultValue + L"(" + defType + L")";
	txt += DKString::Format(" enumValueCount: %d", enumeratedValues.Count());
	txt += L"\n";
	DKLog(txt);
}

DKString DKXmlParser::FormatElementContent(const DKXmlParser::ElementContentDecl& content)
{
	if (content.type == DKXmlParser::ElementContentDecl::TypeUndefined)
		return L"";

	DKString ret = L"";

	if (content.prefix.Length() > 0)
		ret += content.prefix + L":";

	switch (content.type)
	{
	case DKXmlParser::ElementContentDecl::TypePCData:			ret += L"#PCDATA";		break;
	case DKXmlParser::ElementContentDecl::TypeElement:			ret += content.name;	break;
	case DKXmlParser::ElementContentDecl::TypeSequence:			// (A,B,..)
	case DKXmlParser::ElementContentDecl::TypeAlternative:		// (A|B|..)
		break;
	}

	if (content.type == DKXmlParser::ElementContentDecl::TypeSequence ||
		content.type == DKXmlParser::ElementContentDecl::TypeAlternative)
	{
		ret += L"(";
		for (int i = 0; i < content.contents.Count(); i++)
		{
			if ( i > 0)
			{
				if (content.type == DKXmlParser::ElementContentDecl::TypeSequence)
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
	case DKXmlParser::ElementContentDecl::OccurrenceNoneOrMore:		ret += L"*";	break;		// *
	case DKXmlParser::ElementContentDecl::OccurrenceOnceOrMore:		ret += L"+";	break;		// +
	case DKXmlParser::ElementContentDecl::OccurrenceOnceOrNone:		ret += L"?";	break;		// ?
	case DKXmlParser::ElementContentDecl::OccurrenceOnce:
		break;
	}
	return ret;
}

void DKXmlParser::OnElementDeclaration(const ElementDecl& elem, const ElementContentDecl& content)
{
	DKString txt(L"[DKXmlParser::OnElementDeclaration]");
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

void DKXmlParser::OnStartDocument(void)
{
	DKLog("[DKXmlParser::OnStartDocument]\n");
}

void DKXmlParser::OnEndDocument(void)
{
	DKLog("[DKXmlParser::OnEndDocument]\n");
}

void DKXmlParser::OnStartElement(const Element& element, const DKArray<Namespace>& namespaces, const DKArray<Attribute>& attributes)
{
	DKString txt(L"[DKXmlParser::OnStartElement]");
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

void DKXmlParser::OnEndElement(const Element& element)
{
	DKString txt(L"[DKXmlParser::OnEndElement]");
	txt += DKString(L" Name:") + element.name;
	txt += DKString(L" Prefix:") + element.prefix;
	txt += DKString(L" URI:") + element.URI;
	txt += L"\n";
	DKLog(txt);
}

void DKXmlParser::OnComment(const DKString& comment)
{
	DKLog("[DKXmlParser::OnComment] Comment: %ls\n", (const wchar_t*)comment);
}

void DKXmlParser::OnCharacters(const char* ch, size_t len)
{
	DKLog("[DKXmlParser::OnCharacters] %llu bytes\n", len);
}

void DKXmlParser::OnCharacterDataBlock(const char* ch, size_t len)
{
	DKLog("[DKXmlParser::OnCDataBlock] %llu bytes\n", len);
}

void DKXmlParser::OnWarning(const DKString& mesg)
{
	DKLog("[DKXmlParser::OnWarning] Warning: %ls\n", (const wchar_t*)mesg);
}

void DKXmlParser::OnError(const DKString& mesg)
{
	DKLog("[DKXmlParser::OnError] Error: %ls\n", (const wchar_t*)mesg);
}

void DKXmlParser::OnFatalError(const DKString& mesg)
{
	DKLog("[DKXmlParser::OnFatalError] FatalError: %ls\n", (const wchar_t*)mesg);
}

void DKXmlParser::ClearCustomEntityStorage(void)
{
	if (customEntityStorage.Count() > 0)
	{
		for (int i = 0; i < customEntityStorage.Count(); i++)
			xmlFree((xmlEntityPtr)customEntityStorage.Value(i));
	}
	customEntityStorage.Clear();
}
