//
//  File: DKUpdateQueue.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKFoundation.h"


namespace DKFramework
{
	/// @brief
	/// object update queue, enables parallel updates
	/// @see DKParallelUpdateQueue for parallel update.
	/// @see DKSeiralUpdateQueue for serial update.
	class DKUpdateQueue
	{
	public:
		struct Synchronizer
		{
			DKTimeTick tick;
			double tickDelta;
			DKDateTime tickDate;

			virtual void SetComplete(void) = 0; ///< Set this update process is just finished.
			virtual void Synchronize(void*) = 0; ///< Wait for other object to finish
			virtual void Enqueue(DKFunctionSignature<void(Synchronizer&)>) = 0; ///< enqueue follow up object
		};

		DKUpdateQueue(void) {}
		virtual ~DKUpdateQueue(void) {}

		virtual void Complete(void) = 0; ///< Wait all objects are finished.
		virtual void Enqueue(DKFunctionSignature<void(Synchronizer&)> fn) = 0;

		DKTimeTick tick;
		double tickDelta;
		DKDateTime	tickDate;
	};
	using DKUpdateQueueSynchronizer = DKUpdateQueue::Synchronizer;

	/// @brief
	/// serial update queue
	class DKGL_API DKSerialUpdateQueue : public DKUpdateQueue
	{
	public:
		DKSerialUpdateQueue(void);
		~DKSerialUpdateQueue(void);
	};

	/// @brief
	/// parallel (multi threaded) update queue
	class DKGL_API DKParallelUpdateQueue : public DKUpdateQueue
	{
	public:
		DKParallelUpdateQueue(void);
		~DKParallelUpdateQueue(void);
	};
}
