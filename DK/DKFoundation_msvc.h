#pragma once

#ifndef _MSC_VER
#error "Use this header only with Microsoft Visual C++ compilers!"  
#endif

#include "DKInclude.h"

// basic object templates and memory management.
#include "DKFoundation_msvc/DKMemory.h"
#include "DKFoundation_msvc/DKObject.h"
#include "DKFoundation_msvc/DKAllocator.h"
#include "DKFoundation_msvc/DKTypeInfo.h"
#include "DKFoundation_msvc/DKTypeList.h"
#include "DKFoundation_msvc/DKTypeTraits.h"
#include "DKFoundation_msvc/DKSingleton.h"
#include "DKFoundation_msvc/DKSharedInstance.h"

// unicode string
#include "DKFoundation_msvc/DKString.h"
#include "DKFoundation_msvc/DKStringU8.h"

// data collections
#include "DKFoundation_msvc/DKArray.h"
#include "DKFoundation_msvc/DKCircularQueue.h"
#include "DKFoundation_msvc/DKList.h"
#include "DKFoundation_msvc/DKMap.h"
#include "DKFoundation_msvc/DKOrderedArray.h"
#include "DKFoundation_msvc/DKSet.h"
#include "DKFoundation_msvc/DKStack.h"
#include "DKFoundation_msvc/DKStaticArray.h"
#include "DKFoundation_msvc/DKTuple.h"
#include "DKFoundation_msvc/DKQueue.h"

// hash, UUID
#include "DKFoundation_msvc/DKHash.h"
#include "DKFoundation_msvc/DKUUID.h"

// thread, mutex, synchronization objects.
#include "DKFoundation_msvc/DKAtomicNumber32.h"
#include "DKFoundation_msvc/DKAtomicNumber64.h"
#include "DKFoundation_msvc/DKCriticalSection.h"
#include "DKFoundation_msvc/DKDummyLock.h"
#include "DKFoundation_msvc/DKFence.h"
#include "DKFoundation_msvc/DKLock.h"
#include "DKFoundation_msvc/DKMutex.h"
#include "DKFoundation_msvc/DKSharedLock.h"
#include "DKFoundation_msvc/DKSpinLock.h"
#include "DKFoundation_msvc/DKThread.h"
#include "DKFoundation_msvc/DKCondition.h"

// stream, file, buffer, directory (file-system)
#include "DKFoundation_msvc/DKData.h"
#include "DKFoundation_msvc/DKStream.h"
#include "DKFoundation_msvc/DKDataStream.h"
#include "DKFoundation_msvc/DKBuffer.h"
#include "DKFoundation_msvc/DKBufferStream.h"
#include "DKFoundation_msvc/DKDirectory.h"
#include "DKFoundation_msvc/DKFile.h"
#include "DKFoundation_msvc/DKFileMap.h"
#include "DKFoundation_msvc/DKZipArchiver.h"
#include "DKFoundation_msvc/DKZipUnarchiver.h"

// XML
#include "DKFoundation_msvc/DKXMLParser.h"
#include "DKFoundation_msvc/DKXMLDocument.h"

// date time, timer
#include "DKFoundation_msvc/DKTimer.h"
#include "DKFoundation_msvc/DKDateTime.h"
#include "DKFoundation_msvc/DKRational.h"

// operation, invocation (function utilities)
#include "DKFoundation_msvc/DKFunction.h"
#include "DKFoundation_msvc/DKInvocation.h"
#include "DKFoundation_msvc/DKCallback.h"
#include "DKFoundation_msvc/DKOperation.h"
#include "DKFoundation_msvc/DKValue.h"

// run-loop, operation queue, message-handler
#include "DKFoundation_msvc/DKMessageQueue.h"
#include "DKFoundation_msvc/DKOperationQueue.h"
#include "DKFoundation_msvc/DKRunLoop.h"
#include "DKFoundation_msvc/DKRunLoopTimer.h"

// etc
#include "DKFoundation_msvc/DKEndianness.h"
#include "DKFoundation_msvc/DKError.h"
#include "DKFoundation_msvc/DKLog.h"
#include "DKFoundation_msvc/DKUtils.h"

