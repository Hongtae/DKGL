//
//  File: DKVKey.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2016 Hongtae Kim. All rights reserved.
//

#pragma once

namespace DKFramework
{
	/// @brief virtual key map.
	///
	/// keyboard event can contains key-info mapped with Virtual-Key.
	/// this Virtual-Key make easy to build cross-platform.
	enum class DKVirtualKey
	{
        None = 0,

        Escape,

        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        F13,
        F14,
        F15,
        F16,
        F17,
        F18,
        F19,
        F20,

        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,

        Period,
        Comma,
        Slash,
        Tab,
        AccentTilde,
        Backspace,
        Semicolon,
        Quote,
        Backslash,
        Equal,
        Hyphen,
        Space,
        OpenBracket,
        CloseBracket,
        Capslock,

        Return,

        Fn,
        Insert,
        Home,
        PageUp,
        PageDown,
        End,
        Delete,

        Left,
        Right,
        Up,
        Down,

        LeftShift,
        RightShift,
        LeftOption,
        RightOption,
        LeftControl,
        RightControl,
        LeftCommand,
        RightCommand,

        Pad0,
        Pad1,
        Pad2,
        Pad3,
        Pad4,
        Pad5,
        Pad6,
        Pad7,
        Pad8,
        Pad9,
        Enter,
        Numlock,
        PadSlash,
        PadAsterisk,
        PadPlus,
        PadMinus,
        PadEqual,
        PadPeriod,

        MaxValue, ///< for counting number of items in enum
	};
}
