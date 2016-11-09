//
//  File: DKXmlDocument.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#include "DKXmlDocument.h"
#include "DKBuffer.h"
#include "DKStack.h"
#include "DKLog.h"

using namespace DKFoundation;

// DocumentBuilder : inherited from DKXmlParser, to building DKXmlDocument object.
class DKXmlDocument::DocumentBuilder : public DKXmlParser
{
public:
	DocumentBuilder(void) : document(NULL)
	{
	}
	~DocumentBuilder(void)
	{
	}

	DKString									warningDesc;
	DKString									errorDesc;
	DKString									fatalErrorDesc;
	DKObject<DKXmlDocument>						document; // building document.

private:
	DKStack<DKObject<DKXmlElement>>				elementsStack;
	DKObject<DKXmlDocTypeDecl>					docTypeDecl; // DTD

	DKXmlDocument::Namespace* FindNamespace(const DKString& prefix, const DKString& URI)
	{
		if (prefix.Length() == 0 && URI.Length() == 0)
			return NULL;
		DKStack<DKObject<DKXmlElement>> clone = elementsStack;
		while (!clone.IsEmpty())
		{
			DKXmlElement* e = clone.Top();
			if (!e->namespaces.IsEmpty())
			{
				for (size_t i = 0; i < e->namespaces.Count(); i++)
				{
					if (e->namespaces[i].prefix == prefix && e->namespaces[i].URI == URI)
						return &(e->namespaces[i]);
				}
			}
			clone.Pop();
		}
		return NULL;
	}
	////////////////////////////////////////////////////////////////////////////////
	// DKXmlParser overrides
	void OnProcessingInstruction(const DKString& target, const DKString& data)
	{
		if (!document)	return;
		DKObject<DKXmlInstruction> ins = DKObject<DKXmlInstruction>::New();
		ins->target = target;
		ins->data = data;

		if (!elementsStack.IsEmpty())
			elementsStack.Top()->nodes.Add(ins.SafeCast<DKXmlNode>());
		else if (docTypeDecl)
			docTypeDecl->nodes.Add(ins.SafeCast<DKXmlNode>());
		else
			document->nodes.Add(ins.SafeCast<DKXmlNode>());
	}
	void OnInternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
	{
		if (!document)	return;

		DKObject<DKXmlDocTypeDecl> d = DKObject<DKXmlDocTypeDecl>::New();
		d->name = name;
		d->externalID = externalID;
		d->systemID = systemID;
		document->nodes.Add(d.SafeCast<DKXmlNode>());
		docTypeDecl = d;
	}
	void OnExternalSubsetDeclaration(const DKString& name, const DKString& externalID, const DKString& systemID)
	{
		// closing subset.
		docTypeDecl = NULL;
	}
	void OnAttributeDeclaration(const AttributeDecl& attr, const DKArray<DKString>& enumeratedValues)
	{
		if (!docTypeDecl)	return;

		DKObject<DKXmlAttributeDecl> a = DKObject<DKXmlAttributeDecl>::New();
		a->decl = attr;
		a->enumeratedValues = enumeratedValues;
		docTypeDecl->nodes.Add(a.SafeCast<DKXmlNode>());
	}
	void OnElementDeclaration(const ElementDecl& elem, const ElementContentDecl& content)
	{
		if (!docTypeDecl)	return;

		DKObject<DKXmlElementDecl> e = DKObject<DKXmlElementDecl>::New();
		e->decl = elem;
		e->contents = content;

		docTypeDecl->nodes.Add(e.SafeCast<DKXmlNode>());
	}
	void OnStartDocument(void)
	{
		elementsStack.Clear();
		document = DKObject<DKXmlDocument>::New();
		docTypeDecl = NULL;
	}
	void OnEndDocument(void)
	{
		elementsStack.Clear();
		docTypeDecl = NULL;
	}
	void OnStartElement(const Element& element, const DKArray<Namespace>& namespaces, const DKArray<Attribute>& attributes)
	{
		docTypeDecl = NULL;
		if (!document)	return;
		DKObject<DKXmlElement> e = DKObject<DKXmlElement>::New();
		e->name = element.name;
		e->namespaces.Reserve(namespaces.Count());
		for (size_t i = 0; i < namespaces.Count(); i++)
		{
			DKXmlNamespace ns = { namespaces[i].prefix, namespaces[i].URI };
			e->namespaces.Add(ns);
		}
		if (elementsStack.IsEmpty())
			document->SetRootElement(e);
		else
			elementsStack.Top()->nodes.Add(e.SafeCast<DKXmlNode>());
		elementsStack.Push(e);

		// add attribute
		e->attributes.Reserve(attributes.Count());
		for (size_t i = 0; i < attributes.Count(); i++)
		{
			DKXmlAttribute att;
			att.name = attributes[i].name;
			att.value = attributes[i].value;
			att.ns = FindNamespace(attributes[i].prefix, attributes[i].URI);
			e->attributes.Add(att);
		}
		// find namespace
		// current element must be pushed into stack, before searching.
		e->ns = FindNamespace(element.prefix, element.URI);
	}
	void OnEndElement(const Element& element)
	{
		if (!document)	return;
		elementsStack.Pop();
	}
	void OnComment(const DKString& comment)
	{
		if (!document)	return;

		DKObject<DKXmlComment>	c = DKObject<DKXmlComment>::New();
		c->value = comment;

		if (!elementsStack.IsEmpty())
			elementsStack.Top()->nodes.Add((DKXmlNode*)c);
		else if (docTypeDecl)
			docTypeDecl->nodes.Add(c.SafeCast<DKXmlNode>());
		else
			document->nodes.Add(c.SafeCast<DKXmlNode>());
	}
	void OnCharacters(const char* ch, size_t len)
	{
		if (!document)	return;
		if (!elementsStack.IsEmpty())
		{
			DKObject<DKXmlPCData> c = DKObject<DKXmlPCData>::New();
			c->value.SetValue((const DKUniChar8*)ch, len);
			elementsStack.Top()->nodes.Add(c.SafeCast<DKXmlNode>());
		}
	}
	void OnCharacterDataBlock(const char* ch, size_t len)
	{
		if (!document)	return;
		if (!elementsStack.IsEmpty())
		{
			DKObject<DKXmlCData> c = DKObject<DKXmlCData>::New();
			c->value.SetValue((const DKUniChar8*)ch, len);
			elementsStack.Top()->nodes.Add(c.SafeCast<DKXmlNode>());
		}
	}
	void OnWarning(const DKString& mesg)
	{
		this->warningDesc = mesg;
		//	DKLog("DKXmlDocumentBuilder:Warning: %ls\n", (const wchar_t*)mesg);
	}
	void OnError(const DKString& mesg)
	{
		this->errorDesc = mesg;
		//	DKLog("DKXmlDocumentBuilder:Error: %ls\n", (const wchar_t*)mesg);
		document = NULL;
	}
	void OnFatalError(const DKString& mesg)
	{
		this->fatalErrorDesc = mesg;
		//	DKLog("DKXmlDocumentBuilder:FatalError: %ls\n", (const wchar_t*)mesg);
		document = NULL;
	}
};

DKXmlDocument::DKXmlDocument(void)
{
}

DKXmlDocument::DKXmlDocument(DocTypeDecl* dtd, Element* root)
{
	SetDTD(dtd);
	SetRootElement(root);
}

DKXmlDocument::DKXmlDocument(Element* root)
{
	SetRootElement(root);
}

DKXmlDocument::~DKXmlDocument(void)
{
}

DKObject<DKXmlDocument> DKXmlDocument::Open(Type t, const DKString& fileOrURL, DKString* desc)
{
	DocumentBuilder doc;
	bool ret = t == TypeXML ? doc.BeginXml(fileOrURL) : doc.BeginHtml(fileOrURL);
	if (ret)
	{
		if (desc)
			desc->SetValue(doc.warningDesc);
		return doc.document;
	}
	else if (desc)
	{
		if (doc.fatalErrorDesc.Length() > 0)
			desc->SetValue(doc.fatalErrorDesc);
		else
			desc->SetValue(doc.errorDesc);
	}
	return NULL;
}

DKObject<DKXmlDocument> DKXmlDocument::Open(Type t, const DKData* buffer, DKString* desc)
{
	DocumentBuilder doc;
	bool ret = t == TypeXML ? doc.BeginXml(buffer) : doc.BeginHtml(buffer);
	if (ret)
	{
		if (desc)
			desc->SetValue(doc.warningDesc);
		return doc.document;
	}
	else if (desc)
	{
		if (doc.fatalErrorDesc.Length() > 0)
			desc->SetValue(doc.fatalErrorDesc);
		else
			desc->SetValue(doc.errorDesc);
	}
	return NULL;
}

DKObject<DKXmlDocument> DKXmlDocument::Open(Type t, DKStream* stream, DKString* desc)
{
	DocumentBuilder doc;
	bool ret = t == TypeXML ? doc.BeginXml(stream) : doc.BeginHtml(stream);
	if (ret)
	{
		if (desc)
			desc->SetValue(doc.warningDesc);
		return doc.document;
	}
	else if (desc)
	{
		if (doc.fatalErrorDesc.Length() > 0)
			desc->SetValue(doc.fatalErrorDesc);
		else
			desc->SetValue(doc.errorDesc);
	}
	return NULL;
}

DKXmlElement* DKXmlDocument::RootElement(void)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeElement)
			return nodes.Value(i).SafeCast<Element>();
	}
	return NULL;
}

const DKXmlElement* DKXmlDocument::RootElement(void) const
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeElement)
			return nodes.Value(i).SafeCast<Element>();
	}
	return NULL;
}

void DKXmlDocument::SetRootElement(Element* e)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeElement)
		{
			if (e)
				nodes.Value(i) = e;
			else
				nodes.Remove(i);
			return;
		}
	}
	if (e)
		nodes.Add(e);
}

DKXmlDocTypeDecl* DKXmlDocument::DTD(void)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeDocTypeDecl)
			return nodes.Value(i).SafeCast<DocTypeDecl>();
	}
	return NULL;
}

const DKXmlDocTypeDecl* DKXmlDocument::DTD(void) const
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeDocTypeDecl)
			return nodes.Value(i).SafeCast<DocTypeDecl>();
	}
	return NULL;
}

void DKXmlDocument::SetDTD(DocTypeDecl* d)
{
	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i)->Type() == Node::NodeTypeDocTypeDecl)
		{
			if (d)
				nodes.Value(i) = d;
			else
				nodes.Remove(i);
			return;
		}
	}
	if (d)
		nodes.Add(d);
}

DKObject<DKData> DKXmlDocument::Export(DKStringEncoding e) const
{
	if (RootElement() == NULL)
		return NULL;

	if (nodes.IsEmpty())
		return NULL;

	DKString doc = DKString(L"<?xml version=\"1.0\" encoding=\"") + DKString(DKStringEncodingCanonicalName(e)) + L"\" ?>";

	for (int i = 0; i < nodes.Count(); i++)
	{
		if (nodes.Value(i))
			doc += nodes.Value(i)->Export();
	}

	return doc.Encode(e);
}

size_t DKXmlDocument::Export(DKStringEncoding e, DKStream* output) const
{
	size_t s = 0;
	if (output && output->IsWritable())
	{
		DKObject<DKData> d = Export(e);
		if (d)
		{
			const void* p = d->LockShared();
			if (p)
			{
				s = output->Write(p, d->Length());
			}
			d->UnlockShared();
		}
	}
	return s;
}

DKXmlDocument::Node::NodeType DKXmlDocument::Node::Type(void) const
{
	if (this)
		return this->type;
	return NodeTypeUnknown;
}

DKString DKXmlDocument::Instruction::Export(void) const
{
	return DKString(L"<?") + target + L" " + data + L"?>";
}

DKString DKXmlDocument::DocTypeDecl::Export(void) const
{
	DKString ret = DKString(L"<!DOCTYPE ") + name;
	if (externalID.Length() > 0)
		ret += DKString(L" PUBLIC \"") + externalID + L"\" \"" + systemID + L"\"";
	else if (systemID.Length() > 0)
		ret += DKString(L" \"") + systemID + L"\"";

	if (!nodes.IsEmpty())
	{
		ret += L" [";

		////////////////////////////////////////////////////////////////////////////////
		// loop 
		for (int i = 0; i < nodes.Count(); i++)
			ret += nodes.Value(i)->Export();

		ret += L"]";
	}
	ret += L">";
	return ret;
}

DKString DKXmlDocument::ElementDecl::Export(void) const
{
	DKString ret = DKString(L"<!ELEMENT ") + decl.name + L" ";
	switch (decl.type)
	{
	case DKXmlParser::ElementDecl::TypeEmpty:	ret += L"EMPTY";		break;
	case DKXmlParser::ElementDecl::TypeAny:		ret += L"ANY";			break;
	}
	if (contents.type != DKXmlParser::ElementContentDecl::TypeUndefined)
		ret += DKString(L" (") + DKXmlParser::FormatElementContent(contents) + L") >";
	else
		ret += L">";
	return ret;
}

DKString DKXmlDocument::AttributeDecl::Export(void) const
{
	DKString ret = DKString(L"<!ATTLIST ") + decl.element + L" " + decl.name;
	switch (decl.type)
	{
	case DKXmlParser::AttributeDecl::TypeCData:				ret += L" CDATA";		break;
	case DKXmlParser::AttributeDecl::TypeID:				ret += L" ID";			break;
	case DKXmlParser::AttributeDecl::TypeIDRef:				ret += L" IDREF";		break;
	case DKXmlParser::AttributeDecl::TypeIDRefs:			ret += L" IDREFS";		break;
	case DKXmlParser::AttributeDecl::TypeEntity:			ret += L" ENTITY";		break;
	case DKXmlParser::AttributeDecl::TypeEntities:			ret += L" ENTITIES";	break;
	case DKXmlParser::AttributeDecl::TypeNMToken:			ret += L" NMTOKEN";		break;
	case DKXmlParser::AttributeDecl::TypeNMTokens:			ret += L" NMTOKENS";	break;
	case DKXmlParser::AttributeDecl::TypeEnumeration:		ret += L" ENUMERATION";	break;
	case DKXmlParser::AttributeDecl::TypeNotation:			ret += L" NOTATION";	break;
	}
	switch (decl.defaultValueType)
	{
	case DKXmlParser::AttributeDecl::ValueTypeRequired:		ret += L" #REQUIRED";	break;
	case DKXmlParser::AttributeDecl::ValueTypeImplied:		ret += L" #IMPLIED";	break;
	case DKXmlParser::AttributeDecl::ValueTypeFixed:		ret += L" #FIXED";		break;
	}
	if (decl.defaultValue.Length() > 0)
		ret += DKString(L" ") + decl.defaultValue;

	ret += L">";
	return ret;
}

DKString DKXmlDocument::Comment::Export(void) const
{
	return DKString(L"<!--") + value + L"-->";
}

DKString DKXmlDocument::Element::Export(void) const
{
	DKArray<Namespace> dummy;
	return Export(dummy);
}

DKString DKXmlDocument::Element::Export(DKArray<Namespace>& writtenNS) const
{
	if (name.Length() == 0)
		return L"";

	struct
	{
		bool operator () (const Namespace& target) const
		{
			for (size_t i = 0; i < nsArray.Count(); ++i)
			{
				const Namespace& n = nsArray.Value(i);
				if (n.prefix.Compare(target.prefix) == 0 && n.URI.Compare(target.URI) == 0)
					return true;
			}
			return false;
		}
		const DKArray<Namespace>& nsArray;
	} isNamespaceExist = { writtenNS };
	struct
	{
		DKString operator () (const Namespace& ns) const
		{
			if (ns.URI.Length() == 0)
				return L"";

			if (ns.prefix.Length() > 0)
				return DKString::Format(" xmlns:%ls=\"%ls\"", (const wchar_t*)ns.prefix, (const wchar_t*)ns.URI);
			return DKString::Format(" xmlns=\"%ls\"", (const wchar_t*)ns.URI);
		}
	} formatNamespace;

	const DKXmlNamespace* nodeNS = NULL;
	if (ns)
	{
		if (ns->prefix.Length() > 0 && ns->URI.Length() > 0)
			nodeNS = ns;
		else
			DKLog("Warning: Invalid XML namespace for element:%ls.\n", (const wchar_t*)name);
	}

	DKString nodeTag = L"";
	if (nodeNS)
		nodeTag = nodeNS->prefix + L":" + name;
	else
		nodeTag = name;

	DKString ret = DKString::Format("<%ls", (const wchar_t*)nodeTag);

	for (int i = 0; i < namespaces.Count(); i++)
	{
		// same prefix could be exists, register namespace altogether.
		if (namespaces.Value(i).prefix.Length() > 0 && namespaces.Value(i).URI.Length() > 0)
		{
			ret += formatNamespace(namespaces.Value(i));
			writtenNS.Add(namespaces.Value(i));
		}
	}
	// check namespace exists.
	if (nodeNS && isNamespaceExist(*nodeNS) == false)
	{
		ret += formatNamespace(*nodeNS);
		writtenNS.Add(*nodeNS);
	}
	// finding attribute's namespace from stack.
	for (int i = 0; i < attributes.Count(); i++)
	{
		const Attribute& attr = attributes.Value(i);
		if (attr.ns)
		{
			if (attr.ns->prefix.Length() > 0 && attr.ns->URI.Length() > 0)
			{
				if (isNamespaceExist(*attr.ns) == false)
				{
					ret += formatNamespace(*attr.ns);
					writtenNS.Add(*attr.ns);
				}
			}
			else
			{
				DKLog("Warning: Invalid XML namespace for attribute:%ls.\n", (const wchar_t*)attr.name);
			}
		}
	}

	for (int i = 0; i < attributes.Count(); i++)
	{
		const Attribute& attr = attributes.Value(i);
		if (attr.name.Length() == 0)
			continue;

		DKString value = attr.value;
		value.Replace(L"&", L"&amp;");
		value.Replace(L"\"", L"&quot;");
		value.Replace(L"'", L"&apos;");
		value.Replace(L"<", L"&lt;");
		value.Replace(L">", L"&gt;");

		if (attr.ns && attr.ns->prefix.Length() > 0 && attr.ns->URI.Length() > 0)
			ret += DKString(L" ") + attr.ns->prefix + L":" + attr.name + L"=\"" + value + L"\"";
		else
			ret += DKString(L" ") + attr.name + L"=\"" + value + L"\"";
	}
	if (nodes.IsEmpty())
	{
		ret += L" />";
	}
	else
	{
		ret += L">";
		for (size_t i = 0; i < nodes.Count(); i++)
		{
			const Node* node = nodes.Value(i);
			if (node == NULL)
				continue;

			if (node->Type() == Node::NodeTypeElement)
				ret += ((Element*)node)->Export(writtenNS);
			else
				ret += node->Export();
		}
		ret += DKString(L"</") + nodeTag + L">";
	}
	return ret;
}

DKString DKXmlDocument::CData::Export(void) const
{
	DKString cdata(value);
	cdata.Replace(L"]]>", L"]]]]><![CDATA[>");
	return DKString(L"<![CDATA[") + cdata + L"]]>";
}

DKString DKXmlDocument::PCData::Export(void) const
{
	DKString ret = value;
	ret.Replace(L"&", L"&amp;");
	ret.Replace(L"\"", L"&quot;");
	ret.Replace(L"'", L"&apos;");
	ret.Replace(L"<", L"&lt;");
	ret.Replace(L">", L"&gt;");
	return ret;
}
