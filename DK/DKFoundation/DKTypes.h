//
//  File: DKTypes.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once
#include <type_traits>
#include <utility>
#include "../DKInclude.h"

namespace DKFoundation
{
	/// C++ compiler extensions
	/// MSC: http://msdn.microsoft.com/en-us/library/vstudio/ms177194.aspx
	/// GCC: http://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html
	/// Clang : http://clang.llvm.org/docs/LanguageExtensions.html
	template <typename T> struct DKTypeTraitsCppExt
	{
		constexpr static bool HasNothrowAssign()				{return __has_nothrow_assign(T);}
		constexpr static bool HasNothrowCopy()					{return __has_nothrow_copy(T);}
		constexpr static bool HasNothrowConstructor()			{return __has_nothrow_constructor(T);}
		constexpr static bool HasTrivialAssign()				{return __has_trivial_assign(T);}
		constexpr static bool HasTrivialCopy()					{return __has_trivial_copy(T);}
		constexpr static bool HasTrivialConstructor()			{return __has_trivial_constructor(T);}
		constexpr static bool HasTrivialDestructor()			{return __has_trivial_destructor(T);}
		constexpr static bool HasVirtualDestructor()			{return __has_virtual_destructor(T);}
		constexpr static bool IsAbstract()						{return __is_abstract(T);}
		constexpr static bool IsClass()							{return __is_class(T);}
		constexpr static bool IsEmpty()							{return __is_empty(T);}
		constexpr static bool IsEnum()							{return __is_enum(T);}
		constexpr static bool IsPod()							{return __is_pod(T);}
		constexpr static bool IsPolymorphic()					{return __is_polymorphic(T);}
		constexpr static bool IsUnion()							{return __is_union(T);}
		template <typename U> constexpr static bool IsBaseOf()	{return __is_base_of(U, T);} ///< __is_base_of(base, derived)
	};

	/// type number
	template <int i> struct DKNumber {enum {Value = i};	};
	/// boolean type value (for SFINAE)
	using DKTrue = DKNumber<1>;
	using DKFalse = DKNumber<0>;

	namespace Private
	{
		template <typename T, bool isPolymorphic = DKTypeTraitsCppExt<T>::IsPolymorphic()> struct BaseAddress;
		template <typename T> struct BaseAddress<T, true>
		{
			enum {IsPolymorphic = 1};
			static void* Cast(T* p)
			{
				return dynamic_cast<void*>(p);
			}
		};
		template <typename T> struct BaseAddress<T, false>
		{
			enum {IsPolymorphic = 0};
			static void* Cast(T* p)
			{
				return static_cast<void*>(p);
			}
		};

		// conditional type
		template <bool C, typename T, typename U> struct CondType
		{
			using Result = T;
		};
		template <typename T, typename U> struct CondType<false, T, U>
		{
			using Result = U;
		};

		// sum of given integers in Ns...
		// _Sum<1,2,3>::Result = 6
		template <int... Ns> struct Sum
		{
			enum {Result = 0};
		};
		template <int N1, int... Ns> struct Sum<N1, Ns...>
		{
			enum {Result = N1 + Sum<Ns...>::Result};
		};
		// count matching integer in numbers
		template <int... Ns> struct NumMatches
		{
			enum {Result = 0};
		};
		template <int Num, int N1, int... Ns> struct NumMatches<Num, N1, Ns...>
		{
			enum {Rest = NumMatches<Num, Ns...>::Result};
			enum {Result = Rest + (Num == N1 ? 1 : 0)};
		};

		// check T1 is convertible into T2.
		template <typename T1, typename T2> struct ConversionTest
		{
			template <typename U> struct _IsPointerOrReference		{enum {Result = false};};
			template <typename U> struct _IsPointerOrReference<U*>	{enum {Result = true};};
			template <typename U> struct _IsPointerOrReference<U&>	{enum {Result = true};};
			template <typename U> struct _IsPointerOrReference<U&&>	{enum {Result = true};};

			// Use given type if type is pointer or reference,
			// otherwise change into reference-type.
			using Type1Ref = typename CondType<_IsPointerOrReference<T1>::Result, T1, T1&>::Result;
			using Type2Ref = typename CondType<_IsPointerOrReference<T2>::Result, T2, const T2&>::Result;

			static DKTrue _Test(Type2Ref);
			static DKFalse _Test(...);
			static Type1Ref _TypeSub();

			enum {Result = decltype(_Test(_TypeSub()))::Value};
		};
		template <typename T> struct ConversionTest<T, T>		{enum {Result = true};};
		template <typename T> struct ConversionTest<void, T>	{enum {Result = false};};
		template <typename T> struct ConversionTest<T, void>	{enum {Result = false};};
		template <> struct ConversionTest<void, void>			{enum {Result = true};};

		// conditional cast object.
		// perform static_cast for convertible object (up cast)
		// perform dynamic_cast for non-convertible object (down cast)
		template <typename T, typename R, bool IsConvertible> struct SafeCaster;
		template <typename T, typename R> struct SafeCaster<T, R, true>
		{
			static R* Cast(T* p)				{ return static_cast<R*>(p); }
			static const R* Cast(const T* p)	{ return static_cast<const R*>(p); }
		};
		template <typename T, typename R> struct SafeCaster<T, R, false>
		{
			static R* Cast(T* p)				{ return dynamic_cast<R*>(p); }
			static const R* Cast(const T* p)	{ return dynamic_cast<const R*>(p); }
		};
	}

	/// conditional type definition, if C is true result is T, else U.
	template <bool C, typename T, typename U> using DKCondType = typename Private::CondType<C, T, U>::Result;

	/// returns sum of integers in Ns...
	template <int... Ns> constexpr int DKSum(void)
	{
		return Private::Sum<Ns...>::Result;
	}

	/// count number of matches with first integer(Num) in Ns
	template <int Num, int... Ns> constexpr int DKNumMatches(void)
	{
		return Private::NumMatches<Num, Ns...>::Result;
	}

	/// determine whether type Source is convertible into type Target.
	template <typename Source, typename Target> constexpr bool DKTypeConversionTest(void)
	{
		return Private::ConversionTest<Source, Target>::Result;
	}
	
	/// returns object's base address.
	/// In multiple inheritance, base address may be differ.
	template <typename T> void* DKTypeBaseAddressCast(T* p)
	{
		return Private::BaseAddress<T>::Cast(p);
	}
}
