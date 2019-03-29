//
//  File: DKMap.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2017 Hongtae Kim. All rights reserved.
//

#pragma once
#include <initializer_list>
#include "../DKInclude.h"
#include "DKAVLTree.h"
#include "DKDummyLock.h"
#include "DKCriticalSection.h"
#include "DKTypeTraits.h"

namespace DKFoundation
{
	template <typename Key, typename Value> class DKMapPair
	{
	public:
		DKMapPair(const Key& k, const Value& v)
			: key(k), value(v) {}
		DKMapPair(const Key& k, Value&& v)
			: key(k), value(static_cast<Value&&>(v)) {}
		DKMapPair(const DKMapPair& p)
			: key(p.key), value(p.value) {}
		DKMapPair(DKMapPair&& p)
			: key(static_cast<Key&&>(p.key)), value(static_cast<Value&&>(p.value)) {}

		Key key;
		Value value;
	};
	template <typename Key> struct DKMapKeyComparator
	{
		int operator () (const Key& lhs, const Key& rhs) const
		{
			if (lhs > rhs)
				return 1;
			else if (lhs < rhs)
				return -1;
			return 0;
		}
	};
	template <typename Value> struct DKMapValueReplacer
	{
		void operator () (Value& dst, const Value& src) const
		{
			dst = src;
		}
	};

	/**
	 @brief
	 balancing tree class (using AVLTree internally, see DKAVLTree.h).
	 this is simple wrapper of DKAVLTree.

	 Insert: insert value if key is not exists.
	 Update: set value for key whether key is exists or not.

	 insertion, deletion, lookup is thread-safe.
	 If you need to modify value directly, you should have lock object.

	 Example:
	 @code
		{
			typename MyMapType::CriticalSection section(map.lock);	// lock with critical-section
			MyMapType::Pair* p = map.Find(something);
			.... // do something with p
		}	// auto-unlock by critical-section end
	 @endcode

	 To enumerate items:
	 @code
	  typedef DKMap<Key,Value> MyMap;
	  MyMap map;
	  auto enumerator1 = [](const MyMap::Pair& pair) {...}
	  auto enumerator2 = [](const MyMap::Pair& pair, bool* stop) {...}
	  map.EnumerateForward(enumerator1);
	  map.EnumerateForward(enumerator2);	// cancellable by set bool to true.
	 @endcode

	 @tparam Key            key type
	 @tparam ValueT         value type
	 @tparam Lock           locking class
	 @tparam KeyComparator  key comparison function
	 @tparam ValueReplacer  value copy/swap function

	 @see DKAVLTree
	 */
	template <
		typename Key,											// key type
		typename ValueT,										// value type
		typename Lock = DKDummyLock,							// lock
		typename KeyComparator = DKMapKeyComparator<Key>,		// key comparison
		typename ValueReplacer = DKMapValueReplacer<ValueT>,	// copy value
		typename Allocator = DKMemoryDefaultAllocator			// memory allocator
	>
	class DKMap
	{
	public:
		typedef DKMapPair<const Key, ValueT>	Pair;
		typedef DKCriticalSection<Lock>			CriticalSection;
		typedef DKTypeTraits<Key>				KeyTraits;
		typedef DKTypeTraits<ValueT>			ValueTraits;

		struct PairComparator
		{
			int operator () (const Pair& lhs, const Pair& rhs) const
			{
				return comparator(lhs.key, rhs.key);
			}
			KeyComparator comparator;
		};
		struct PairValueReplacer
		{
			void operator () (Pair& dst, const Pair& src) const
			{
				replacer(dst.value, src.value);
			}
			ValueReplacer replacer;
		};
		typedef DKAVLTree<Pair, PairComparator, PairValueReplacer, Allocator> Container;
		constexpr static size_t NodeSize() { return Container::NodeSize(); }

		KeyComparator comparator;

		/// lock is public. to provde lock object from outside!
		/// FindNoLock, CountNoLock is usable regardless of locking.
		Lock	lock;

		DKMap()
		{
		}
		DKMap(DKMap&& m)
			: container(static_cast<Container&&>(m.container))
			, comparator(static_cast<KeyComparator&&>(m.comparator))
		{
		}
		DKMap(const DKMap& m)
		{
			CriticalSection guard(m.lock);
			container = m.container;
			comparator = m.comparator;
		}
		DKMap(std::initializer_list<Pair> il)
		{
			for (const Pair& p : il)
				container.Insert(p);
		}
		template <typename K, typename V>
		DKMap(std::initializer_list<K> keys, std::initializer_list<V> values)
		{
			DKASSERT_DEBUG(keys.size() == values.size());
			auto k = keys.begin();
			auto k_end = keys.end();
			auto v = values.begin();
			auto v_end = values.end();
			while (k != k_end && v != v_end)
			{
				Insert(*k, *v);
				++k; ++v;
			}
		}
		~DKMap()
		{
			Clear();
		}
		/// overwrite value if key is exists, or insert item.
		void Update(const Pair& p)
		{
			CriticalSection guard(lock);
			container.Update(p);			
		}
		void Update(Pair&& p)
		{
			CriticalSection guard(lock);
			container.Update(static_cast<Pair&&>(p));			
		}
		void Update(const Key& k, const ValueT& v)
		{
			Update(Pair(k,v));
		}
		void Update(const Key& k, ValueT&& v)
		{
			Update(Pair(k,static_cast<ValueT&&>(v)));
		}
		void Update(const Pair* p, size_t size)
		{
			for (size_t i = 0; i < size; i++)
				Update(p[i]);
		}
		template <typename ...Args>
		void Update(const DKMap<Key, ValueT, Args...>& m)
		{
			CriticalSection guard(lock);
			m.EnumerateForward([this](const typename DKMap<Key, ValueT, Args...>::Pair& pair)
			{
				container.Update(pair);
			});
		}
		void Update(std::initializer_list<Pair> il)
		{
			CriticalSection guard(lock);
			for (const Pair& p : il)
				container.Update(p);
		}
		template <typename K, typename V>
		void Update(std::initializer_list<K> keys, std::initializer_list<V> values)
		{
			DKASSERT_DEBUG(keys.size() == values.size());

			auto k = keys.begin();
			auto k_end = keys.end();
			auto v = values.begin();
			auto v_end = values.end();

			CriticalSection guard(lock);
			while (k != k_end && v != v_end)
			{
				Update(*k, *v);
				++k; ++v;
			}
		}
		/// insert item if key is not exist, fails otherwise.
		bool Insert(const Pair& p)
		{
			CriticalSection guard(lock);
			return container.Insert(p) != NULL;
		}
		bool Insert(Pair&& p)
		{
			CriticalSection guard(lock);
			return container.Insert(static_cast<Pair&&>(p)) != NULL;
		}
		bool Insert(const Key& k, const ValueT& v)
		{
			return Insert(Pair(k, v));
		}
		bool Insert(const Key& k, ValueT&& v)
		{
			return Insert(Pair(k, static_cast<ValueT&&>(v)));
		}
		template <typename ...Args> size_t Insert(const DKMap<Key, ValueT, Args...>& m)
		{
			size_t n = 0;
			CriticalSection guard(lock);
			m.EnumerateForward([this, &n](const typename DKMap<Key, ValueT, Args...>::Pair& pair)
			{
				if (container.Insert(pair) != NULL)
					n++;
			});
			return n;
		}
		size_t Insert(std::initializer_list<Pair> il)
		{
			size_t n = 0;
			CriticalSection guard(lock);
			for (const Pair& p : il)
			{
				if (container.Insert(p) != NULL)
					n++;
			}
			return n;
		}
		template <typename K, typename V>
		size_t Insert(std::initializer_list<K> keys, std::initializer_list<V> values)
		{
			DKASSERT_DEBUG(keys.size() == values.size());
			size_t n = 0;
			auto k = keys.begin();
			auto k_end = keys.end();
			auto v = values.begin();
			auto v_end = values.end();

			CriticalSection guard(lock);
			while (k != k_end && v != v_end)
			{
				if (container.Insert(Pair(*k, *v)))
					n++;
				++k; ++v;
			}
			return n;
		}
		void Remove(const Key& k)
		{
			CriticalSection guard(lock);
			container.Remove(k, [this](const Pair& lhs, const Key& key)
			{
				return comparator(lhs.key, key);
			});
		}
		void Remove(std::initializer_list<Key> il)
		{
			CriticalSection guard(lock);
			for (const Key& k : il)
				container.Remove(k);
		}
		void Clear()
		{
			CriticalSection guard(lock);
			container.Clear();
		}
		Pair* Find(const Key& k)
		{
			return const_cast<Pair*>(static_cast<const DKMap&>(*this).Find(k));
		}
		const Pair* Find(const Key& k) const
		{
			CriticalSection guard(lock);
			return FindNoLock(k);
		}
		/// Perform search operation without locking.
		/// useful if you have locked already in your context.
		Pair* FindNoLock(const Key& k)
		{
			return const_cast<Pair*>(static_cast<const DKMap&>(*this).FindNoLock(k));
		}
		const Pair* FindNoLock(const Key& k) const
		{
			return container.Find(k, [this](const Pair& lhs, const Key& key)
			{
				return comparator(lhs.key, key);
			});
		}
		/// if key 'k' is not exist, an new value inserted and returns.
		ValueT& Value(const Key& k)
		{
			CriticalSection guard(lock);
			Pair* p = FindNoLock(k);
			if (p == NULL)
				p = const_cast<Pair*>(container.Insert(Pair(k, ValueT())));
			return p->value;
		}
		bool IsEmpty() const
		{
			CriticalSection guard(lock);
			return container.Count() == 0;
		}
		size_t Count() const
		{
			CriticalSection guard(lock);
			return container.Count();
		}
		size_t CountNoLock() const
		{
			return container.Count();
		}
		DKMap& operator = (DKMap&& m)
		{
			if (this != &m)
			{
				CriticalSection guard(lock);
				container = static_cast<Container&&>(m.container);
				comparator = static_cast<KeyComparator&&>(m.comparator);
			}
			return *this;
		}
		DKMap& operator = (const DKMap& m)
		{
			if (this != &m)
			{
				CriticalSection guardOther(m.lock);
				CriticalSection guardSelf(lock);

				container = m.container;
				comparator = m.comparator;
			}
			return *this;
		}
		DKMap& operator = (std::initializer_list<Pair> il)
		{
			CriticalSection guard(lock);
			container.Clear();
			for (const Pair& p : il)
				container.Insert(p);
			return *this;
		}
		/// EnumerateForward / EnumerateBackward: enumerate all items.
		/// You cannot insert, remove items while enumerating. (container is read-only)
		/// enumerator can be lambda or any function type that can receive arguments (VALUE&) or (VALUE&, bool*)
		/// (VALUE&, bool*) type can cancel iteration by set boolean value to true.
		template <typename T> void EnumerateForward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<Pair&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<Pair&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator)
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<Pair&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<Pair&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (VALUE&) or (VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		/// lambda enumerator (const VALUE&) or (const VALUE&, bool*) function type.
		template <typename T> void EnumerateForward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const Pair&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const Pair&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateForward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}
		template <typename T> void EnumerateBackward(T&& enumerator) const
		{
			using Func = typename DKFunctionType<T>::Signature;
			enum {ValidatePType1 = Func::template CanInvokeWithParameterTypes<const Pair&>()};
			enum {ValidatePType2 = Func::template CanInvokeWithParameterTypes<const Pair&, bool*>()};
			static_assert(ValidatePType1 || ValidatePType2, "enumerator's parameter is not compatible with (const VALUE&) or (const VALUE&,bool*)");

			EnumerateBackward(std::forward<T>(enumerator), typename Func::ParameterNumber());
		}

	private:
		// lambda enumerator (VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			container.EnumerateForward([&enumerator](Pair& val, bool*) {enumerator(val);});
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>)
		{
			CriticalSection guard(lock);
			container.EnumerateBackward([&enumerator](Pair& val, bool*) {enumerator(val);});
		}
		// lambda enumerator (const VALUE&)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			container.EnumerateForward([&enumerator](const Pair& val, bool*) {enumerator(val);});
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<1>) const
		{
			CriticalSection guard(lock);
			container.EnumerateBackward([&enumerator](const Pair& val, bool*) {enumerator(val);});
		}
		// lambda enumerator (VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			container.EnumerateForward(enumerator);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>)
		{
			CriticalSection guard(lock);
			container.EnumerateBackward(enumerator);
		}
		// lambda enumerator (const VALUE&, bool*)
		template <typename T> void EnumerateForward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			container.EnumerateForward(enumerator);
		}
		template <typename T> void EnumerateBackward(T&& enumerator, DKNumber<2>) const
		{
			CriticalSection guard(lock);
			container.EnumerateBackward(enumerator);
		}
		
		Container	container;
	};
}
