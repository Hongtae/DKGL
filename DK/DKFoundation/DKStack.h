//
//  File: DKStack.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKQueue.h"
#include "DKMemory.h"

namespace DKFoundation
{
	/// a stack implemented template class.
	template <typename VALUE, typename ALLOC = DKMemoryDefaultAllocator>
	class DKStack
	{
	public:
		typedef ALLOC					Allocator;
		typedef DKQueue<VALUE, ALLOC>	Container;

		constexpr static size_t NodeSize()	{ return Container::NodeSize(); }

		DKStack() {}
		~DKStack() {}
		
		void Push(const VALUE& v)	{container.PushFront(v);}
		void Push(VALUE&& v)		{container.PushFront(static_cast<VALUE&&>(v)); }
		void Pop(VALUE& v)			{container.PopFront(v);}
		void Pop()					{container.PopFront();}

		VALUE& Top()				{return container.Front();}
		const VALUE& Top() const	{return container.Front();}
		
		bool IsEmpty() const		{return container.IsEmpty();}
		void Clear()				{container.Clear();}
		
		size_t Count()				{return container.Count();}

		/// EnumerateForward / EnumerateBackward: enumerate all items.
		/// You cannot insert, remove items while enumerating. (container is read-only)
		/// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		/// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");

			container.EnumerateForward(std::forward<T>(enumerator));
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");

			container.EnumerateBackward(std::forward<T>(enumerator));
		}
		/// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			container.EnumerateForward(std::forward<T>(enumerator));
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			container.EnumerateBackward(std::forward<T>(enumerator));
		}
	private:
		Container container;
	};
}
