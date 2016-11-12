//
//  File: DKError.cpp
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//


#if	defined(__APPLE__) && defined(__MACH__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <signal.h>
#endif

#if defined(__linux__)
// linux not supported yet.
#endif

#if defined(_WIN32)
#include <Windows.h>
#include <DbgHelp.h>
#endif

#include <stdlib.h>
#include "DKError.h"
#include "DKArray.h"
#include "DKMap.h"
#include "DKLog.h"
#include "DKSpinLock.h"
#include "DKCriticalSection.h"
#include "DKThread.h"
#include "DKFile.h"
#include "DKString.h"

#ifndef DKERROR_HANDLE_CRITICAL_ERROR

	#if defined(_WIN32)
		#define DKERROR_HANDLE_CRITICAL_ERROR 1
	#elif defined(__APPLE__) && defined(__MACH__)
		#define DKERROR_HANDLE_CRITICAL_ERROR 1
	#else
		#define DKERROR_HANDLE_CRITICAL_ERROR 0
	#endif
#endif

namespace DKFoundation
{
	namespace Private
	{
		struct UnexpectedError
		{
			DKArray<DKError::StackFrame> callstack;
			DKString description;
			unsigned int code;
			void Dump(void)
			{
				DKError::DumpUnexpectedError(this);
			}
		};
		namespace
		{
			DKSpinLock traceLock;
			DKSpinLock fileLock;
			DKObject<DKError::Descriptor> defaultDescriptor = NULL;

#if DKERROR_HANDLE_CRITICAL_ERROR
			DKSpinLock critFuncLock;
			DKObject<DKCriticalErrorHandler> criticalErrorFunc = NULL;
#endif
			DKObject<DKCriticalErrorHandler> GetCriticalErrorHandler(void)
			{
#if DKERROR_HANDLE_CRITICAL_ERROR
				DKCriticalSection<DKSpinLock> guard(critFuncLock);
				return criticalErrorFunc;
#endif
				return NULL;
			}

#ifdef _WIN32
			struct DbgHelpDLL
			{
				HMODULE hDLL;
				// SymInitializeW
				typedef BOOL (__stdcall *PFSymInitializeW)(HANDLE, PCWSTR, BOOL);
				PFSymInitializeW pSymInitializeW;
				// SymCleanup
				typedef BOOL (__stdcall *PFSymCleanup)(HANDLE);
				PFSymCleanup pSymCleanup;
				// SymFunctionTableAccess64
				typedef PVOID (__stdcall *PFSymFunctionTableAccess64)(HANDLE, DWORD64);
				PFSymFunctionTableAccess64 pSymFunctionTableAccess64;
				// SymGetLineFromAddrW64
				typedef BOOL (__stdcall *PFSymGetLineFromAddrW64)(HANDLE, DWORD64, PDWORD, PIMAGEHLP_LINEW64);
				PFSymGetLineFromAddrW64 pSymGetLineFromAddrW64;
				// SymGetModuleBase64
				typedef DWORD64 (__stdcall *PFSymGetModuleBase64)(HANDLE, DWORD64);
				PFSymGetModuleBase64 pSymGetModuleBase64;
				// SymGetModuleInfoW64
				typedef BOOL (__stdcall *PFSymGetModuleInfoW64)(HANDLE, DWORD64, PIMAGEHLP_MODULEW64);
				PFSymGetModuleInfoW64 pSymGetModuleInfoW64;
				// SymGetOptions
				typedef DWORD (__stdcall *PFSymGetOptions)(VOID);
				PFSymGetOptions pSymGetOptions;
				// SymSetOptions
				typedef DWORD (__stdcall *PFSymSetOptions)(DWORD);
				PFSymSetOptions pSymSetOptions;
				// SymFromAddrW
				typedef BOOL (__stdcall *PFSymFromAddrW)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFOW);
				PFSymFromAddrW pSymFromAddrW;
				// SymLoadModule64
				typedef DWORD64 (__stdcall *PFSymLoadModule64)(HANDLE, HANDLE, PSTR, PSTR, DWORD64, DWORD);
				PFSymLoadModule64 pSymLoadModule64;
				// StackWalk64
				typedef BOOL (__stdcall *PFStackWalk64)(DWORD, HANDLE, HANDLE, LPSTACKFRAME64, PVOID, PREAD_PROCESS_MEMORY_ROUTINE64, PFUNCTION_TABLE_ACCESS_ROUTINE64, PGET_MODULE_BASE_ROUTINE64, PTRANSLATE_ADDRESS_ROUTINE64);
				PFStackWalk64 pStackWalk64;
				// UnDecorateSymbolNameW
				typedef DWORD (__stdcall *PFUnDecorateSymbolNameW)(PCWSTR, PWSTR, DWORD, DWORD);
				PFUnDecorateSymbolNameW pUnDecorateSymbolNameW;
				// SymGetSearchPathW
				typedef BOOL (__stdcall *PFSymGetSearchPathW)(HANDLE, PWSTR, DWORD);
				PFSymGetSearchPathW pSymGetSearchPathW;
			};

			inline DKString GetErrorString(DWORD dwError)
			{
				DKString ret = L"";
				// error!
				LPVOID lpMsgBuf;
				::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
								 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &lpMsgBuf, 0, NULL );

				ret = (const wchar_t*)lpMsgBuf;
				::LocalFree(lpMsgBuf);
				return ret;
			}
			inline DKString GetExceptionString(DWORD ec)
			{
				DKString ret = L"";
				switch (ec)
				{
					case EXCEPTION_ACCESS_VIOLATION:
						ret = L"[EXCEPTION_ACCESS_VIOLATION] The thread attempted to read from or write to a virtual address that it does not have appropriate access to.";
						break;
					case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
						ret = L"[EXCEPTION_ARRAY_BOUNDS_EXCEEDED] The thread attempted to access an array element that is out of bounds, and the underlying hardware supports bounds checking.";
						break;
					case EXCEPTION_BREAKPOINT:
						ret = L"[EXCEPTION_BREAKPOINT] A breakpoint was encountered.";
						break;
					case EXCEPTION_DATATYPE_MISALIGNMENT:
						ret = L"[EXCEPTION_DATATYPE_MISALIGNMENT] The thread attempted to read or write data that is misaligned on hardware that does not provide alignment.";
						break;
					case EXCEPTION_FLT_DENORMAL_OPERAND:
						ret = L"[EXCEPTION_FLT_DENORMAL_OPERAND] An operand in a floating-point operation is too small to represent as a standard floating-point value.";
						break;
					case EXCEPTION_FLT_DIVIDE_BY_ZERO:
						ret = L"[EXCEPTION_FLT_DIVIDE_BY_ZERO] The thread attempted to divide a floating-point value by a floating-point divisor of zero.";
						break;
					case EXCEPTION_FLT_INEXACT_RESULT:
						ret = L"[EXCEPTION_FLT_INEXACT_RESULT] The result of a floating-point operation cannot be represented exactly as a decimal fraction.";
						break;
					case EXCEPTION_FLT_INVALID_OPERATION:
						ret = L"[EXCEPTION_FLT_INVALID_OPERATION] This exception represents a floating-point exception not included in this list.";
						break;
					case EXCEPTION_FLT_OVERFLOW:
						ret = L"[EXCEPTION_FLT_OVERFLOW] The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.";
						break;
					case EXCEPTION_FLT_STACK_CHECK:
						ret = L"[EXCEPTION_FLT_STACK_CHECK] The stack overflowed or underflowed as a result of a floating-point operation.";
						break;
					case EXCEPTION_FLT_UNDERFLOW:
						ret = L"[EXCEPTION_FLT_UNDERFLOW] The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.";
						break;
					case EXCEPTION_GUARD_PAGE:
						ret = L"[EXCEPTION_GUARD_PAGE] The thread accessed memory allocated with the PAGE_GUARD modifier.";
						break;
					case EXCEPTION_ILLEGAL_INSTRUCTION:
						ret = L"[EXCEPTION_ILLEGAL_INSTRUCTION] The thread tries to execute an invalid instruction.";
						break;
					case EXCEPTION_IN_PAGE_ERROR:
						ret = L"[EXCEPTION_IN_PAGE_ERROR] The thread tries to access a page that is not present, and the system is unable to load the page.";
						break;
					case EXCEPTION_INT_DIVIDE_BY_ZERO:
						ret = L"[EXCEPTION_INT_DIVIDE_BY_ZERO] The thread attempted to divide an integer value by an integer divisor of zero.";
						break;
					case EXCEPTION_INT_OVERFLOW:
						ret = L"[EXCEPTION_INT_OVERFLOW] The result of an integer operation caused a carry out of the most significant bit of the result.";
						break;
					case EXCEPTION_INVALID_DISPOSITION:
						ret = L"[EXCEPTION_INVALID_DISPOSITION] An exception handler returns an invalid disposition to the exception dispatcher.";
						break;
					case EXCEPTION_INVALID_HANDLE:
						ret = L"[EXCEPTION_INVALID_HANDLE] The thread used a handle to a kernel object that was invalid.";
						break;
					case EXCEPTION_NONCONTINUABLE_EXCEPTION:
						ret = L"[EXCEPTION_NONCONTINUABLE_EXCEPTION] The thread attempted to continue execution after a noncontinuable exception occurred.";
						break;
					case EXCEPTION_PRIV_INSTRUCTION:
						ret = L"[EXCEPTION_PRIV_INSTRUCTION] The thread attempted to execute an instruction whose operation is not allowed in the current machine mode.";
						break;
					case EXCEPTION_SINGLE_STEP:
						ret = L"[EXCEPTION_SINGLE_STEP] A trace trap or other single-instruction mechanism signaled that one instruction has been run.";
						break;
					case EXCEPTION_STACK_OVERFLOW:
						ret = L"[EXCEPTION_STACK_OVERFLOW] The thread uses up its stack.";
						break;
					case STATUS_UNWIND_CONSOLIDATE:
						ret = L"[STATUS_UNWIND_CONSOLIDATE] A frame consolidation has been executed.";
						break;
					default:
						ret = DKString::Format("ExceptionCode:%x is unknown.", ec);
						break;
				}
				return ret;
			}

			DbgHelpDLL* GetDbgHelpDLL(void)
			{
				static bool init = false;
				static DbgHelpDLL dbg;

				if (init == false)
				{
					memset(&dbg, 0, sizeof(dbg));
					dbg.hDLL = ::LoadLibraryW(L"DbgHelp.dll");
					if (dbg.hDLL)
					{
						struct ProcAddress
						{
							const char* name;
							FARPROC* pfn;
						} pa[13] =
						{
							{"SymInitializeW",				(FARPROC*)&dbg.pSymInitializeW},
							{"SymCleanup",					(FARPROC*)&dbg.pSymCleanup},
							{"StackWalk64",					(FARPROC*)&dbg.pStackWalk64},
							{"SymGetOptions",				(FARPROC*)&dbg.pSymGetOptions},
							{"SymSetOptions",				(FARPROC*)&dbg.pSymSetOptions},
							{"SymFunctionTableAccess64",	(FARPROC*)&dbg.pSymFunctionTableAccess64},
							{"SymGetLineFromAddrW64",		(FARPROC*)&dbg.pSymGetLineFromAddrW64},
							{"SymGetModuleBase64",			(FARPROC*)&dbg.pSymGetModuleBase64},
							{"SymGetModuleInfoW64",			(FARPROC*)&dbg.pSymGetModuleInfoW64},
							{"SymFromAddrW",				(FARPROC*)&dbg.pSymFromAddrW},
							{"UnDecorateSymbolNameW",		(FARPROC*)&dbg.pUnDecorateSymbolNameW},
							{"SymLoadModule64",				(FARPROC*)&dbg.pSymLoadModule64},
							{"SymGetSearchPathW",			(FARPROC*)&dbg.pSymGetSearchPathW},
						};
						bool failed = false;
						for (int i = 0; i < 13; i++)
						{
							*pa[i].pfn = ::GetProcAddress(dbg.hDLL, pa[i].name);
							if (pa[i].pfn == NULL)
							{
								failed = true;
								DWORD dwError = ::GetLastError();
								if (dwError)
									DKLog("GetProcAddress failed with error %d: %ls", dwError, (const wchar_t*)GetErrorString(dwError));
								else
									DKLog("GetProcAddress failed with unknown error.\n");
								break;
							}
						}

						if (failed)
						{
							::FreeLibrary(dbg.hDLL);
							DKLog("Failed to load library: DbgHelp.dll\n");
						}
						else
						{
							DWORD opt = dbg.pSymGetOptions();
							opt |= SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_DEBUG;
							dbg.pSymSetOptions(opt);

							init = true;
						}
					}
					else
					{
						DWORD dwError = ::GetLastError();
						if (dwError)
						{
							DKLog("LoadLibrary failed with error %d: %ls", dwError, (const wchar_t*)GetErrorString(dwError));
						}
					}
				}
				if (init)
					return &dbg;
				return NULL;
			}

			void TraceCallStack(const CONTEXT *pCtx, size_t skip, size_t maxDepth, DKArray<DKError::StackFrame>& frames)
			{
				DKCriticalSection<DKSpinLock> guard(traceLock);

				DbgHelpDLL* dbg = GetDbgHelpDLL();
				if (dbg)
				{
					HANDLE hProcess = ::GetCurrentProcess();
					if (dbg->pSymInitializeW(hProcess, NULL, TRUE))
					{
						CONTEXT ctx = *pCtx;

						STACKFRAME64 frm;
						::memset(&frm, 0, sizeof(frm));
						DWORD imageType;

#ifdef _M_IX86
						imageType = IMAGE_FILE_MACHINE_I386;
						frm.AddrPC.Offset = ctx.Eip;
						frm.AddrPC.Mode = AddrModeFlat;
						frm.AddrFrame.Offset = ctx.Ebp;
						frm.AddrFrame.Mode = AddrModeFlat;
						frm.AddrStack.Offset = ctx.Esp;
						frm.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
						imageType = IMAGE_FILE_MACHINE_AMD64;
						frm.AddrPC.Offset = ctx.Rip;
						frm.AddrPC.Mode = AddrModeFlat;
						frm.AddrFrame.Offset = ctx.Rsp;
						frm.AddrFrame.Mode = AddrModeFlat;
						frm.AddrStack.Offset = ctx.Rsp;
						frm.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
						imageType = IMAGE_FILE_MACHINE_IA64;
						frm.AddrPC.Offset = ctx.StIIP;
						frm.AddrPC.Mode = AddrModeFlat;
						frm.AddrFrame.Offset = ctx.IntSp;
						frm.AddrFrame.Mode = AddrModeFlat;
						frm.AddrBStore.Offset = ctx.RsBSP;
						frm.AddrBStore.Mode = AddrModeFlat;
						frm.AddrStack.Offset = ctx.IntSp;
						frm.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

						HANDLE hThread = ::GetCurrentThread();
						BYTE symbolBuffer[sizeof(SYMBOL_INFOW) + sizeof(wchar_t[MAX_SYM_NAME])];

						for (size_t level = 0; level < maxDepth; level++)
						{
							if (dbg->pStackWalk64(imageType, hProcess, hThread, &frm, &ctx, NULL, dbg->pSymFunctionTableAccess64, dbg->pSymGetModuleBase64, NULL))
							{
								if ( frm.AddrPC.Offset != 0 )
								{
									if (level >= skip)
									{
										DKError::StackFrame frame;
										frame.module = L"";
										frame.baseAddress = 0;
										frame.function = L"";
										frame.offset = 0;
										frame.filename = L"";
										frame.line = 0;
										frame.address = reinterpret_cast<void*>(frm.AddrFrame.Offset);

										::memset(symbolBuffer, 0, sizeof(symbolBuffer));

										PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)symbolBuffer;
										pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
										pSymbol->MaxNameLen = MAX_SYM_NAME;

										// retrieve function name.
										DWORD64 symDisplacement = 0;
										if (dbg->pSymFromAddrW(hProcess, frm.AddrPC.Offset, &symDisplacement, pSymbol))
										{
											frame.function = pSymbol->Name;
											frame.offset = symDisplacement;
										}
										else
										{
											DWORD dwError = ::GetLastError();
											if (dwError)
												DKLog("SymFromAddr failed with error %d: %ls", dwError, (const wchar_t*)GetErrorString(dwError));
											else
												DKLog("SymFromAddr failed with unknown error!\n");
										}

										// retrieve source-file, line number.
										IMAGEHLP_LINEW64 lineInfo = { sizeof(IMAGEHLP_LINEW64) };
										DWORD lineDisplacement = 0;
										if (dbg->pSymGetLineFromAddrW64(hProcess, frm.AddrPC.Offset, &lineDisplacement, &lineInfo))
										{
											frame.filename = lineInfo.FileName;
											frame.line = lineInfo.LineNumber;
										}
										else	// no source-info
										{
										}

										// retrieve module info.
										IMAGEHLP_MODULEW64 moduleInfo = {sizeof(IMAGEHLP_MODULEW64)};
										if (dbg->pSymGetModuleInfoW64(hProcess, frm.AddrPC.Offset, &moduleInfo))
										{
											frame.module = moduleInfo.ImageName;
											frame.baseAddress = reinterpret_cast<void*>(moduleInfo.BaseOfImage);
										}
										else
										{
										}

										frames.Add(frame);
									}
								}

								if (frm.AddrReturn.Offset == 0)
									break;
							}
							else
							{
								DWORD dwError = ::GetLastError();
								if (dwError)
									DKLog("StackWalk64 failed with error %d: %ls", dwError, (const wchar_t*)GetErrorString(dwError));
								else
									DKLog("StackWalk64 failed with unknown error!\n");

								break;
							}
						}

						dbg->pSymCleanup(hProcess);
					}
					else
					{
						DWORD dwError = ::GetLastError();
						if (dwError)
							DKLog("SymInitialize failed with error %d: %ls", dwError, (const wchar_t*)GetErrorString(dwError));
						else
							DKLog("SymInitialize failed with unknown error!\n");
					}
				}
				else
				{
					DKLog("Failed to load library DbgHelp.dll\n");
				}
			}

			void TraceCallStack(const _EXCEPTION_POINTERS* ep, size_t skip, size_t maxDepth, DKArray<DKError::StackFrame>& frames)
			{
				TraceCallStack(ep->ContextRecord, skip, maxDepth, frames);
			}

			void TraceCallStack(size_t skip, size_t maxDepth, DKArray<DKError::StackFrame>& frames)
			{
				// using try-except.
				// RtlCaptureContext for i386 and x64 are different.
				__try
				{
					::RaiseException(0, 0, 0, 0);
				}
				__except( TraceCallStack((EXCEPTION_POINTERS*)GetExceptionInformation(), skip+1, maxDepth, frames), EXCEPTION_CONTINUE_EXECUTION )
				{
				}
				//CONTEXT ctxt;
				//memset(&ctxt, 0, sizeof(CONTEXT));
				//ctxt.ContextFlags = CONTEXT_FULL;
				//RtlCaptureContext(&ctxt);
				//Private::TraceCallStack(&ctxt, skip, maxDepth, frames);
			}

#if DKERROR_HANDLE_CRITICAL_ERROR
			void DumpUnexpectedError(DWORD code, const _EXCEPTION_POINTERS* ep, size_t maxDepth)
			{
				UnexpectedError ue;
				ue.code = code;
				ue.description = GetExceptionString(code);
				TraceCallStack(ep, 0, maxDepth, ue.callstack);
				ue.Dump();
			}
#endif
#else	//define _WIN32
#if	defined(__APPLE__) && defined(__MACH__)
			void __attribute__((noinline)) TraceCallStack(int skip, int maxDepth, DKArray<DKError::StackFrame>& frames)
			{
				DKCriticalSection<DKSpinLock> guard(traceLock);

				// generate callstack
				void** callstack = (void**)malloc(sizeof(void*) * maxDepth);
				int numFrames = backtrace(callstack, maxDepth);

				for (int i = skip; i < numFrames; ++i)
				{
					Dl_info info;
					memset(&info, 0, sizeof(Dl_info));

					void* addr = callstack[i];
					dladdr(addr, &info);

					DKError::StackFrame frame;
					frame.module = (info.dli_fname) ? DKString(info.dli_fname) : DKString::empty;
					frame.baseAddress = info.dli_fbase;

					if (info.dli_sname)
					{
						int status;
						char* functionName = abi::__cxa_demangle(info.dli_sname, 0, 0, &status);
						if (status == 0)	// successful demangling
						frame.function.SetValue(functionName);
						else
						frame.function.SetValue(info.dli_sname);
						if (functionName)
						free(functionName);
					}
					else
					{
						frame.function = L"";
					}

					frame.address = info.dli_saddr;		// symbol address.
					frame.offset = reinterpret_cast<unsigned char*>(addr) - reinterpret_cast<unsigned char*>(info.dli_saddr);
					frame.filename = L"";
					frame.line = 0;

					frames.Add(frame);
				}
				free(callstack);
			}
#elif defined(__linux__)
			void __attribute__((noinline)) TraceCallStack(int skip, int maxDepth, DKArray<DKError::StackFrame>& frames)
			{
				// not implemented yet.
			}
#endif
#if DKERROR_HANDLE_CRITICAL_ERROR
			void DumpUnexpectedError(int sig, struct __siginfo* info, void* uap)
			{
				UnexpectedError ue;
				ue.code = info->si_code;
				ue.description = strsignal(sig);
				TraceCallStack(2, 1024, ue.callstack);
				ue.Dump();
				signal(sig, SIG_DFL);
			}
#endif
#endif	//define _WIN32
		}

		void PerformOperationWithErrorHandler(const DKOperation* p, size_t maxDepth)
		{
#if DKERROR_HANDLE_CRITICAL_ERROR
#ifdef _WIN32
			__try {p->Perform();}
			__except( DumpUnexpectedError(GetExceptionCode(), (EXCEPTION_POINTERS*)GetExceptionInformation(), maxDepth), EXCEPTION_CONTINUE_SEARCH ){}
#else
			static DKSpinLock lock;
			static bool handlerInstalled = false;
			if (!handlerInstalled)
			{
				DKCriticalSection<DKSpinLock> guard(lock);
				if (!handlerInstalled)
				{
					/* Install signal handler */
					struct sigaction sa;
					sa.sa_sigaction = &DumpUnexpectedError;
					// blocking other signals, while handling signal.
					sigfillset(&sa.sa_mask);
					sa.sa_flags = SA_RESTART | SA_SIGINFO;

					const int sig[6] = {SIGSEGV, SIGBUS, SIGSYS, SIGILL, SIGFPE, SIGABRT};
					for (size_t i = 0; i < 6; ++i)
						sigaction(sig[i], &sa, NULL);

					handlerInstalled = true;
				}
			}
			p->Perform();
#endif
#else
			p->Perform();
#endif
		}
	}	// namespace Private

	bool DKGL_API DKIsDebuggerPresent(void)
	{
#ifdef _WIN32
		return ::IsDebuggerPresent() != FALSE;
#elif defined(__APPLE__) && defined(__MACH__)
		int mib[4];
		struct kinfo_proc info;
		size_t size;

		info.kp_proc.p_flag = 0;
		mib[0] = CTL_KERN;
		mib[1] = KERN_PROC;
		mib[2] = KERN_PROC_PID;
		mib[3] = getpid();

		size = sizeof(info);
		sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

		return ((info.kp_proc.p_flag & P_TRACED) != 0);
#else
		return false;
#endif
	}
	bool DKGL_API DKIsDebugBuild(void)
	{
#ifdef DKGL_DEBUG_ENABLED
		return true;
#else
		return false;
#endif
	}

	void DKGL_API DKSetCriticalErrorHandler(DKCriticalErrorHandler* h)
	{
#if DKERROR_HANDLE_CRITICAL_ERROR
		DKCriticalSection<DKSpinLock> guard(Private::critFuncLock);
		Private::criticalErrorFunc = h;
#endif
	}

	void DKGL_API DKErrorRaiseException(const char* fn, const char* file, unsigned int ln, const char* desc)
	{
		DKError::RaiseException(fn, file, ln, desc);
	}
}

using namespace DKFoundation;

DKError::DKError(void)
	: errorCode(0)
	, functionName(L"")
	, fileName(L"")
	, lineNo(0)
	, description(L"")
	, numFrames(0)
	, stackFrames(NULL)
	, threadId(DKThread::CurrentThreadId())
{
}

DKError::DKError(const DKString& desc)
	: errorCode(0)
	, functionName(L"")
	, fileName(L"")
	, lineNo(0)
	, description(desc)
	, numFrames(0)
	, stackFrames(NULL)
	, threadId(DKThread::CurrentThreadId())
{
}

DKError::DKError(int ec, const DKString& desc)
	: errorCode(ec)
	, functionName(L"")
	, fileName(L"")
	, lineNo(0)
	, description(desc)
	, numFrames(0)
	, stackFrames(NULL)
	, threadId(DKThread::CurrentThreadId())
{
}

DKError::DKError(DKError&& e)
	: errorCode(0)
	, functionName(L"")
	, fileName(L"")
	, lineNo(0)
	, description(L"")
	, numFrames(0)
	, stackFrames(NULL)
	, threadId(DKThread::CurrentThreadId())
{
	errorCode = e.errorCode;
	functionName = static_cast<DKString&&>(e.functionName);
	fileName = static_cast<DKString&&>(e.fileName);
	lineNo = e.lineNo;
	description = static_cast<DKString&&>(e.description);
	numFrames = e.numFrames;
	stackFrames = e.stackFrames;
	threadId = e.threadId;

	e.errorCode = 0;
	e.functionName = L"";
	e.fileName = L"";
	e.lineNo = 0;
	e.description = L"";
	e.numFrames = 0;
	e.stackFrames = NULL;
}

DKError::DKError(const DKError& e)
	: errorCode(e.errorCode)
	, functionName(e.functionName)
	, fileName(e.fileName)
	, lineNo(e.lineNo)
	, description(e.description)
	, numFrames(0)
	, stackFrames(NULL)
	, threadId(e.threadId)
{
	if (e.numFrames > 0)
	{
		this->numFrames = e.numFrames;
		this->stackFrames = new StackFrame[this->numFrames];
		for (size_t i = 0; i < this->numFrames; ++i)
			this->stackFrames[i] = e.stackFrames[i];
	}
}

DKError::~DKError(void)
{
	if (stackFrames)
		delete[] stackFrames;
}

DKError& DKError::operator = (DKError&& e)
{
	if (this != &e)
	{
		errorCode = e.errorCode;
		functionName = static_cast<DKString&&>(e.functionName);
		fileName = static_cast<DKString&&>(e.fileName);
		lineNo = e.lineNo;
		description = static_cast<DKString&&>(e.description);
		numFrames = e.numFrames;
		stackFrames = e.stackFrames;
		threadId = e.threadId;

		e.errorCode = 0;
		e.functionName = L"";
		e.fileName = L"";
		e.lineNo = 0;
		e.description = L"";
		e.numFrames = 0;
		e.stackFrames = NULL;
	}
	return *this;
}

DKError& DKError::operator = (const DKError& e)
{
	if (this != &e)
	{
		this->errorCode = e.errorCode;
		this->functionName = e.functionName;
		this->fileName = e.fileName;
		this->lineNo = e.lineNo;
		this->description = e.description;
		this->threadId = e.threadId;

		if (this->stackFrames)
			delete[] stackFrames;

		this->numFrames = 0;
		this->stackFrames = NULL;

		// copy stack info.
		if (e.numFrames > 0)
		{
			this->numFrames = e.numFrames;
			this->stackFrames = new StackFrame[this->numFrames];
			for (size_t i = 0; i < this->numFrames; ++i)
				this->stackFrames[i] = e.stackFrames[i];
		}
	}
	return *this;
}

int DKError::Code(void) const
{
	return errorCode;
}

const DKString& DKError::Function(void) const
{
	return functionName;
}

const DKString& DKError::File(void) const
{
	return fileName;
}

int DKError::Line(void) const
{
	return lineNo;
}

const DKString& DKError::Description(void) const
{
	return description;
}

size_t DKError::NumberOfStackFrames(void) const
{
	return numFrames;
}

const DKError::StackFrame* DKError::StackFrameAtIndex(unsigned int index) const
{
	if (numFrames > index)
	{
		return &stackFrames[index];
	}
	return NULL;
}

size_t DKError::CopyStackFrames(StackFrame* s, size_t maxCount) const
{
	if (s)
	{
		size_t c = Min(maxCount, numFrames);
		try
		{
			for (size_t i = 0; i < c; ++i)
			{
				s[i] = stackFrames[i];
			}
		}
		catch (...)
		{
			DKLog("%s CRITICAL ERROR!\n", DKGL_FUNCTION_NAME);
			return 0;
		}
		return c;
	}
	return 0;
}

size_t DKError::RetraceStackFrames(int skip, int maxDepth)
{
	if (stackFrames)
		delete[] stackFrames;

	numFrames = 0;
	stackFrames = NULL;
	threadId = DKThread::CurrentThreadId();

	maxDepth = Clamp(maxDepth, 0, 1024);
	if (maxDepth == 0 || skip > maxDepth)
		return 0;
	skip = Max<int>(0, skip);

	// generate call stack
	DKArray<StackFrame> sf;
	Private::TraceCallStack(skip+1, maxDepth+1, sf);

	numFrames = sf.Count();
	if (numFrames > 0)
	{
		stackFrames = new StackFrame[numFrames];
		StackFrame* frames = (StackFrame*)sf;
		try
		{
			for (size_t i = 0; i < numFrames; ++i)
			{
				stackFrames[i] = frames[i];
			}
		}
		catch (...)
		{
			numFrames = 0;
			if (stackFrames)
				delete[] stackFrames;
			DKLog("[%s] CRITICAL-ERROR: unknown error occurred while copying frame data.\n", DKGL_FUNCTION_NAME);
		}
	}
	return numFrames;
}

void DKError::RaiseException(const DKString& func, const DKString& file, unsigned int line, const DKString& desc)
{
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);

	DKError err(desc);
	err.functionName = func;
	err.fileName = file;
	err.lineNo = line;
	err.RetraceStackFrames(1, 1024);

#ifdef DKGL_DEBUG_ENABLED
	err.PrintDescriptionWithStackFrames();
#else
	if (IsDebuggerPresent())
		err.PrintDescriptionWithStackFrames();
#endif

	throw err;
}

void DKError::RaiseException(int errorCode, const DKString& desc)
{
	DKLog("%s]\n", DKGL_FUNCTION_NAME);

	DKError err(errorCode, desc);
	err.RetraceStackFrames(1, 1024);

#ifdef DKGL_DEBUG_ENABLED
	err.PrintDescriptionWithStackFrames();
#else
	if (IsDebuggerPresent())
		err.PrintDescriptionWithStackFrames();
#endif

	throw err;
}

void DKError::RaiseException(const DKString& desc)
{
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);

	DKError err(desc);
	err.RetraceStackFrames(1, 1024);

#ifdef DKGL_DEBUG_ENABLED
	err.PrintDescriptionWithStackFrames();
#else
	if (IsDebuggerPresent())
		err.PrintDescriptionWithStackFrames();
#endif

	throw err;
}

void DKError::RaiseException(const DKError& e)
{
	DKLog("[%s]\n", DKGL_FUNCTION_NAME);

	DKError err(e);
	err.RetraceStackFrames(1, 1024);

#ifdef DKGL_DEBUG_ENABLED
	err.PrintDescriptionWithStackFrames();
#else
	if (IsDebuggerPresent())
		err.PrintDescriptionWithStackFrames();
#endif

	throw err;
}

void DKError::PrintDescription(void) const
{
	PrintDescription(DKFunction((void (*)(const DKString&))&DKLog));
}

void DKError::PrintDescription(const StringOutput* pfn) const
{
	if (pfn == NULL)
		return;

	pfn->Invoke(DKString::Format("DKError(%p) Printing Description.\n", this));
	pfn->Invoke(DKString::Format("Debug Build: %s\n", DKIsDebugBuild() ? "yes" : "no"));
	pfn->Invoke(DKString::Format("Debugger Present: %s\n", DKIsDebuggerPresent() ? "yes" : "no"));
	pfn->Invoke(DKString::Format("Thread-Id: %lu\n", threadId));
	pfn->Invoke(DKString::Format("Error-Code:%d(0x%x)\n", errorCode, errorCode));
	if (functionName.Length() > 0)
		pfn->Invoke(DKString::Format("Function: %ls\n", (const wchar_t*)functionName));
	if (fileName.Length() > 0)
		pfn->Invoke(DKString::Format("File: %ls (%u)\n", (const wchar_t*)fileName, lineNo));
	if (description.Length() > 0)
		pfn->Invoke(DKString::Format("Error description: %ls\n", (const wchar_t*)description));
}

void DKError::PrintStackFrames(void) const
{
	PrintStackFrames(DKFunction((void (*)(const DKString&))&DKLog));
}

void DKError::PrintStackFrames(const StringOutput* pfn) const
{
	if (pfn == NULL)
		return;

	if (numFrames > 0)
	{
		pfn->Invoke(DKString::Format("Call Stack Information. (%u frames)\n", numFrames));

		void* baseAddress = 0;
		DKString module = L"";
		for (size_t i = 0; i < numFrames; ++i)
		{
			const StackFrame& frame = stackFrames[i];
			if (baseAddress != frame.baseAddress || module.CompareNoCase(frame.module))
			{
				baseAddress = frame.baseAddress;
				module = frame.module;
				if (module.Length() > 0)
					pfn->Invoke(DKString::Format("Module: %ls (base address:%p)\n", (const wchar_t*)module, frame.baseAddress));
				else
					pfn->Invoke(L" <Module information not available>\n");
			}
			if (frame.function.Length() > 0)
				pfn->Invoke(DKString::Format("%u: %ls (%p + %u)\n", i, (const wchar_t*)frame.function, frame.address, frame.offset));
			else
				pfn->Invoke(DKString::Format("%u: <function name not available> (%p + %u)\n", i, frame.address, frame.offset));
			if (frame.filename.Length() > 0)
				pfn->Invoke(DKString::Format(" %ls (%u)\n", (const wchar_t*)frame.filename, frame.line));
			else
				pfn->Invoke(L" <source information not available>\n");
		}
	}
	else
	{
		pfn->Invoke(L"No stack frame information.\n");
	}
}

void DKError::PrintDescriptionWithStackFrames(void) const
{
	PrintDescriptionWithStackFrames(DKFunction((void (*)(const DKString&))&DKLog));
}

void DKError::PrintDescriptionWithStackFrames(const StringOutput* pfn) const
{
	PrintDescription(pfn);
	PrintStackFrames(pfn);
}

bool DKError::WriteToFile(const DKString& file) const
{
	if (file.Length() > 0)
	{
		DKObject<DKFile> f = DKFile::Create(file, DKFile::ModeOpenAlways, DKFile::ModeShareExclusive);
		if (f)
		{
			return WriteToStream(f.SafeCast<DKStream>());
		}
	}
	return false;
}

bool DKError::WriteToStream(DKStream* stream) const
{
	if (stream && stream->IsWritable())
	{
		struct FileToDescriptor
		{
			FileToDescriptor(DKStream* s) : stream(s) {}
			void operator () (const void* p, size_t s)
			{
				stream->Write(p, s);
			}
			DKStream* stream;
		};
		WriteToDescriptor(DKFunction(FileToDescriptor(stream)));
		return true;
	}
	return false;
}

void DKError::WriteToDescriptor(const Descriptor* d) const
{
	if (d)
	{
		struct UTF8StrCCat
		{
			UTF8StrCCat(DKStringU8& s) : result(s) {}
			void operator () (const DKString& s)
			{
				result += s;
			}
			DKStringU8& result;
		};
		DKStringU8 text = "";
		DKObject<StringOutput> sp = DKFunction(UTF8StrCCat(text));

		PrintDescriptionWithStackFrames(sp);

		d->Invoke((const char*)text, text.Bytes());
	}
}

void DKError::WriteToDefaultDescriptor(void) const
{
	Private::fileLock.Lock();
	DKObject<Descriptor> d = Private::defaultDescriptor;
	Private::fileLock.Unlock();

	if (d)
	{
		WriteToDescriptor(d);
	}
	else
	{
		DKLog("[DKError::WriteToDefaultDescriptor] Error: default descriptor not defined.\n");
	}
}

void DKError::SetDefaultDescriptor(const Descriptor* d)
{
	DKCriticalSection<DKSpinLock> guard(Private::fileLock);
	Private::defaultDescriptor = const_cast<Descriptor*>(d);
}

void DKError::DumpUnexpectedError(Private::UnexpectedError* e)
{
	DKError err(e->code, e->description);
	err.numFrames = e->callstack.Count();
	if (err.numFrames > 0)
	{
		StackFrame* frames = (StackFrame*)e->callstack;

		err.numFrames = e->callstack.Count();
		err.stackFrames = new StackFrame[err.numFrames];
		for (size_t i = 0; i < err.numFrames; ++i)
			err.stackFrames[i] = frames[i];
	}

	DKLog("Unexpected exception occurred from thread id:0x%x\n", DKThread::CurrentThreadId());
	err.PrintDescriptionWithStackFrames();	
	
	DKObject<DKCriticalErrorHandler> critErrFunc = Private::GetCriticalErrorHandler();	
	if (critErrFunc)
	{
		critErrFunc->Invoke(err);
	}
	else
	{
		err.WriteToDefaultDescriptor();
	}
}
