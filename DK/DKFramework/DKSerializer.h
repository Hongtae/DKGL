//
//  File: DKSerializer.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVariant.h"

namespace DKFramework
{
	class DKResource;
	class DKResourceLoader;
	/// @brief
	/// Object serializer, deserializer class.
	///
	/// @details
	/// Using XML or binary formatted data, XML formatted data can be transferred on
	/// text-based protocols.
	/// Using DKVariant for internal components, You can also have external
	/// resources. Each external resource can have separate serializer object.
	/// You can embed any external resources in a single serializer object.
	/// External resources could be single or array, map with String key.
	/// You can also embed any serializer objects in a single serializer object.
	///
	/// Once a class object generates serializer object, then object can be
	/// serializable, deserializable by single serializer object.
	/// On generating serializer object, you need to define your components which
	/// sould be embedded. You can define your components getter, setter with
	/// DKFunction.
	///
	/// For external resources, You can specify how resource can be refered.
	/// see ExternalResource enums on class declarations below.
	///
	/// You may need DKResourceLoader object for deserialize object. because of
	/// referred external resources can be loaded separately by DKResourceLoader.
	///
	/// An external resource should have a name as file-name to be loaded separately.
	/// Unnamed resource should be embedded and not valid as references which loads
	/// separately while deserialize in progress.
	///
	/// On any errors has occurred, serializer calls fault-handler, if not NULL.
	/// You may need to provide fault-handler to control error by conditions.
	/// If you don't provde fault-handler, deserialization process succeeded only if
	/// no error has occurred. (missing files, file has wrong types will stop
	/// deserialize process.)
	///
	/// You can also privdes 'validator' operation, which will be called on
	/// deserialization process. If operation provided and called, return true for
	/// valid state, return false to invalid state and stop deserialize.
	///
	/// Behaviors on serialize:
	///    - if getter is NULL, entry ignored and will not be serialized.
	///    - if faultHandler is NULL, error on external resource failed.
	/// Behaviors on deserialize:
	///    - if setter is NULL, entry ignored and will not be restored.
	///    - if setter is not NULL and faultHandler is NULL, regarded as deserialized
	///      without any error.
	///    - if faultHandler is not NULL, will be called when data is missing.
	///    - if validator is NULL, will not validate entry.
	///    - if validator is not NULL, will be called and returns true for validate,
	///    or generates an error if validator returns false. (error can be recovered
	///    by faultHandler, if available.)
	///
	///
	/// @note
	///    You can use this class for not only DKResource object but any types of
	///    object also. But external resources should be DKResource.
	///    (You need wrapper class for non DKResource object)
	class DKSerializer
	{
	public:
		/// SerializeForm
		/// - SerializeFormXML:
		///    - xml format plane text. (can be large)
		/// - SerializeFormBinXML:
		///    - xml format text with compressed contents.
		///    - useful for text based transfer protocols.
		/// - SerializeFormBinary:
		///    - uncompressed binary format. (faster loading)
		/// - SerializeFormCompressedBinary:
		///    - compressed binary format. (smallest, faster than xml)
		enum SerializeForm : int
		{
			SerializeFormXML				= '_XML',
			SerializeFormBinXML				= 'bXML',
			SerializeFormBinary				= '_BIN',
			SerializeFormCompressedBinary	= 'cBIN',
		};
		/// serialize/deserialize callback state
		enum State
		{
			StateSerializeBegin = 0,  ///< On serialization begins
			StateSerializeSucceed,    ///< On serialization succeeded
			StateSerializeFailed,     ///< On serialization failed
			StateDeserializeBegin,    ///< On deserialization begins
			StateDeserializeSucceed,  ///< On deserialization succeeded
			StateDeserializeFailed,   ///< On deserialization failed
		};
		/// defining referencing external resource
		enum ExternalResource
		{
			ExternalResourceInclude = 0,         ///< included in current serializer.
			ExternalResourceReferenceIfPossible, ///< reference as external if possible.
			ExternalResourceForceReference,      ///< force reference (never include)
		};

		/// using DKVariant as components container.
		typedef DKVariant ValueType;
		/// DKResource type can be reference to external resource.
		/// external resources not included in this serializer.
		/// external resources will be loaded separately with this serailizer.
		typedef DKObject<DKResource> ExternalType;
		typedef DKArray<ExternalType> ExternalArrayType;
		typedef DKMap<DKString, ExternalType> ExternalMapType;

		/// component value getter, setter, validator
		/// when data needs to be validated, calling validator to validate if validator is not NULL.
		typedef DKFunctionSignature<void (ValueType&)> ValueGetter;
		typedef DKFunctionSignature<void (ValueType&)> ValueSetter;
		typedef DKFunctionSignature<bool (const ValueType&)> ValueValidator;

		/// external resource getter, setter, validator
		typedef DKFunctionSignature<void (ExternalType&)> ExternalGetter;
		typedef DKFunctionSignature<void (ExternalType&)> ExternalSetter;
		typedef DKFunctionSignature<bool (const ExternalType&)> ExternalValidator;

		/// external resource array
		typedef DKFunctionSignature<void (ExternalArrayType&)> ExternalArrayGetter;
		typedef DKFunctionSignature<void (ExternalArrayType&)> ExternalArraySetter;
		typedef DKFunctionSignature<bool (const ExternalArrayType&)> ExternalArrayValidator;

		/// external resource map (key-value pairs of String, Resource)
		typedef DKFunctionSignature<void (ExternalMapType&)> ExternalMapGetter;
		typedef DKFunctionSignature<void (ExternalMapType&)> ExternalMapSetter;
		typedef DKFunctionSignature<bool (const ExternalMapType&)> ExternalMapValidator;

		/// fault handler.
		/// if you provide fault-handler, called on error situation to recover.
		typedef DKOperation FaultHandler;
		typedef DKFunctionSignature<void (State)> Callback;

		DKSerializer(void);
		virtual ~DKSerializer(void);

		void SetCallback(Callback* c);
		void SetResourceClass(const DKString& rc);
		DKString ResourceClass(void) const;

		bool Bind(const DKString& key, DKSerializer* s, FaultHandler* faultHandler);
		bool Bind(const DKString& key, ValueGetter* getter, ValueSetter* setter, ValueValidator* validator, FaultHandler* faultHandler);
		bool Bind(const DKString& key, ExternalGetter* getter, ExternalSetter* setter, ExternalValidator* validator, ExternalResource ext, FaultHandler* faultHandler);
		bool Bind(const DKString& key, ExternalArrayGetter* getter, ExternalArraySetter* setter, ExternalArrayValidator* validator, ExternalResource ext, FaultHandler* faultHandler);
		bool Bind(const DKString& key, ExternalMapGetter* getter, ExternalMapSetter* setter, ExternalMapValidator* validator, ExternalResource ext, FaultHandler* faultHandler);
		void Unbind(const DKString& key);

		DKObject<DKData> Serialize(SerializeForm sf) const;
		size_t Serialize(SerializeForm sf, DKStream* output) const;
		DKObject<DKXmlElement> SerializeXML(SerializeForm sf) const;
		bool Deserialize(const DKXmlElement* e, DKResourceLoader* p) const;
		bool Deserialize(DKStream* s, DKResourceLoader* p) const;
		bool Deserialize(const DKData* d, DKResourceLoader* p) const;

		typedef DKFunctionSignature<DKObject<DKSerializer> (const DKString&)> Selector;
		static bool RestoreObject(const DKXmlElement* e, DKResourceLoader* p, Selector* sel);
		static bool RestoreObject(DKStream* s, DKResourceLoader* p, Selector* sel);
		static bool RestoreObject(const DKData* d, DKResourceLoader* p, Selector* sel);

		DKByteOrder outputStreamByteOrder;	///< output binary byte-order

	private:
		struct VariantEntity;
		struct SerializerEntity;
		struct ExternalEntity;
		struct ExternalEntityArray;
		struct ExternalEntityMap;
		struct Entity
		{
			// fault-handler, should be serialized without conditions if faultHandler is NULL.
			DKObject<FaultHandler> faultHandler;

			virtual ~Entity(void)											{}
			virtual const VariantEntity*		Variant(void) const			{return NULL;}
			virtual const SerializerEntity*		Serializer(void) const		{return NULL;}
			virtual const ExternalEntity*		External(void) const		{return NULL;}
			virtual const ExternalEntityArray*	ExternalArray(void) const	{return NULL;}
			virtual const ExternalEntityMap*	ExternalMap(void) const		{return NULL;}
		};
		struct VariantEntity : public Entity
		{
			DKObject<ValueGetter>		getter; // component getter
			DKObject<ValueSetter>		setter; // component setter
			DKObject<ValueValidator>	validator; // component validator, called on deserialize if not NULL.
			const VariantEntity*	Variant(void) const				{return this;}
		};
		struct SerializerEntity : public Entity
		{
			DKObject<DKSerializer> serializer;
			const SerializerEntity*		Serializer(void) const		{return this;}
		};
		struct ExternalEntity : public Entity
		{
			DKObject<ExternalGetter>		getter; // external resource getter
			DKObject<ExternalSetter>		setter; // external resource setter
			DKObject<ExternalValidator>	validator; // validator, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntity*		External(void) const		{return this;}
		};
		struct ExternalEntityArray : public Entity
		{
			DKObject<ExternalArrayGetter>		getter; // getter for external resource array
			DKObject<ExternalArraySetter>		setter; // setter for external resource array
			DKObject<ExternalArrayValidator>	validator; // validator, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntityArray*	ExternalArray(void) const		{return this;}
		};
		struct ExternalEntityMap : public Entity
		{
			DKObject<ExternalMapGetter>		getter; // getter for external resource map
			DKObject<ExternalMapSetter>		setter; // setter for external resource map
			DKObject<ExternalMapValidator>	validator; // validator, called on deserialize if not NULL.
			ExternalResource			external;
			const ExternalEntityMap*	ExternalMap(void) const			{return this;}
		};

		typedef DKMap<DKString, Entity*> EntityMap;
		EntityMap entityMap;
		DKString resourceClass;
		DKObject<Callback> callback;
		DKSpinLock	lock;

		struct DeserializerEntity		// data for object restoration.
		{
			ValueType rootValue;
			DKMap<DKString, ExternalType> externals;
			DKMap<DKString, ExternalArrayType> externalArrays;
			DKMap<DKString, ExternalMapType> externalMaps;
			DKArray<DKObject<DKOperation>> operations;
			DKObject<Callback> callback;
		};

		bool DeserializeXMLOperations(const DKXmlElement* e, DKArray<DKObject<DeserializerEntity>>& entities, DKResourceLoader* pool) const;
		bool DeserializeBinaryOperations(DKStream* s, DKArray<DKObject<DeserializerEntity>>& entities, DKResourceLoader* pool) const;
		size_t SerializeBinary(SerializeForm sf, DKStream* output) const;
		bool DeserializeBinary(DKStream* s, DKResourceLoader* p) const;
		static bool DeserializeBinary(DKStream* s, DKResourceLoader* p, Selector* sel);
		
		// copy constructor not allowed.
		DKSerializer(const DKSerializer&) = delete;
		DKSerializer& operator = (const DKSerializer&) = delete;

		class EntityRestore;
	};
}
