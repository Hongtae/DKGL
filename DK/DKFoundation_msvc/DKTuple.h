//
//  File: DKTuple.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include <type_traits>
#include "../DKInclude.h"
#include "DKTypeList.h"
#include "DKTypeTraits.h"

////////////////////////////////////////////////////////////////////////////////
// DKTuple
// A tuple template class.
// Tuple can contains various types and values.
//
// Example:
//  - create object and set values.
//     DKTuple<int, float, const char*> myTuple;
//     myTuple.SetValue<0>(2, 3.14f, "tuple");
//
//  - set values individually.
//     DKTuple<int, float, const char*> myTuple;
//     myTuple.SetValue<0>(2);
//     myTuple.SetValue<1>(3.14f);
//     myTuple.SetValue<2>("tuple");
//
//  - using DKTupleMake helper function.
//     auto myTuple = DKTupleMake(2, 3.14f, "tuple");
//
//  - use tuple class constructor.
//     DKTuple<int, float, const char*> myTuple(DKTupleValueSet(), 2, 3.14f, "tuple");
//
//     (You must specify DKTupleValueSet() as first argument,
//      it is dummy object but required for overloaded constructor
//      which makes compiler able to distinguish constructer
//      with variadic-initial-values from other constructors.)
//
//  - retrieve value from tuple.
//   int n = myTuple.Value<0>();
//   float f = myTuple.Value<1>();
//   const char* str = myTuple.Value<2>();
//
// Note:
//  This file re-written for MSVC without using 'constexpr'. (Visual Studio 2013)
////////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	// DKTupleUnit: a class which can store value for specified template type.
	// Note: References are stored as pointers.
	template <typename T> struct DKTupleUnit
	{
		using ValueType = typename DKTypeTraits<T>::UnqualifiedType;
		using RefType = T&;
		using CRefType = const T&;

		ValueType value;

		DKTupleUnit(void) {}
		DKTupleUnit(const T& v) : value(v) {}
		DKTupleUnit(T&& v) : value(static_cast<T&&>(v)) {}
		DKTupleUnit(const DKTupleUnit& t) : value(t.value) {}
		DKTupleUnit(DKTupleUnit&& t) : value(static_cast<ValueType&&>(t.value)) {}

		void SetValue(const T& v)	{this->value = v;}
		void SetValue(T&& v)		{this->value = static_cast<T&&>(v);}
		RefType Value(void)			{return this->value;}
		CRefType Value(void) const	{return this->value;}

		DKTupleUnit& operator = (const T& v)
		{
			value = v;
			return *this;
		}
		DKTupleUnit& operator = (const DKTupleUnit& t)
		{
			value = t.value;
			return *this;
		}
		DKTupleUnit& operator = (T&& v)
		{
			this->value = static_cast<T&&>(v);
			return *this;
		}
		DKTupleUnit& operator = (DKTupleUnit&& t)
		{
			this->value = static_cast<ValueType&&>(t.value);
			return *this;
		}
	};
	template <typename T> struct DKTupleUnit<T&>
	{
		using ValueType = typename DKTypeTraits<T>::UnqualifiedType*;
		using RefType = T&;
		using CRefType = const T&;

		ValueType value;

		DKTupleUnit(void) : value(NULL) {}
		DKTupleUnit(const DKTupleUnit& t) : value(t.value) {}
		DKTupleUnit(DKTupleUnit&& t) : value(t.value) {t.value = NULL;}
		DKTupleUnit(T& v) : value(const_cast<ValueType>(&v)) {}

		void SetValue(T& v)			{this->value = const_cast<ValueType>(&v);}
		RefType Value(void)			{return *this->value;}
		CRefType Value(void) const	{return *this->value;}

		DKTupleUnit& operator = (T& v)
		{
			this->value = const_cast<ValueType>(&v);
			return *this;
		}
		DKTupleUnit& operator = (const DKTupleUnit& t)
		{
			value = t.value;
			return *this;
		}
		DKTupleUnit& operator = (DKTupleUnit&& t)
		{
			this->value = t.value;
			t.value = NULL;
			return *this;
		}
	};
	template <typename T> struct DKTupleUnit<T&&>
	{
		using ValueType = typename DKTypeTraits<T>::UnqualifiedType;
		using RefType = T&&;
		using CRefType = const T&;

		ValueType value;

		DKTupleUnit(void) {}
		DKTupleUnit(const DKTupleUnit& t) : value(t.value) {}
		DKTupleUnit(DKTupleUnit&& t) : value(static_cast<ValueType&&>(t.value)) {}
		DKTupleUnit(T&& v) : value(static_cast<T&&>(v)) {}

		void SetValue(T&& v)		{this->value = static_cast<T&&>(v);}
		RefType Value(void)			{return static_cast<RefType>(this->value);}
		CRefType Value(void) const	{return static_cast<CRefType>(this->value);}

		DKTupleUnit& operator = (T&& v)
		{
			this->value = static_cast<T&&>(v);
			return *this;
		}
		DKTupleUnit& operator = (const DKTupleUnit& t)
		{
			value = t.value;
			return *this;
		}
		DKTupleUnit& operator = (DKTupleUnit&& t)
		{
			this->value = static_cast<ValueType&&>(t.value);
			return *this;
		}
	};

	// DKTupleValueSet: a dummy type for overloaded constructor.
	// Required for overloaded function with variadic templates.
	struct DKTupleValueSet {};
	template <typename... Types> class DKTuple
	{
		// a data unit object that includes single DKTupleUnit.
		// each types can be recognized with index.
		// one object should contains all types with inheritance.
		template <size_t Level, typename... Ts> struct DataUnitHierarchy
		: public DataUnitHierarchy<Level-1, Ts...>
		{
			using Super = DataUnitHierarchy<Level-1, Ts...>;
			using TypeList = DKTypeList<Ts...>;
			enum {Index = TypeList::Length - Level};
			static_assert(Index >= 0, "Index must be greater or equal to zero");
			using UnitType = typename TypeList::template TypeAt<Index>;
			using TupleUnit = DKTupleUnit<UnitType>;

			DataUnitHierarchy(void) {}
			DataUnitHierarchy(const DataUnitHierarchy& d) : Super(d), unit(d.unit) {}
			DataUnitHierarchy(DataUnitHierarchy&& d) : Super(static_cast<Super&&>(d)), unit(static_cast<TupleUnit&&>(d.unit)) {}

			// Use DKTupleValueSet dummy object as first argument
			// to use variadic templates for constructor.
			template <typename V, typename... Vs>
			DataUnitHierarchy(const DKTupleValueSet& tv, V&& v1, Vs&&... vs) : Super(tv, std::forward<Vs>(vs)...), unit(std::forward<V>(v1)) {}
			DataUnitHierarchy(const DKTupleValueSet&) {}

			DataUnitHierarchy& operator = (const DataUnitHierarchy& d)
			{
				Super::operator=(d);
				unit = d.unit;
				return *this;
			}
			DataUnitHierarchy& operator = (DataUnitHierarchy&& d)
			{
				Super::operator=(static_cast<Super&&>(d));
				unit = static_cast<TupleUnit&&>(d.unit);
				return *this;
			}
			TupleUnit unit;
		};
		template <typename... Ts> struct DataUnitHierarchy<0, Ts...> // 최상위 객체
		{
			DataUnitHierarchy(void)						{}
			DataUnitHierarchy(const DKTupleValueSet&)	{}
		};
		// a type, could be recognized by index.
		template <size_t Index> using DataUnitAtIndex = DataUnitHierarchy<sizeof...(Types) - Index, Types...>;
		using DataUnitType = DataUnitHierarchy<sizeof...(Types), Types...>;

		DataUnitType dataUnits;

	public:
		enum {Length = sizeof...(Types)};
		using TypeList = DKTypeList<Types...>;

		template <size_t Index> using TypeAt = typename TypeList::template TypeAt<Index>;
		//template <typename T> using IndexOf = typename TypeList::template IndexOf<T>;
		template <typename T> struct _IndexOf
		{
			typedef typename TypeList::template IndexOf<T> Result;
		};
		template <typename T> using IndexOf = typename _IndexOf<T>::Result;

		template <typename T> static auto HasType(void)->bool
		{
			return TypeList::template Count<T>::Value > 0;
		};
		
		template <size_t Index> auto Unit(void) -> DKTupleUnit<TypeAt<Index>>&
		{
			static_assert(Index < sizeof...(Types), "Index must be lesser than type size");
			return static_cast<DataUnitAtIndex<Index>&>(dataUnits).unit;
		};
		template <size_t Index> auto Unit(void) const -> const DKTupleUnit<TypeAt<Index>>&
		{
			static_assert(Index < sizeof...(Types), "Index must be lesser than type size");
			return static_cast<const DataUnitAtIndex<Index>&>(dataUnits).unit;
		};
		template <size_t Index> auto Value(void) -> typename DKTupleUnit<TypeAt<Index>>::RefType
		{
			static_assert(Index < sizeof...(Types), "Index must be lesser than type size");
			return Unit<Index>().Value();
		};
		template <size_t Index> auto Value(void) const -> typename DKTupleUnit<TypeAt<Index>>::CRefType
		{
			static_assert(Index < sizeof...(Types), "Index must be lesser than type size");
			return Unit<Index>().Value();
		};

		template <size_t Index, typename... Ts> void SetValue(Ts&&...) {}
		template <size_t Index, typename T, typename... Ts> void SetValue(T&& v, Ts&&... vs)
		{
			static_assert(Index < sizeof...(Types)," Index must be lesser than type size");
			Unit<Index>().SetValue(std::forward<T>(v));
			SetValue<Index+1, Ts...>(std::forward<Ts>(vs)...);
		}

		DKTuple(void) {}
		DKTuple(const DKTuple& t) : dataUnits(t.dataUnits) {}
		DKTuple(DKTuple&& t) : dataUnits(static_cast<DataUnitType&&>(t.dataUnits)) {}

		// You should pass DKTupleValueSet object as first argument to use
		// initial values in constructor.
		// It makes compiler can distinguish overloaded constructor with
		// variadic-templates from other constructors.
		template <typename... Ts> DKTuple(const DKTupleValueSet& tv, Ts&&... vs) : dataUnits(tv, std::forward<Ts>(vs)...)
		{
			static_assert(sizeof...(Ts) <= Length, "Invalid arguments");
		}

		DKTuple& operator = (const DKTuple& t)
		{
			dataUnits = t.dataUnits;
			return *this;
		}
		DKTuple& operator = (DKTuple&& t)
		{
			if (this != &t)
				dataUnits = static_cast<DataUnitType&&>(t.dataUnits);
			return *this;
		}
	};

	template <typename... Types> auto DKTupleMake(Types&&... vs)-> DKTuple<Types...>
	{
		return DKTuple<Types...>(DKTupleValueSet(), std::forward<Types>(vs)...);
	}
}
