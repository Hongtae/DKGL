//
//  File: DKPropertySet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVariant.h"

////////////////////////////////////////////////////////////////////////////////
// DKPropertySet
// Sotre key-value pair set, that can be imported from URL or file.
// You can export your set into file also.
// The key-value set notifies by callback when data has modified,
// created, deleted. You can register your callback functions.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	class DKGL_API DKPropertySet
	{
	public:
		// DefaultSet: user preferences. You can export, import from file.
		static DKPropertySet& DefaultSet(void);
		// SystemConfig: default system config. do not save or export this.
		static DKPropertySet& SystemConfig(void);

		using InsertionFunc = void (DKString, DKVariant);				// key, new-value
		using ModificationFunc = void (DKString, DKVariant, DKVariant);	// key, old-value, new-value
		using DeletionFunc = void (DKString, DKVariant);				// key, old-value

		using InsertionCallback = DKFunctionSignature<InsertionFunc>;
		using ModificationCallback = DKFunctionSignature<ModificationFunc>;
		using DeletionCallback = DKFunctionSignature<DeletionFunc>;

		using Enumerator = DKFunctionSignature<void (const DKString&, const DKVariant&)>;

		DKPropertySet(void);
		~DKPropertySet(void);

		// Import, import from file or URL
		// returns number of pairs imported or -1 if import failed.
		int Import(const DKString& url, bool overwrite);
		int Import(const DKPropertySet& prop, bool overwrite);
		int Import(const DKXmlElement* e, bool overwrite);
		int Import(DKStream* stream, bool overwrite);
		// Export to file, returns number of pairs exported or -1 if export filed.
		// You can export with XML or binary format
		int Export(const DKString& file, bool exportXML) const;
		int Export(DKStream* stream, bool exportXML) const;
		DKObject<DKXmlElement> Export(bool exportXML, int* numExported) const;

		// SetInitialValue, add new value for key, or fail if key is exits already.
		bool SetInitialValue(const DKString& key, const DKVariant& value);

		void SetValue(const DKString& key, const DKVariant& value);
		const DKVariant& Value(const DKString& key) const;
		bool HasValue(const DKString& key) const;
		void Remove(const DKString& key);
		size_t NumberOfEntries(void) const;

		// Add / Remove callback for key insertion, modification, deletion.
		using ObserverContext = const void*;
		void AddObserver(ObserverContext context, const DKString& key, InsertionCallback* insertion, ModificationCallback* modification, DeletionCallback* deletion);
		void RemoveObserver(ObserverContext context, const DKString& key);
		void RemoveObserver(ObserverContext context); // remove all keys for context

		// enumerate all key, value pairs. (read-only)
		void EnumerateForward(const Enumerator* e) const;
		void EnumerateBackward(const Enumerator* e) const;
				
	private:
		DKSpinLock lock;
		typedef DKMap<DKString, DKVariant> PropertyMap;
		PropertyMap properties;

		DKSpinLock callbackLock;
		template <typename T> using ObserverMap = DKMap<ObserverContext, T>;
		DKMap<DKString, ObserverMap<DKObject<InsertionCallback>>> insertionCallbacks;
		DKMap<DKString, ObserverMap<DKObject<ModificationCallback>>> modificationCallbacks;
		DKMap<DKString, ObserverMap<DKObject<DeletionCallback>>> deletionCallbacks;

		template <typename T, typename... Args> void CallbackObservers(const DKString& key, const DKMap<DKString, ObserverMap<DKObject<T>>>& target, Args&&... args) const;
	};
}
