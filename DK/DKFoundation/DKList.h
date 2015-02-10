//
//  File: DKList.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include <initializer_list>
#include "DKTypeTraits.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKMemory.h"

////////////////////////////////////////////////////////////////////////////////
// DKList
// a simple linked list.
//
// Note:
//  When using range-based-for-loop, object does not locked by default.
//  You should lock list from outside of loop.
//
// Range based for loop example:
//  DKList<MyObject> myList;
//  // lock within current scope for range-based-for-loop
//  DKList<MyObject>::CriticalSection(myList.lock);
//  for (MyObject& obj : myList)
//     // do something with obj..
//
// Iterator iteration example:
//  DKList<MyObject> myList;
//  for (auto it = myList.LockHead(); it.IsValid(); ++it)
//  {
//       MyObject& obj = it.Value();
//       // do something with obj..
//  }
//  myList.Unlock();
//
// Note:
//  This class does not tested fully. (may have bugs?)
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	template <typename VALUE, typename LOCK = DKDummyLock, typename ALLOC = DKMemoryDefaultAllocator> class DKList
	{
	private:
		struct Node
		{
			Node(const VALUE& v, Node* p, Node* n) : value(v), prev(p), next(n) {}
			VALUE value;
			Node* prev;
			Node* next;
		};

	public:
		typedef LOCK					Lock;
		typedef DKCriticalSection<LOCK>	CriticalSection;
		typedef DKTypeTraits<VALUE>		ValueTraits;
		typedef ALLOC					Allocator;

		template <typename NodeT, typename ValueT> class IteratorT
		{
		public:
			IteratorT(const IteratorT& it) : node(it.node) {}
			ValueT Value(void)				{return node->value;}
			bool IsValid(void) const		{return node != NULL;}
			IteratorT Next(void)			{return IteratorT(node->next);}
			IteratorT Prev(void)			{return IteratorT(node->prev);}

			// member functions for RB-loop
			IteratorT& operator ++ (void)					{node = node->next; return *this;}	// prefix++
			ValueT operator * (void)						{return node->value;};
			bool operator != (const IteratorT& it) const	{return node != it.node;}
		private:
			IteratorT(NodeT* n) : node(n) {}
			NodeT* node;
			friend class DKList;
		};
		typedef IteratorT<Node, VALUE&> Iterator;
		typedef IteratorT<const Node, const VALUE&> ConstIterator;

		// lock is public. to provde lock object from outside!
		Lock lock;

		DKList(void) : firstNode(NULL), lastNode(NULL), count(0)
		{
		}
		DKList(DKList&& list) : firstNode(NULL), lastNode(NULL), count(0)
		{
			firstNode = list.firstNode;
			lastNode = list.lastNode;
			count = list.count;
			list.firstNode = NULL;
			list.lastNode = NULL;
			list.count = 0;
		}
		DKList(const DKList& list) : firstNode(NULL), lastNode(NULL), count(0)
		{
			CriticalSection guard(list.lock);

			for (const Node* n = list.firstNode; n != NULL; n = n->next)
				AddTailNL(n->value);
		}
		DKList(const VALUE* values, size_t num) : firstNode(NULL), lastNode(NULL), count(0)
		{
			for (size_t i = 0; i < num; ++i)
				AddTailNL(values[i]);
		}
		DKList(std::initializer_list<VALUE> il) : firstNode(NULL), lastNode(NULL), count(0)
		{
			for (const VALUE& v : il)
				AddTailNL(il);
		}
		~DKList(void)
		{
			Clear();
		}
		size_t Count(void) const
		{
			CriticalSection guard(lock);
			return count;
		}
		size_t CountNoLock(void) const
		{
			return count;
		}
		void Clear(void)
		{
			CriticalSection guard(lock);
			Node* n = firstNode;
			while (n)
			{
				Node* next = n->next;
				Allocator::Free(n);
				n = next;
			}
			firstNode = NULL;
			lastNode = NULL;
			count = 0;
		}
		Iterator LockHead(void)
		{
			lock.Lock();
			return firstNode;
		}
		ConstIterator LockHead(void) const
		{
			lock.Lock();
			return firstNode;
		}
		Iterator LockTail(void)
		{
			lock.Lock();
			return lastNode;
		}
		ConstIterator LockTail(void) const
		{
			lock.Lock();
			return lastNode;
		}
		void Remove(Iterator& it)
		{
			DKASSERT_DESC_DEBUG(CheckIterator(it), "Invalid iterator");
			if (it.node)
			{
				DKASSERT_DEBUG(count > 0);

				Node* prev = it.node->prev;
				Node* next = it.node->next;

				if (prev)
					prev->next = next;
				else
				{
					DKASSERT_DEBUG(firstNode == it.node);
					firstNode = next;
				}

				if (next)
					next->prev = prev;
				else
				{
					DKASSERT_DEBUG(lastNode == it.node);
					lastNode = prev;
				}

				Node* n = it.node;
				it.node = NULL;
				n->~Node();
				Allocator::Free(n);
				count--;
			}
		}
		void Unlock(void)
		{
			DKASSERT_DESC_DEBUG(lock.TryLock() == false, "List does not locked");
			lock.Unlock();
		}
		Iterator AddHead(const VALUE& v)
		{
			CriticalSection guard(lock);
			return AddHeadNL(v);
		}
		Iterator AddTail(const VALUE& v)
		{
			CriticalSection guard(lock);
			return AddTailNL(v);
		}

		// Iterator class for range-based for loop.
		Iterator begin(void)				{return firstNode;}
		ConstIterator begin(void) const		{return firstNode;}
		Iterator end(void)					{return NULL;}
		ConstIterator end(void) const		{return NULL;}

		bool CheckIterator(const Iterator& it) const
		{
			if (it.node)
			{
				for (const Node* n = this->firstNode; n != NULL; n = n->next)
				{
					if (it.node == n)
						return true;
				}
				return false;
			}
			return true;
		}
		bool CheckIterator(const ConstIterator& it) const
		{
			if (it.node)
			{
				for (const Node* n = this->firstNode; n != NULL; n = n->next)
				{
					if (it.node == n)
						return true;
				}
				return false;
			}
			return true;
		}
		DKList& operator = (DKList&& list)
		{
			if (this != &list)
			{
				CriticalSection guard(lock);
				Node* n = firstNode;
				while (n)
				{
					Node* next = n->next;
					Allocator::Free(n);
					n = next;
				}
				firstNode = list.firstNode;
				lastNode = list.lastNode;
				count = list.count;

				list.firstNode = NULL;
				list.lastNode = NULL;
				list.count = 0;
			}
			return *this;
		}
		DKList& operator = (const DKList& list)
		{
			if (this != &list)
			{
				Clear();
				AddTail(list);
			}
			return *this;
		}
		DKList& operator = (std::initializer_list<VALUE> il)
		{
			Clear();
			for (const VALUE& v : il)
				AddTail(v);
			return *this;
		}
		DKList operator + (const VALUE& v) const
		{
			DKList	ret(*this);
			ret.AddTail(v);
			return ret;
		}
		DKList operator + (const DKList& v) const
		{
			DKList ret(*this);
			return ret + v;
		}
		DKList operator + (std::initializer_list<VALUE> il)
		{
			DKList ret(*this);
			for (const VALUE& v : il)
				ret.AddTailNL(v);
			return ret;
		}
		DKList& operator += (const VALUE& v) const
		{
			AddTail(v);
			return *this;
		}
		DKList& operator += (const DKList& v)
		{
			CriticalSection guard1(lock);
			CriticalSection guard2(v.lock);
			for (const Node* n = v.firstNode; n != NULL; n = n->next)
				AddTailNL(n->value);
			return *this;
		}
		DKList& operator += (std::initializer_list<VALUE> il)
		{
			CriticalSection guard(lock);
			for (const VALUE& v : il)
				AddTailNL(v);
			return *this;
		}
		// EnumerateForward / EnumerateBackward: enumerate all items.
		// You cannot insert, remove items while enumerating. (container is read-only)
		// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T&&>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}

	private:
		// lambda enumerator (VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			for (Node* n = firstNode; n != NULL; n = n->next)
				enumerator(n->value);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			for (Node* n = lastNode; n != NULL; n = n->prev)
				enumerator(n->value);
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			for (Node* n = firstNode; n != NULL; n = n->next)
				enumerator(n->value);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			for (Node* n = lastNode; n != NULL; n = n->prev)
				enumerator(n->value);
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (Node* n = firstNode; n != NULL && !stop; n = n->next)
				enumerator(n->value, &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (Node* n = lastNode; n != NULL && !stop; n = n->prev)
				enumerator(n->value, &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (Node* n = firstNode; n != NULL && !stop; n = n->next)
				enumerator(n->value, &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			bool stop = false;
			for (Node* n = lastNode; n != NULL && !stop; n = n->prev)
				enumerator(n->value, &stop);
		}
		Iterator AddHeadNL(const VALUE& v)
		{
			Node* n = new( Allocator::Alloc(sizeof(Node)) ) Node(v, NULL, firstNode);
			if (firstNode)
				firstNode->prev = n;
			firstNode = n;
			if (lastNode == NULL)
				lastNode = n;

			count++;
			return n;
		}
		Iterator AddTailNL(const VALUE& v)
		{
			Node* n = new( Allocator::Alloc(sizeof(Node)) ) Node(v, lastNode, NULL);
			if (lastNode)
				lastNode->next = n;
			lastNode = n;
			if (firstNode == NULL)
				firstNode = n;

			count++;
			return n;
		}

		
		Node* firstNode;
		Node* lastNode;
		size_t count;
	};
}
