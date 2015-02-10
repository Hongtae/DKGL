//
//  File: DKSharedInstance.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKSpinLock.h"
#include "DKCondition.h"
#include "DKCriticalSection.h"

////////////////////////////////////////////////////////////////////////////////
// DKSharedInstance
// a shared instance interface.
// Object can be shared like singleton, but deleted automatically if no objects has references.
// You can re-create instance by calling SharedInstance() after previous object deleted.
//
// You can define your type by 'typedef' with DKSharedInstance or
// You can subclass inheritanced from DKSharedInstance.
//
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <class TYPE> class DKSharedInstance
	{
	public:
		static DKObject<TYPE> SharedInstance(void)
		{
			DKCriticalSection<DKSpinLock> cs(creatorLock);

			DKObject<TYPE> obj = NULL;
			if (obj == NULL)
			{
				DKCriticalSection<DKSpinLock> guard(sharedRefLock);
				obj = sharedRef;

				// IsManaged() returns false when object is being deleted.
				if (obj.IsManaged() == false)
					obj = NULL;
			}
			if (obj == NULL)  // no object or deleting in progress.
			{
				creatorCond.Lock();
				while (creator != NULL)
					creatorCond.Wait(); // wait until completely destroyed.

				DKCriticalSection<DKSpinLock> guard(sharedRefLock);

				DKASSERT_DEBUG(creator == NULL);

				obj = DKObject<TYPE>::New();
				sharedRef = obj;
				creator = obj.template SafeCast<DKSharedInstance>();

				creatorCond.Unlock();

				DKASSERT_DESC_DEBUG(creator != NULL, "Object creation failure!");
			}
			DKASSERT_DESC_DEBUG(obj != NULL, "Object creation failure!");
			return obj;
		}
		static DKObject<TYPE> GetSharedInstanceIfExist(void)
		{
			DKCriticalSection<DKSpinLock> guard(sharedRefLock);
			if (creator)
			{
				DKObject<TYPE> obj = sharedRef;
				if (obj.IsManaged())
					return obj;
			}
			return NULL;
		}
	protected:
		DKSharedInstance(void)
		{
		}
		virtual ~DKSharedInstance(void)
		{
			DKCriticalSection<DKSpinLock> guard(sharedRefLock);
			if (creator == this)
			{
				creatorCond.Lock();
				creator = NULL;
				sharedRef = ObjectRef();
				creatorCond.Signal();
				creatorCond.Unlock();
			}
		}
	private:
		typedef typename DKObject<TYPE>::Ref	ObjectRef;

		// Ref type of shared object.
		static ObjectRef			sharedRef;
		// save original type while object turns out of non-polymorphic in destorying process.
		static DKSharedInstance*	creator;
		// lock for sharedInstance.
		static DKSpinLock			sharedRefLock;
		// lock for critical-section of object creation.
		static DKSpinLock			creatorLock;
		// lock for object were deleted completely.
		static DKCondition			creatorCond;
	};

	template <typename TYPE> typename DKSharedInstance<TYPE>::ObjectRef DKSharedInstance<TYPE>::sharedRef;
	template <typename TYPE> DKSharedInstance<TYPE>* DKSharedInstance<TYPE>::creator = NULL;
	template <typename TYPE> DKSpinLock DKSharedInstance<TYPE>::sharedRefLock;
	template <typename TYPE> DKSpinLock DKSharedInstance<TYPE>::creatorLock;
	template <typename TYPE> DKCondition DKSharedInstance<TYPE>::creatorCond;
}
