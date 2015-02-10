//
//  File: DKSerializer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "../DKFoundation.h"
#include "DKVariant.h"


////////////////////////////////////////////////////////////////////////////////
// DKSerializer
// Object serializer, deserializer class.
// Using XML or binary formatted data, XML formatted data can be transferred on
// text-based protocols.
// Using DKVariant for internal components, You can also have external
// resources. Each external resource can have separate serializer object.
// You can embed any external resources in a single serializer object.
// External resources could be single or array, map with String key.
// You can also embed any serializer objects in a single serializer object.
//
// Once a class object generates serializer object, then object can be
// serializable, deserializable by single serializer object.
// On generating serializer object, you need to define your components which
// sould be embedded. You can define your components getter, setter with
// DKFunction.
//
// For external resources, You can specify how resource can be refered.
// see ExternalResource enums on class declarations below.
//
// You may need DKResourceLoader object for deserialize object. because of
// referred external resources can be loaded separately by DKResourceLoader.
//
// An external resource should have a name as file-name to be loaded separately.
// Unnamed resource should be embedded and not valid as references which loads
// separately while deserialize in progress.
//
// On any errors has occurred, serializer calls fault-handler, if not NULL.
// You may need to provide fault-handler to control error by conditions.
// If you don't provde fault-handler, deserialization process succeeded only if
// no error has occurred. (missing files, file has wrong types will stop
// deserialize process.)
//
// You can also privdes 'checker' operation, which will be called on
// deserialization process. If operation provided and called, return true for
// valid state, return false to invalid state and stop deserialize.
//
// Behaviors on serialize:
//    if getter is NULL, entry ignored and will not be serialized.
//    if faultHandler is NULL, error on external resource failed.
// Behaviors on deserialize:
//    if setter is NULL, entry ignored and will not be restored.
//    if setter is not NULL and faultHandler is NULL, regarded as deserialized
//    without any error.
//    if faultHandler is not NULL, will be called when data is missing.
//    if checker is NULL, will not validate entry.
//    if checker is not NULL, will be called and returns true for validate,
//    or generates an error if checker returns false. (error can be recovered
//    by faultHandler, if available.)
//
//
// Note:
//    You can use this class for not only DKResource object but any types of
//    object also. But external resources should be DKResource.
//    (You need wrapper class for non DKResource object)
//
////////////////////////////////////////////////////////////////////////////////


namespace DKFramework
{
	class DKResource;
	class DKResourceLoader;
	class DKSerializer
	{
	public:
		// SerializeForm
		// - SerializeFormXML:
		//     xml format plane text. (can be large)
		// - SerializeFormBinXML:
		//     xml format text with compressed contents.
		//     useful for text based transfer protocols.
		// - SerializeFormBinary:
		//     uncompressed binary format. (faster loading)
		// - SerializeFormCompressedBinary:
		//     compressed binary format. (smallest, faster than xml)
		enum SerializeForm : int
		{
			SerializeFormXML				= '_XML',
			SerializeFormBinXML				= 'bXML',
			SerializeFormBinary				= '_BIN',
			SerializeFormCompressedBinary	= 'cBIN',
		};
		// serialize/deserialize callback state
		enum State
		{
			StateSerializeBegin = 0,  // On serialization begins
			StateSerializeSucceed,    // On serialization succeeded
			StateSerializeFailed,     // On serialization failed
			StateDeserializeBegin,    // On deserialization begins
			StateDeserializeSucceed,  // On deserialization succeeded
			StateDeserializeFailed,   // On deserialization failed
		};
		// defining referencing external resource
		enum ExternalResource
		{
			ExternalResourceInclude = 0,         // included in current serializer.
			ExternalResourceReferenceIfPossible, // reference as external if possible.
			ExternalResourceForceReference,      // force reference (never include)
		};

		// using DKVariant as components container.
		typedef DKVariant ValueType;
		// DKResource type can be reference to external resource.
		// external resources not included in this serializer.
		// external resources will be loaded separately with this serailizer.
		typedef DKFoundation::DKObject<DKResource> ExternalType;
		typedef DKFoundation::DKArray<ExternalType> ExternalArrayType;
		typedef DKFoundation::DKMap<DKFoundation::DKString, ExternalType> ExternalMapType;

		// component value getter, setter, checker
		// when data needs to be validated, calling checker to validate if checker is not NULL.
		typedef DKFoundation::DKFunctionSignature<void (ValueType&)> ValueGetter;
		typedef DKFoundation::DKFunctionSignature<void (ValueType&)> ValueSetter;
		typedef DKFoundation::DKFunctionSignature<bool (const ValueType&)> ValueChecker;

		// external resource getter, setter, checker
		typedef DKFoundation::DKFunctionSignature<void (ExternalType&)> ExternalGetter;
		typedef DKFoundation::DKFunctionSignature<void (ExternalType&)> ExternalSetter;
		typedef DKFoundation::DKFunctionSignature<bool (const ExternalType&)> ExternalChecker;

		// external resource array
		typedef DKFoundation::DKFunctionSignature<void (ExternalArrayType&)> ExternalArrayGetter;
		typedef DKFoundation::DKFunctionSignature<void (ExternalArrayType&)> ExternalArraySetter;
		typedef DKFoundation::DKFunctionSignature<bool (const ExternalArrayType&)> ExternalArrayChecker;

		// external resource map (key-value pairs of String, Resource)
		typedef DKFoundation::DKFunctionSignature<void (ExternalMapType&)> ExternalMapGetter;
		typedef DKFoundation::DKFunctionSignature<void (ExternalMapType&)> ExternalMapSetter;
		typedef DKFoundation::DKFunctionSignature<bool (const ExternalMapType&)> ExternalMapChecker;

		// fault handler.
		// if you provide fault-handler, called on error situation to recover.
		typedef DKFoundation::DKOperation FaultHandler;
		typedef DKFoundation::DKFunctionSignature<void (State)> Callback;

		DKSerializer(void);
		virtual ~DKSerializer(void);

		void SetCallback(Callback* c);
		void SetResourceClass(const DKFoundation::DKString& rc);
		DKFoundation::DKString ResourceClass(void) const;

		bool Bind(const DKFoundation::DKString& key, DKSerializer* s, FaultHandler* faultHandler);
		bool Bind(const DKFoundation::DKString& key, ValueGetter* getter, ValueSetter* setter, ValueChecker* checker, FaultHandler* faultHandler);
		bool Bind(const DKFoundation::DKString& key, ExternalGetter* getter, ExternalSetter* setter, ExternalChecker* checker, ExternalResource ext, FaultHandler* faultHandler);
		bool Bind(const DKFoundation::DKString& key, ExternalArrayGetter* getter, ExternalArraySetter* setter, ExternalArrayChecker* checker, ExternalResource ext, FaultHandler* faultHandler);
		bool Bind(const DKFoundation::DKString& key, ExternalMapGetter* getter, ExternalMapSetter* setter, ExternalMapChecker* checker, ExternalResource ext, FaultHandler* faultHandler);
		void Unbind(const DKFoundation::DKString& key);

		DKFoundation::DKObject<DKFoundation::DKData> Serialize(SerializeForm sf) const;
		size_t Serialize(SerializeForm sf, DKFoundation::DKStream* output) const;
		DKFoundation::DKObject<DKFoundation::DKXMLElement> SerializeXML(SerializeForm sf) const;
		bool Deserialize(const DKFoundation::DKXMLElement* e, DKResourceLoader* p) const;
		bool Deserialize(DKFoundation::DKStream* s, DKResourceLoader* p) const;
		bool Deserialize(const DKFoundation::DKData* d, DKResourceLoader* p) const;

		typedef DKFoundation::DKFunctionSignature<DKFoundation::DKObject<DKSerializer> (const DKFoundation::DKString&)> Selector;
		static bool RestoreObject(const DKFoundation::DKXMLElement* e, DKResourceLoader* p, Selector* sel);
		static bool RestoreObject(DKFoundation::DKStream* s, DKResourceLoader* p, Selector* sel);
		static bool RestoreObject(const DKFoundation::DKData* d, DKResourceLoader* p, Selector* sel);

	private:
		struct VariantEntity;
		struct SerializerEntity;
		struct ExternalEntity;
		struct ExternalEntityArray;
		struct ExternalEntityMap;
		struct Entity
		{
			// fault-handler, should be serialized without conditions if faultHandler is NULL.
			DKFoundation::DKObject<FaultHandler> faultHandler;

			virtual ~Entity(void)											{}
			virtual const VariantEntity*		Variant(void) const			{return NULL;}
			virtual const SerializerEntity*		Serializer(void) const		{return NULL;}
			virtual const ExternalEntity*		External(void) const		{return NULL;}
			virtual const ExternalEntityArray*	ExternalArray(void) const	{return NULL;}
			virtual const ExternalEntityMap*	ExternalMap(void) const		{return NULL;}
		};
		struct VariantEntity : public Entity
		{
			DKFoundation::DKObject<ValueGetter>		getter; // component getter
			DKFoundation::DKObject<ValueSetter>		setter; // component setter
			DKFoundation::DKObject<ValueChecker>	checker; // component checker, called on deserialize if not NULL.
			const VariantEntity*	Variant(void) const				{return this;}
		};
		struct SerializerEntity : public Entity
		{
			DKFoundation::DKObject<DKSerializer> serializer;
			const SerializerEntity*		Serializer(void) const		{return this;}
		};
		struct ExternalEntity : public Entity
		{
			DKFoundation::DKObject<ExternalGetter>	getter; // external resource getter
			DKFoundation::DKObject<ExternalSetter>	setter; // external resource setter
			DKFoundation::DKObject<ExternalChecker>	checker; // checker, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntity*		External(void) const		{return this;}
		};
		struct ExternalEntityArray : public Entity
		{
			DKFoundation::DKObject<ExternalArrayGetter>		getter; // getter for external resource array
			DKFoundation::DKObject<ExternalArraySetter>		setter; // setter for external resource array
			DKFoundation::DKObject<ExternalArrayChecker>	checker; // checker, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntityArray*	ExternalArray(void) const		{return this;}
		};
		struct ExternalEntityMap : public Entity
		{
			DKFoundation::DKObject<ExternalMapGetter>	getter; // getter for external resource map
			DKFoundation::DKObject<ExternalMapSetter>	setter; // setter for external resource map
			DKFoundation::DKObject<ExternalMapChecker>	checker; // checker, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntityMap*	ExternalMap(void) const			{return this;}
		};

		typedef DKFoundation::DKMap<DKFoundation::DKString, Entity*> EntityMap;
		EntityMap entityMap;
		DKFoundation::DKString resourceClass;
		DKFoundation::DKObject<Callback> callback;
		DKFoundation::DKSpinLock	lock;

		struct DeserializerEntity		// data for object restoration.
		{
			ValueType rootValue;
			DKFoundation::DKMap<DKFoundation::DKString, ExternalType> externals;
			DKFoundation::DKMap<DKFoundation::DKString, ExternalArrayType> externalArrays;
			DKFoundation::DKMap<DKFoundation::DKString, ExternalMapType> externalMaps;
			DKFoundation::DKArray<DKFoundation::DKObject<DKFoundation::DKOperation>> operations;
			DKFoundation::DKObject<Callback> callback;
		};

		bool DeserializeXMLOperations(const DKFoundation::DKXMLElement* e, DKFoundation::DKArray<DKFoundation::DKObject<DeserializerEntity>>& entities, DKResourceLoader* pool) const;
		bool DeserializeBinaryOperations(DKFoundation::DKStream* s, DKFoundation::DKArray<DKFoundation::DKObject<DeserializerEntity>>& entities, DKResourceLoader* pool) const;
		size_t SerializeBinary(SerializeForm sf, DKFoundation::DKStream* output) const;
		bool DeserializeBinary(DKFoundation::DKStream* s, DKResourceLoader* p) const;
		static bool DeserializeBinary(DKFoundation::DKStream* s, DKResourceLoader* p, Selector* sel);
		
		// copy constructor not allowed.
		DKSerializer(const DKSerializer&);
		DKSerializer& operator = (const DKSerializer&);

		class EntityRestore;
	};
}
