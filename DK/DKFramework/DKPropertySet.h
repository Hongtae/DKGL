//
//  File: DKPropertySet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"
#include "DKVariant.h"

namespace DKFramework
{
	/// @brief Sotre key-value pair set, that can be imported from URL or file.
	///
	/// You can export your set into file also.
	/// The key-value set notifies by callback when data has modified,
	/// created, deleted. You can register your callback functions.
	class DKGL_API DKPropertySet
	{
	public:
		/// user preferences. You can export, import from file.
		static DKPropertySet& DefaultSet();
		/// default system config. do not save or export this.
		static DKPropertySet& SystemConfig();

		using InsertionFunc = void (DKString, DKVariant);				///< a function type of insertion (key, new-value)
		using ModificationFunc = void (DKString, DKVariant, DKVariant);	///< a function type of modification (key, old-value, new-value)
		using DeletionFunc = void (DKString, DKVariant);				///< a function type of deletion (key, old-value)

		using InsertionCallback = DKFunctionSignature<InsertionFunc>;
		using ModificationCallback = DKFunctionSignature<ModificationFunc>;
		using DeletionCallback = DKFunctionSignature<DeletionFunc>;

		using Enumerator = DKFunctionSignature<void (const DKString&, const DKVariant&)>;
		using Replacer = DKFunctionSignature<DKVariant (const DKVariant&)>;

		DKPropertySet();
		~DKPropertySet();

		/// Import, import from file or URL
		/// @return number of pairs imported or -1 if import failed.
		int Import(const DKString& url, bool overwrite);
		int Import(const DKPropertySet& prop, bool overwrite);
		int Import(const DKXmlElement* e, bool overwrite);
		int Import(DKStream* stream, bool overwrite);
		/// Export to file, returns number of pairs exported or -1 if export filed.
		/// You can export with XML or binary format
		int Export(const DKString& file, bool exportXML) const;
		int Export(DKStream* stream, bool exportXML) const;
		DKObject<DKXmlElement> Export(bool exportXML, int* numExported) const;

		/// add new value for key, or fail if key is exits already.
		bool SetInitialValue(const DKString& key, const DKVariant& value);

		void SetValue(const DKString& key, const DKVariant& value);
		/// Atomically examine and update value.
		/// Returning DKVariant::TypeUndefined the matching key will be removed.
		/// Never call DKPropertySet member functions inside callback!
		void ReplaceValue(const DKString& key, Replacer* replacer);

		const DKVariant& Value(const DKString& key) const;
		bool HasValue(const DKString& key) const;
		void Remove(const DKString& key);
		size_t NumberOfEntries() const;

		/// search all values that matching key-path.
		bool LookUpValueForKeyPath(const DKString& path, DKVariant::ConstKeyPathEnumerator* callback) const;

		// Add / Remove callback for key insertion, modification, deletion.
		using ObserverContext = const void*;
		/// add callback for key insertion, modification, deletion
		void AddObserver(ObserverContext context, const DKString& key, InsertionCallback* insertion, ModificationCallback* modification, DeletionCallback* deletion);
		void RemoveObserver(ObserverContext context, const DKString& key);
		void RemoveObserver(ObserverContext context); // remove all keys for context

		/// enumerate all key, value pairs. (read-only)
		void EnumerateForward(const Enumerator* e) const;
		void EnumerateBackward(const Enumerator* e) const;
				
	private:
		DKSpinLock lock;
		using PropertyMap = DKVariant::VPairs;
		DKVariant dataSet;

		DKSpinLock callbackLock;
		template <typename T> using ObserverMap = DKMap<ObserverContext, T>;
		DKMap<DKString, ObserverMap<DKObject<InsertionCallback>>> insertionCallbacks;
		DKMap<DKString, ObserverMap<DKObject<ModificationCallback>>> modificationCallbacks;
		DKMap<DKString, ObserverMap<DKObject<DeletionCallback>>> deletionCallbacks;

		template <typename T, typename... Args>
		void CallbackObservers(const DKString& key, const DKMap<DKString, ObserverMap<DKObject<T>>>& target, Args&&... args) const;
	};
}
