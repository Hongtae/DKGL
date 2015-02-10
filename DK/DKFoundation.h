#pragma once

#ifdef _MSC_VER
// visual studio 2013 still not support some features of C++11.
// DKFoundation_msvc.h is should be used for msvc (vs2013 for now)
#include "DKFoundation_msvc.h"
#else

#include "DKInclude.h"

// basic object templates and memory management.
#include "DKFoundation/DKMemory.h"
#include "DKFoundation/DKObject.h"
#include "DKFoundation/DKAllocator.h"
#include "DKFoundation/DKTypeInfo.h"
#include "DKFoundation/DKTypeList.h"
#include "DKFoundation/DKTypeTraits.h"
#include "DKFoundation/DKSingleton.h"
#include "DKFoundation/DKSharedInstance.h"

// unicode string
#include "DKFoundation/DKString.h"
#include "DKFoundation/DKStringU8.h"

// data collections
#include "DKFoundation/DKArray.h"
#include "DKFoundation/DKCircularQueue.h"
#include "DKFoundation/DKList.h"
#include "DKFoundation/DKMap.h"
#include "DKFoundation/DKOrderedArray.h"
#include "DKFoundation/DKSet.h"
#include "DKFoundation/DKStack.h"
#include "DKFoundation/DKStaticArray.h"
#include "DKFoundation/DKTuple.h"
#include "DKFoundation/DKQueue.h"

// hash, UUID
#include "DKFoundation/DKHash.h"
#include "DKFoundation/DKUUID.h"

// thread, mutex, synchronization objects.
#include "DKFoundation/DKAtomicNumber32.h"
#include "DKFoundation/DKAtomicNumber64.h"
#include "DKFoundation/DKCriticalSection.h"
#include "DKFoundation/DKDummyLock.h"
#include "DKFoundation/DKFence.h"
#include "DKFoundation/DKLock.h"
#include "DKFoundation/DKMutex.h"
#include "DKFoundation/DKSharedLock.h"
#include "DKFoundation/DKSpinLock.h"
#include "DKFoundation/DKThread.h"
#include "DKFoundation/DKCondition.h"

// stream, file, buffer, directory (file-system)
#include "DKFoundation/DKData.h"
#include "DKFoundation/DKStream.h"
#include "DKFoundation/DKDataStream.h"
#include "DKFoundation/DKBuffer.h"
#include "DKFoundation/DKBufferStream.h"
#include "DKFoundation/DKDirectory.h"
#include "DKFoundation/DKFile.h"
#include "DKFoundation/DKFileMap.h"
#include "DKFoundation/DKZipArchiver.h"
#include "DKFoundation/DKZipUnarchiver.h"

// XML
#include "DKFoundation/DKXMLParser.h"
#include "DKFoundation/DKXMLDocument.h"

// date time, timer
#include "DKFoundation/DKTimer.h"
#include "DKFoundation/DKDateTime.h"
#include "DKFoundation/DKRational.h"

// operation, invocation (function utilities)
#include "DKFoundation/DKFunction.h"
#include "DKFoundation/DKInvocation.h"
#include "DKFoundation/DKCallback.h"
#include "DKFoundation/DKOperation.h"
#include "DKFoundation/DKValue.h"

// run-loop, operation queue, message-handler
#include "DKFoundation/DKMessageQueue.h"
#include "DKFoundation/DKOperationQueue.h"
#include "DKFoundation/DKRunLoop.h"
#include "DKFoundation/DKRunLoopTimer.h"

// etc
#include "DKFoundation/DKEndianness.h"
#include "DKFoundation/DKError.h"
#include "DKFoundation/DKLog.h"
#include "DKFoundation/DKUtils.h"

#endif //ifdef _MSC_VER
