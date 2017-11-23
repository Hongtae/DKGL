#pragma once
#include "DKInclude.h"

/// @brief
///  Classes for supporting common platform features like file-system,
///  threading, date and time, etc.\n
///  Template data collection and abstration for generic data models are also
///  supported.
/// 
///  Supports following features:
///  - Object & Memory management
///  - Unicode string (UTF-8,16,32)
///  - Data Collection
///  - Hash, UUID
///  - Thread and Synchronization Objects. (Mutex, Cond, etc.)
///  - Stream, File, Buffer, File-system directory
///  - XML reader / writer
///  - Date Time (ISO-8601 support)
///  - Float16(half), Rational math type
///  - Event-Loop, Loop Timer, Scheduler
///  - Operation Queue, Thread Pool
///  - Error handler
///  - Process and environments info
namespace DKFoundation {}

// basic object templates and memory management.
#include "DKFoundation/DKMemory.h"
#include "DKFoundation/DKObject.h"
#include "DKFoundation/DKAllocator.h"
#include "DKFoundation/DKAllocatorChain.h"
#include "DKFoundation/DKFixedSizeAllocator.h"
#include "DKFoundation/DKTypes.h"
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
#include "DKFoundation/DKBitArray.h"
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
#include "DKFoundation/DKUuid.h"

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

// stream, buffer, compressor
#include "DKFoundation/DKData.h"
#include "DKFoundation/DKStream.h"
#include "DKFoundation/DKDataStream.h"
#include "DKFoundation/DKBuffer.h"
#include "DKFoundation/DKBufferStream.h"

// file, file-map, and directory
#include "DKFoundation/DKFile.h"
#include "DKFoundation/DKFileMap.h"
#include "DKFoundation/DKDirectory.h"

// compressor, archiver
#include "DKFoundation/DKCompressor.h"
#include "DKFoundation/DKZipArchiver.h"
#include "DKFoundation/DKZipUnarchiver.h"

// XML
#include "DKFoundation/DKXmlParser.h"
#include "DKFoundation/DKXmlDocument.h"

// date time, timer
#include "DKFoundation/DKTimer.h"
#include "DKFoundation/DKDateTime.h"

// math types
#include "DKFoundation/DKFloat16.h"
#include "DKFoundation/DKRational.h"

// operation, invocation (function utilities)
#include "DKFoundation/DKFunction.h"
#include "DKFoundation/DKInvocation.h"
#include "DKFoundation/DKOperation.h"
#include "DKFoundation/DKValue.h"

// event-loop, event-loop timer, operation queue
#include "DKFoundation/DKEventLoop.h"
#include "DKFoundation/DKEventLoopTimer.h"
#include "DKFoundation/DKOperationQueue.h"

// etc
#include "DKFoundation/DKEndianness.h"
#include "DKFoundation/DKError.h"
#include "DKFoundation/DKLog.h"
#include "DKFoundation/DKLogger.h"
#include "DKFoundation/DKUtils.h"
