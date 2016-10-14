//
//  File: DKPropertySet.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2015 Hongtae Kim. All rights reserved.
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

namespace DKGL
{
	class DKGL_API DKPropertySet
	{
	public:
		static DKPropertySet& DefaultSet(void);

		typedef void InsertionFunc(DKString, DKVariant);					// key, new-value
		typedef void ModificationFunc(DKString, DKVariant, DKVariant);	// key, old-value, new-value
		typedef void DeletionFunc(DKString, DKVariant);					// key, old-value

		typedef DKFunctionSignature<InsertionFunc>		InsertionCallback;
		typedef DKFunctionSignature<ModificationFunc>		ModificationCallback;
		typedef DKFunctionSignature<DeletionFunc>			DeletionCallback;
		typedef DKFunctionSignature<void (const DKString&, const DKVariant&)> Enumerator;

		DKPropertySet(void);
		~DKPropertySet(void);

		// Import, import from file or URL
		// returns number of pairs imported or -1 if import failed.
		int Import(const DKString& url, bool overwrite);
		int Import(const DKPropertySet& prop, bool overwrite);
		int Import(const DKXMLElement* e, bool overwrite);
		int Import(DKStream* stream, bool overwrite);
		// Export to file, returns number of pairs exported or -1 if export filed.
		// You can export with XML or binary format
		int Export(const DKString& file, bool exportXML) const;
		int Export(DKStream* stream, bool exportXML) const;
		DKObject<DKXMLElement> Export(bool exportXML, int* numExported) const;

		// SetInitialValue, add new value for key, or fail if key is exits already.
		bool SetInitialValue(const DKString& key, const DKVariant& value);

		void SetValue(const DKString& key, const DKVariant& value);
		const DKVariant& Value(const DKString& key) const;
		bool HasValue(const DKString& key) const;
		void Remove(const DKString& key);
		size_t NumberOfEntries(void) const;

		// Add / Remove callback for key insertion, modification, deletion.
		void SetCallback(const DKString& key, InsertionCallback* insertion, ModificationCallback* modification, DeletionCallback* deletion, DKRunLoop* runLoop, void* context);
		void RemoveCallback(const DKString& key, void* context);
		void RemoveCallback(void* context);

		// enumerate all key, value pairs. (read-only)
		void EnumerateForward(const Enumerator* e) const;
		void EnumerateBackward(const Enumerator* e) const;
				
	private:
		DKSpinLock lock;
		typedef DKMap<DKString, DKVariant> PropertyMap;
		PropertyMap properties;

		typedef DKMap<DKString, DKCallback<InsertionFunc, void*, DKSpinLock>>		InsertionCallbackMap;
		typedef DKMap<DKString, DKCallback<ModificationFunc, void*, DKSpinLock>>	ModificationCallbackMap;
		typedef DKMap<DKString, DKCallback<DeletionFunc, void*, DKSpinLock>>		DeletionCallbackMap;

		InsertionCallbackMap		insertionCallbacks;
		ModificationCallbackMap		modificationCallbacks;
		DeletionCallbackMap			deletionCallbacks;
	};
}
