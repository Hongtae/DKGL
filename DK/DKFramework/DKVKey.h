//
//  File: DKVKey.h
//  Author: Hongtae Kim (tiff2766@gmail.com)
//
//  Copyright (c) 2004-2014 Hongtae Kim. All rights reserved.
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// DKVirtualKey
// virtual key map.
// keyboard event can contains key-info mapped with Virtual-Key.
// this Virtual-Key make easy to build cross-platform.
////////////////////////////////////////////////////////////////////////////////

namespace DKFramework
{
	enum DKVirtualKey
	{
		DKVK_NONE = 0,

		DKVK_ESCAPE,

		DKVK_F1,
		DKVK_F2,
		DKVK_F3,
		DKVK_F4,
		DKVK_F5,
		DKVK_F6,
		DKVK_F7,
		DKVK_F8,
		DKVK_F9,
		DKVK_F10,
		DKVK_F11,
		DKVK_F12,
		DKVK_F13,
		DKVK_F14,
		DKVK_F15,
		DKVK_F16,
		DKVK_F17,
		DKVK_F18,
		DKVK_F19,
		DKVK_F20,

		DKVK_0,
		DKVK_1,
		DKVK_2,
		DKVK_3,
		DKVK_4,
		DKVK_5,
		DKVK_6,
		DKVK_7,
		DKVK_8,
		DKVK_9,

		DKVK_A,
		DKVK_B,
		DKVK_C,
		DKVK_D,
		DKVK_E,
		DKVK_F,
		DKVK_G,
		DKVK_H,
		DKVK_I,
		DKVK_J,
		DKVK_K,
		DKVK_L,
		DKVK_M,
		DKVK_N,
		DKVK_O,
		DKVK_P,
		DKVK_Q,
		DKVK_R,
		DKVK_S,
		DKVK_T,
		DKVK_U,
		DKVK_V,
		DKVK_W,
		DKVK_X,
		DKVK_Y,
		DKVK_Z,

		DKVK_PERIOD,
		DKVK_COMMA,
		DKVK_SLASH,
		DKVK_TAB,
		DKVK_ACCENT_TILDE,
		DKVK_BACKSPACE,
		DKVK_SEMICOLON,
		DKVK_QUOTE,
		DKVK_BACKSLASH,
		DKVK_EQUAL,
		DKVK_HYPHEN,
		DKVK_SPACE,
		DKVK_OPEN_BRACKET,
		DKVK_CLOSE_BRACKET,
		DKVK_CAPSLOCK,

		DKVK_RETURN,

		DKVK_FN,
		DKVK_INSERT,
		DKVK_HOME,
		DKVK_PAGE_UP,
		DKVK_PAGE_DOWN,
		DKVK_END,
		DKVK_DELETE,

		DKVK_LEFT,
		DKVK_RIGHT,
		DKVK_UP,
		DKVK_DOWN,

		DKVK_LEFT_SHIFT,
		DKVK_RIGHT_SHIFT,
		DKVK_LEFT_OPTION,
		DKVK_RIGHT_OPTION,
		DKVK_LEFT_CONTROL,
		DKVK_RIGHT_CONTROL,
		DKVK_LEFT_COMMAND,
		DKVK_RIGHT_COMMAND,

		DKVK_PAD_0,
		DKVK_PAD_1,
		DKVK_PAD_2,
		DKVK_PAD_3,
		DKVK_PAD_4,
		DKVK_PAD_5,
		DKVK_PAD_6,
		DKVK_PAD_7,
		DKVK_PAD_8,
		DKVK_PAD_9,
		DKVK_ENTER,
		DKVK_NUMLOCK,
		DKVK_PAD_SLASH,
		DKVK_PAD_ASTERISK,
		DKVK_PAD_PLUS,
		DKVK_PAD_MINUS,
		DKVK_PAD_EQUAL,
		DKVK_PAD_PERIOD,

		DKVK_MAXVALUE,
	};
}
