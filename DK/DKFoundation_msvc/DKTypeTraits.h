//
//  File: DKTypeTraits.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKTypes.h"
#include "DKTypeList.h"

///////////////////////////////////////////////////////////////////////////////
// DKTypeTraits
// a template type traits class.
///////////////////////////////////////////////////////////////////////////////

namespace DKFoundation
{
	// type traits of global function pointer.
	template <typename> struct DKFunctionPointerTraits
	{
		enum {IsFunctionPointer = false};
	};
	template <typename R, typename... Args> struct DKFunctionPointerTraits<R(*)(Args...)>
	{
		enum {IsFunctionPointer = true};
		enum {HasVariadicParameter = false};
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args...);
		typedef R GenericType(Args...);
		using PointerType = R (*)(Args...);
	};
	template <typename R, typename... Args> struct DKFunctionPointerTraits<R(*)(Args..., ...)>
	{
		enum {IsFunctionPointer = true};
		enum {HasVariadicParameter = true};
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args..., ...);
		typedef R GenericType(Args..., ...);
		using PointerType = R (*)(Args..., ...);
	};
	// type traits of class member functions.
	template <typename> struct DKMemberFunctionPointerTraits
	{
		enum {IsFunctionPointer = false};
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args...)>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = false};
		enum {IsVolatile = false};
		enum {HasVariadicParameter = false};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args...);
		typedef R GenericType(Args...);
		using PointerType = R (ClassType::*)(Args...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args..., ...)>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = false};
		enum {IsVolatile = false};
		enum {HasVariadicParameter = true};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args..., ...);
		typedef R GenericType(Args..., ...);
		using PointerType = R (ClassType::*)(Args..., ...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args...) const>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = true};
		enum {IsVolatile = false};
		enum {HasVariadicParameter = false};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args...);
		typedef R GenericType(Args...);
		using PointerType = R (ClassType::*)(Args...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args..., ...) const>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = true};
		enum {IsVolatile = false};
		enum {HasVariadicParameter = true};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args..., ...);
		typedef R GenericType(Args..., ...);
		using PointerType = R (ClassType::*)(Args..., ...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args...) volatile>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = false};
		enum {IsVolatile = true};
		enum {HasVariadicParameter = false};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args...);
		typedef R GenericType(Args...);
		using PointerType = R (ClassType::*)(Args...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args..., ...) volatile>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = false};
		enum {IsVolatile = true};
		enum {HasVariadicParameter = true};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args..., ...);
		typedef R GenericType(Args..., ...);
		using PointerType = R (ClassType::*)(Args..., ...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args...) const volatile>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = true};
		enum {IsVolatile = true};
		enum {HasVariadicParameter = false};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args...);
		typedef R GenericType(Args...);
		using PointerType = R (ClassType::*)(Args...);
	};
	template <typename R, class C, typename... Args> struct DKMemberFunctionPointerTraits<R(C::*)(Args..., ...) const volatile>
	{
		enum {IsFunctionPointer = true};
		enum {IsConst = true};
		enum {IsVolatile = true};
		enum {HasVariadicParameter = true};
		using ClassType = C;
		using ReturnType = R;
		using ParameterTypeList = DKTypeList<Args...>;
		//using GenericType = R (Args..., ...);
		typedef R GenericType(Args..., ...);
		using PointerType = R (ClassType::*)(Args..., ...);
	};

	template <typename T> class DKTypeTraits : public DKTypeTraitsCppExt<T>
	{
		template <typename U> struct ReferenceTraits
		{
			enum {Result = false};
			enum {IsLValueRef = false};
			enum {IsRValueRef = false};
			using ReferredType = U;
		};
		template <typename U> struct ReferenceTraits<U&>
		{
			enum {Result = true};
			enum {IsLValueRef = true};
			enum {IsRValueRef = false};
			using ReferredType = U;
		};
		template <typename U> struct ReferenceTraits<U&&>
		{
			enum {Result = true};
			enum {IsLValueRef = false};
			enum {IsRValueRef = true};
			using ReferredType = U;
		};
        template <typename U> struct PointerTraits
        {
			enum {Result = false};
			using PointeeType = U;
        };
        template <typename U> struct PointerTraits<U*>
        {
			enum {Result = true};
			using PointeeType = U;
        };
        template <typename U> struct PointerTraits<U*&>
        {
			enum {Result = true};
			using PointeeType = U;
        };
        template <typename U> struct PointerTraits<U*&&>
        {
			enum {Result = true};
			using PointeeType = U;
        };
        template <typename U> struct MemberPointerTraits
		{
			enum {Result = false};
		};
        template <typename U, typename V> struct MemberPointerTraits<U V::*>
		{
			enum {Result = true};
		};
        template <typename U, typename V> struct MemberPointerTraits<U V::*&>
		{
			enum {Result = true};
		};
        template <typename U, typename V> struct MemberPointerTraits<U V::*&&>
		{
			enum {Result = true};
		};
        template <typename U> struct UnConst
        {
            enum {IsConst = false};
			using Type = U;
        };
        template <typename U> struct UnConst<const U>
        {
            enum {IsConst = true};
			using Type = U;
        };
        template <typename U> struct UnConst<const U&>
        {
            enum {IsConst = true};
			using Type = U&;
        };
        template <typename U> struct UnConst<const U&&>
        {
            enum {IsConst = true};
			using Type = U&&;
        };
        template <typename U> struct UnVolatile
        {
            enum {IsVolatile = false};
			using Type = U;
        };
        template <typename U> struct UnVolatile<volatile U>
        {
            enum {IsVolatile = true};
			using Type = U;
        };
        template <typename U> struct UnVolatile<volatile U&>
        {
            enum {IsVolatile = true};
			using Type = U&;
        };
        template <typename U> struct UnVolatile<volatile U&&>
        {
            enum {IsVolatile = true};
			using Type = U&&;
        };

		template <typename U> struct AddPointer			{using Type = U*;};
        template <typename U> struct AddPointer<U&>		{using Type = U*;};
        template <typename U> struct AddPointer<U&&>	{using Type = U*;};
        template <typename U> struct AddReference		{using Type = U&;};
        template <typename U> struct AddReference<U&>	{using Type = U&;};
        template <typename U> struct AddReference<U&&>	{using Type = U&&;};

	public:
		using NonConstType				= typename UnConst<T>::Type;
		using NonVolatileType			= typename UnVolatile<T>::Type;
		using UnqualifiedType			= typename UnVolatile<typename UnConst<T>::Type>::Type;
		using PointeeType				= typename PointerTraits<UnqualifiedType>::PointeeType;
		using ReferredType				= typename ReferenceTraits<T>::ReferredType;
		using UnqualifiedReferredType	= typename ReferenceTraits<UnqualifiedType>::ReferredType;

		using FunctionTraits			= DKFunctionPointerTraits<UnqualifiedReferredType>;
		using MemberFunctionTraits		= DKMemberFunctionPointerTraits<UnqualifiedReferredType>;

        enum {IsConst					= UnConst<T>::IsConst};
        enum {IsVolatile				= UnVolatile<T>::IsVolatile};
        enum {IsReference				= ReferenceTraits<UnqualifiedType>::Result};
		enum {IsLValueReference			= ReferenceTraits<UnqualifiedType>::IsLValueRef};
		enum {IsRValueReference			= ReferenceTraits<UnqualifiedType>::IsRValueRef};
		enum {IsFunction				= DKFunctionPointerTraits<typename AddPointer<T>::Type>::IsFunctionPointer};
		enum {IsFunctionPointer			= FunctionTraits::IsFunctionPointer};
		enum {IsMemberFunctionPointer	= MemberFunctionTraits::IsFunctionPointer};
		enum {IsMemberPointer			= MemberPointerTraits<UnqualifiedReferredType>::Result || IsMemberFunctionPointer};
		enum {IsPointer					= PointerTraits<UnqualifiedReferredType>::Result || IsFunctionPointer};
	};
}
