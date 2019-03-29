//
//  File: DKError.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKString.h"
#include "DKFunction.h"
#include "DKStream.h"

#define DKERROR_DEFAULT_CALLSTACK_TRACE_DEPTH	1024

namespace DKFoundation
{
	typedef DKFunctionSignature<void (class DKError&)> DKCriticalErrorHandler;

	/// test debugger attached or not. (may not works on some platforms)
	DKGL_API bool DKIsDebuggerPresent();

	/// test build configuration
	DKGL_API bool DKIsDebugBuild();

	/// set critical error handler. (may not works on some platforms)
	DKGL_API void DKSetCriticalErrorHandler(DKCriticalErrorHandler*);

	/// raise exception manually. use DKERROR_THROW() macro instead.
	DKGL_API void DKErrorRaiseException(const char*, const char*, unsigned int, const char*);

	namespace Private { struct UnexpectedError; }

	/**
	 @brief an exception object.
	 you can review call-stack info with this object.
	 you can write error description to file also.


	 Usage:
	 @code
		try
		{
		  .. exception occurred in somewhere ..
		}
		catch (DKError& e)
		{
		  // print call stack
			e.PrintDescriptionWithStackFrames();
		  // descript error info with default descriptor (may be console)
			e.WriteToDefaultDescriptor();
		  // write error info into myLogFile
			e.WriteToFile(myLogFile);

		  .. try to recover or do something useful.

		}
	 @endcode

	 @note
	  On some platforms, the trace call stack may not work.
	 */
	class DKGL_API DKError
	{
	public:
		constexpr static int DefaultCallstackTraceDepth() {return 1024;} ///< call stack trace depth

		struct StackFrame
		{
			DKString	module;				///< module file
			void*		baseAddress;		///< module base address
			DKString	function;			///< function name
			void*		address;			///< frame address
			size_t		offset;				///< function offset
			DKString	filename;			///< source-file
			size_t		line;				///< source-file line
		};
		typedef DKFunctionSignature<void (const void*, size_t)> Descriptor;
		typedef DKFunctionSignature<void (const DKString&)> StringOutput;

		DKError();
		DKError(const DKString& desc);
		DKError(int errorCode, const DKString& desc);
		DKError(DKError&&);
		DKError(const DKError&);
		~DKError();

		DKError& operator = (DKError&&);
		DKError& operator = (const DKError&);

		/// Raises an exception with the specified description.
		static void RaiseException(const DKString& func, const DKString& file, unsigned int line, const DKString& desc);
		/// Raises an exception with the error object.
		static void RaiseException(const DKError& e);
		/// Raises an exception with the specified description.
		static void RaiseException(int errorCode, const DKString& desc);
		/// Raises an exception with the specified description.
		static void RaiseException(const DKString& desc);

		int Code() const;
		const DKString& Function() const;
		const DKString& File() const;
		int Line() const;
		const DKString& Description() const;

		size_t NumberOfStackFrames() const;
		const StackFrame* StackFrameAtIndex(unsigned int index) const;
		size_t CopyStackFrames(StackFrame* s, size_t maxCount) const;

		/// retrace call stack frame info.
		size_t RetraceStackFrames(int skip = 0, int maxDepth = DKERROR_DEFAULT_CALLSTACK_TRACE_DEPTH);

		/// writing error info
		void PrintDescription() const;
		void PrintDescription(const StringOutput*) const;
		void PrintStackFrames() const;
		void PrintStackFrames(const StringOutput*) const;
		void PrintDescriptionWithStackFrames() const;
		void PrintDescriptionWithStackFrames(const StringOutput*) const;

		/// writing error info into file or stream
		void WriteToDescriptor(const Descriptor*) const;
		void WriteToDefaultDescriptor() const;
		bool WriteToFile(const DKString& file) const;
		bool WriteToStream(DKStream* stream) const;

		/// set default descriptor.
		/// descriptor is function or function object. (lambda is ok)
		/// see 'Descriptor' typedef above.
		static void SetDefaultDescriptor(const Descriptor* output);

	private:
		int errorCode;
		DKString functionName;
		DKString fileName;
		uint32_t lineNo;
		DKString description;

		// callstack
		size_t numFrames;
		StackFrame* stackFrames;
		uintptr_t threadId;
		friend struct Private::UnexpectedError;
		static void DumpUnexpectedError(Private::UnexpectedError*);
	};
}
