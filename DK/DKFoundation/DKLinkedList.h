//
//  File: DKLinkedList.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include <initializer_list>
#include "../DKInclude.h"
#include "DKTypeTraits.h"
#include "DKMemory.h"

namespace DKFoundation
{
	/**
	 @brief A simple linked list.

	 @note
	  When using range-based-for-loop, object does not locked by default.
	  You should lock list from outside of loop.

	 @note
	  This class does not tested fully. (may have bugs?)
	 */
	template <typename VALUE, typename ALLOC = DKMemoryDefaultAllocator> class DKLinkedList
	{
	private:
		struct Node
		{
			Node(const VALUE& v, Node* p, Node* n) : value(v), prev(p), next(n) {}
			Node(VALUE&& v, Node* p, Node* n) : value(static_cast<VALUE&&>(v)), prev(p), next(n) {}
			VALUE value;
			Node* prev;
			Node* next;
		};

	public:
		typedef DKTypeTraits<VALUE>		ValueTraits;
		typedef ALLOC					Allocator;

		/// defined for range-based loop
		template <typename NodeT, typename ValueT> class IteratorT
		{
		public:
			IteratorT(const IteratorT& it) : node(it.node) {}
			ValueT Value()				{return node->value;}
			bool IsValid() const		{return node != NULL;}
			IteratorT Next()			{return IteratorT(node->next);}
			IteratorT Prev()			{return IteratorT(node->prev);}

			/// member functions for RB-loop
			IteratorT& operator ++ ()					{node = node->next; return *this;}	// prefix++
			ValueT operator * ()						{return node->value;};
			bool operator != (const IteratorT& it) const	{return node != it.node;}
		private:
			IteratorT(NodeT* n) : node(n) {}
			NodeT* node;
			friend class DKLinkedList;
		};
		typedef IteratorT<Node, VALUE&> Iterator;
		typedef IteratorT<const Node, const VALUE&> ConstIterator;

		constexpr static size_t NodeSize()	{ return sizeof(Node); }

		DKLinkedList() : firstNode(NULL), lastNode(NULL), count(0)
		{
		}
		DKLinkedList(DKLinkedList&& list) : firstNode(NULL), lastNode(NULL), count(0)
		{
			firstNode = list.firstNode;
			lastNode = list.lastNode;
			count = list.count;
			list.firstNode = NULL;
			list.lastNode = NULL;
			list.count = 0;
		}
		DKLinkedList(const DKLinkedList& list) : firstNode(NULL), lastNode(NULL), count(0)
		{
			for (const Node* n = list.firstNode; n != NULL; n = n->next)
				AppendT(n->value);
		}
		DKLinkedList(const VALUE* values, size_t num) : firstNode(NULL), lastNode(NULL), count(0)
		{
			for (size_t i = 0; i < num; ++i)
				AppendT(values[i]);
		}
		DKLinkedList(std::initializer_list<VALUE> il) : firstNode(NULL), lastNode(NULL), count(0)
		{
			for (const VALUE& v : il)
				AppendT(il);
		}
		~DKLinkedList()
		{
			Clear();
		}
		size_t Count() const
		{
			return count;
		}
		void Clear()
		{
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
		Iterator Head()
		{
			return firstNode;
		}
		ConstIterator Head() const
		{
			return firstNode;
		}
		Iterator Tail()
		{
			return lastNode;
		}
		ConstIterator Tail() const
		{
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
		Iterator Prepend(const VALUE& v)
		{
			return PrependT(v);
		}
		Iterator Prepend(VALUE&& v)
		{
			return PrependT(static_cast<VALUE&&>(v));
		}
		Iterator Append(const VALUE& v)
		{
			return AppendT(v);
		}
		Iterator Append(VALUE&& v)
		{
			return AppendT(static_cast<VALUE&&>(v));
		}

		Iterator begin()				{return firstNode;}	///< implemented for range-based for loop
		ConstIterator begin() const		{return firstNode;}	///< implemented for range-based for loop
		Iterator end()					{return NULL;}	///< implemented for range-based for loop
		ConstIterator end() const		{return NULL;}	///< implemented for range-based for loop

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
		DKLinkedList& operator = (DKLinkedList&& list)
		{
			if (this != &list)
			{
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
		DKLinkedList& operator = (const DKLinkedList& list)
		{
			if (this != &list)
			{
				Clear();
				Append(list);
			}
			return *this;
		}
		DKLinkedList& operator = (std::initializer_list<VALUE> il)
		{
			Clear();
			for (const VALUE& v : il)
				Append(v);
			return *this;
		}
		DKLinkedList operator + (const VALUE& v) const
		{
			DKLinkedList	ret(*this);
			ret.Append(v);
			return ret;
		}
		DKLinkedList operator + (const DKLinkedList& v) const
		{
			DKLinkedList ret(*this);
			return ret + v;
		}
		DKLinkedList operator + (std::initializer_list<VALUE> il)
		{
			DKLinkedList ret(*this);
			for (const VALUE& v : il)
				ret.AppendT(v);
			return ret;
		}
		DKLinkedList& operator += (const VALUE& v) const
		{
			Append(v);
			return *this;
		}
		DKLinkedList& operator += (const DKLinkedList& v)
		{
			for (const Node* n = v.firstNode; n != NULL; n = n->next)
				AppendT(n->value);
			return *this;
		}
		DKLinkedList& operator += (std::initializer_list<VALUE> il)
		{
			for (const VALUE& v : il)
				AppendT(v);
			return *this;
		}
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
			
			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");
			
			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		/// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const VALUE&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const VALUE&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}

	private:
		// lambda enumerator (VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			for (Node* n = firstNode; n != NULL; n = n->next)
				enumerator(n->value);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			for (Node* n = lastNode; n != NULL; n = n->prev)
				enumerator(n->value);
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			for (Node* n = firstNode; n != NULL; n = n->next)
				enumerator(n->value);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			for (Node* n = lastNode; n != NULL; n = n->prev)
				enumerator(n->value);
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			for (Node* n = firstNode; n != NULL && !stop; n = n->next)
				enumerator(n->value, &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			bool stop = false;
			for (Node* n = lastNode; n != NULL && !stop; n = n->prev)
				enumerator(n->value, &stop);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			for (Node* n = firstNode; n != NULL && !stop; n = n->next)
				enumerator(n->value, &stop);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			bool stop = false;
			for (Node* n = lastNode; n != NULL && !stop; n = n->prev)
				enumerator(n->value, &stop);
		}
		template <typename T> Iterator PrependT(T&& v)
		{
			Node* n = new( Allocator::Alloc(sizeof(Node)) ) Node(std::forward<T>(v), NULL, firstNode);
			if (firstNode)
				firstNode->prev = n;
			firstNode = n;
			if (lastNode == NULL)
				lastNode = n;

			count++;
			return n;
		}
		template <typename T> Iterator AppendT(T&& v)
		{
			Node* n = new( Allocator::Alloc(sizeof(Node)) ) Node(std::forward<T>(v), lastNode, NULL);
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
