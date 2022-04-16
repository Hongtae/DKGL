//
//  File: DKCallableRef.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2021 Hongtae Kim. All rights reserved.
//

#pragma once
#include "../DKInclude.h"
#include "DKObject.h"
#include "DKFunction.h"

namespace DKFoundation
{
    template <typename Function>
    class DKCallableRef
    {
    public:
        using TypeTraits = DKTypeTraits<Function>;
        static_assert(TypeTraits::IsFunction, "Type is not a function!");

        using Signature = DKFunctionSignature<Function>;

        DKCallableRef(Signature* s = nullptr) : fn(s) {}
        DKCallableRef(const DKCallableRef& r) : fn(r.fn) {}
        DKCallableRef(const DKObject<Signature>& s) : fn(s) {}

        template <typename T> inline constexpr
        static bool isFunctionType =
            DKFunctionTest<T>::IsCallable &&
            std::is_base_of_v<std::remove_reference_t<T>, DKCallableRef> == 0;

        template <typename Fn>
        DKCallableRef(Fn&& f, std::enable_if_t<isFunctionType<Fn>, int> = 0)
            : fn(DKFunction(std::forward<Fn>(f))) {}

        template <typename T, typename Fn>
        DKCallableRef(T&& obj, Fn&& f)
            : fn(DKFunction(std::forward<T>(obj), std::forward<Fn>(f))) {}

        Signature* operator -> ()               { return fn; }
        const Signature* operator -> () const   { return fn; }
        Signature& operator * ()                { return *fn; }
        const Signature& operator * () const    { return *fn; }

        operator Signature* () { return fn; }
        operator const Signature* () const { return fn; }

        operator bool () const { return fn != nullptr; }

    private:
        DKObject<DKFunctionSignature<Function>> fn;
    };
}
