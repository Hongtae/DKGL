//
//  File: DKPropertySet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
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
	class DKLIB_API DKPropertySet
	{
	public:
		static DKPropertySet& DefaultSet(void);

		typedef void InsertionFunc(DKFoundation::DKString, DKVariant);					// key, new-value
		typedef void ModificationFunc(DKFoundation::DKString, DKVariant, DKVariant);	// key, old-value, new-value
		typedef void DeletionFunc(DKFoundation::DKString, DKVariant);					// key, old-value

		typedef DKFoundation::DKFunctionSignature<InsertionFunc>		InsertionCallback;
		typedef DKFoundation::DKFunctionSignature<ModificationFunc>		ModificationCallback;
		typedef DKFoundation::DKFunctionSignature<DeletionFunc>			DeletionCallback;
		typedef DKFoundation::DKFunctionSignature<void (const DKFoundation::DKString&, const DKVariant&)> Enumerator;

		DKPropertySet(void);
		~DKPropertySet(void);

		// Import, import from file or URL
		// returns number of pairs imported or -1 if import failed.
		int Import(const DKFoundation::DKString& url, bool overwrite);
		int Import(const DKPropertySet& prop, bool overwrite);
		int Import(const DKFoundation::DKXMLElement* e, bool overwrite);
		int Import(DKFoundation::DKStream* stream, bool overwrite);
		// Export to file, returns number of pairs exported or -1 if export filed.
		// You can export with XML or binary format
		int Export(const DKFoundation::DKString& file, bool exportXML) const;
		int Export(DKFoundation::DKStream* stream, bool exportXML) const;
		DKFoundation::DKObject<DKFoundation::DKXMLElement> Export(bool exportXML, int* numExported) const;

		// SetInitialValue, add new value for key, or fail if key is exits already.
		bool SetInitialValue(const DKFoundation::DKString& key, const DKVariant& value);

		void SetValue(const DKFoundation::DKString& key, const DKVariant& value);
		const DKVariant& Value(const DKFoundation::DKString& key) const;
		bool HasValue(const DKFoundation::DKString& key) const;
		void Remove(const DKFoundation::DKString& key);
		size_t NumberOfEntries(void) const;

		// Add / Remove callback for key insertion, modification, deletion.
		void SetCallback(const DKFoundation::DKString& key, InsertionCallback* insertion, ModificationCallback* modification, DeletionCallback* deletion, DKFoundation::DKRunLoop* runLoop, void* context);
		void RemoveCallback(const DKFoundation::DKString& key, void* context);
		void RemoveCallback(void* context);

		// enumerate all key, value pairs. (read-only)
		void EnumerateForward(const Enumerator* e) const;
		void EnumerateBackward(const Enumerator* e) const;
				
	private:
		DKFoundation::DKSpinLock lock;
		typedef DKFoundation::DKMap<DKFoundation::DKString, DKVariant> PropertyMap;
		PropertyMap properties;

		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKCallback<InsertionFunc, void*, DKFoundation::DKSpinLock>>		InsertionCallbackMap;
		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKCallback<ModificationFunc, void*, DKFoundation::DKSpinLock>>	ModificationCallbackMap;
		typedef DKFoundation::DKMap<DKFoundation::DKString, DKFoundation::DKCallback<DeletionFunc, void*, DKFoundation::DKSpinLock>>		DeletionCallbackMap;

		InsertionCallbackMap		insertionCallbacks;
		ModificationCallbackMap		modificationCallbacks;
		DeletionCallbackMap			deletionCallbacks;
	};
}
