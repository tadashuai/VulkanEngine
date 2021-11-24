#pragma once

namespace VE
{
	typedef enum class KeyCode : uint16_t
	{
		// From glfw3.h
		Space = 32,
		Apostrophe = 39, /* ' */
		Comma = 44, /* , */
		Minus = 45, /* - */
		Period = 46, /* . */
		Slash = 47, /* / */

		D0 = 48, /* 0 */
		D1 = 49, /* 1 */
		D2 = 50, /* 2 */
		D3 = 51, /* 3 */
		D4 = 52, /* 4 */
		D5 = 53, /* 5 */
		D6 = 54, /* 6 */
		D7 = 55, /* 7 */
		D8 = 56, /* 8 */
		D9 = 57, /* 9 */

		Semicolon = 59, /* ; */
		Equal = 61, /* = */

		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,

		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */

		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */

		/* Function keys */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,

		/* Keypad */
		KP0 = 320,
		KP1 = 321,
		KP2 = 322,
		KP3 = 323,
		KP4 = 324,
		KP5 = 325,
		KP6 = 326,
		KP7 = 327,
		KP8 = 328,
		KP9 = 329,
		KPDecimal = 330,
		KPDivide = 331,
		KPMultiply = 332,
		KPSubtract = 333,
		KPAdd = 334,
		KPEnter = 335,
		KPEqual = 336,

		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	} Key;

	inline std::ostream& operator<<( std::ostream& os, KeyCode keyCode )
	{
		os << static_cast< int32_t >( keyCode );
		return os;
	}
}

// From glfw3.h
#define VE_KEY_SPACE           ::VE::Key::Space
#define VE_KEY_APOSTROPHE      ::VE::Key::Apostrophe    /* ' */
#define VE_KEY_COMMA           ::VE::Key::Comma         /* , */
#define VE_KEY_MINUS           ::VE::Key::Minus         /* - */
#define VE_KEY_PERIOD          ::VE::Key::Period        /* . */
#define VE_KEY_SLASH           ::VE::Key::Slash         /* / */
#define VE_KEY_0               ::VE::Key::D0
#define VE_KEY_1               ::VE::Key::D1
#define VE_KEY_2               ::VE::Key::D2
#define VE_KEY_3               ::VE::Key::D3
#define VE_KEY_4               ::VE::Key::D4
#define VE_KEY_5               ::VE::Key::D5
#define VE_KEY_6               ::VE::Key::D6
#define VE_KEY_7               ::VE::Key::D7
#define VE_KEY_8               ::VE::Key::D8
#define VE_KEY_9               ::VE::Key::D9
#define VE_KEY_SEMICOLON       ::VE::Key::Semicolon     /* ; */
#define VE_KEY_EQUAL           ::VE::Key::Equal         /* = */
#define VE_KEY_A               ::VE::Key::A
#define VE_KEY_B               ::VE::Key::B
#define VE_KEY_C               ::VE::Key::C
#define VE_KEY_D               ::VE::Key::D
#define VE_KEY_E               ::VE::Key::E
#define VE_KEY_F               ::VE::Key::F
#define VE_KEY_G               ::VE::Key::G
#define VE_KEY_H               ::VE::Key::H
#define VE_KEY_I               ::VE::Key::I
#define VE_KEY_J               ::VE::Key::J
#define VE_KEY_K               ::VE::Key::K
#define VE_KEY_L               ::VE::Key::L
#define VE_KEY_M               ::VE::Key::M
#define VE_KEY_N               ::VE::Key::N
#define VE_KEY_O               ::VE::Key::O
#define VE_KEY_P               ::VE::Key::P
#define VE_KEY_Q               ::VE::Key::Q
#define VE_KEY_R               ::VE::Key::R
#define VE_KEY_S               ::VE::Key::S
#define VE_KEY_T               ::VE::Key::T
#define VE_KEY_U               ::VE::Key::U
#define VE_KEY_V               ::VE::Key::V
#define VE_KEY_W               ::VE::Key::W
#define VE_KEY_X               ::VE::Key::X
#define VE_KEY_Y               ::VE::Key::Y
#define VE_KEY_Z               ::VE::Key::Z
#define VE_KEY_LEFT_BRACKET    ::VE::Key::LeftBracket   /* [ */
#define VE_KEY_BACKSLASH       ::VE::Key::Backslash     /* \ */
#define VE_KEY_RIGHT_BRACKET   ::VE::Key::RightBracket  /* ] */
#define VE_KEY_GRAVE_ACCENT    ::VE::Key::GraveAccent   /* ` */
#define VE_KEY_WORLD_1         ::VE::Key::World1        /* non-US #1 */
#define VE_KEY_WORLD_2         ::VE::Key::World2        /* non-US #2 */

/* Function keys */
#define VE_KEY_ESCAPE          ::VE::Key::Escape
#define VE_KEY_ENTER           ::VE::Key::Enter
#define VE_KEY_TAB             ::VE::Key::Tab
#define VE_KEY_BACKSPACE       ::VE::Key::Backspace
#define VE_KEY_INSERT          ::VE::Key::Insert
#define VE_KEY_DELETE          ::VE::Key::Delete
#define VE_KEY_RIGHT           ::VE::Key::Right
#define VE_KEY_LEFT            ::VE::Key::Left
#define VE_KEY_DOWN            ::VE::Key::Down
#define VE_KEY_UP              ::VE::Key::Up
#define VE_KEY_PAGE_UP         ::VE::Key::PageUp
#define VE_KEY_PAGE_DOWN       ::VE::Key::PageDown
#define VE_KEY_HOME            ::VE::Key::Home
#define VE_KEY_END             ::VE::Key::End
#define VE_KEY_CAPS_LOCK       ::VE::Key::CapsLock
#define VE_KEY_SCROLL_LOCK     ::VE::Key::ScrollLock
#define VE_KEY_NUM_LOCK        ::VE::Key::NumLock
#define VE_KEY_PRINT_SCREEN    ::VE::Key::PrintScreen
#define VE_KEY_PAUSE           ::VE::Key::Pause
#define VE_KEY_F1              ::VE::Key::F1
#define VE_KEY_F2              ::VE::Key::F2
#define VE_KEY_F3              ::VE::Key::F3
#define VE_KEY_F4              ::VE::Key::F4
#define VE_KEY_F5              ::VE::Key::F5
#define VE_KEY_F6              ::VE::Key::F6
#define VE_KEY_F7              ::VE::Key::F7
#define VE_KEY_F8              ::VE::Key::F8
#define VE_KEY_F9              ::VE::Key::F9
#define VE_KEY_F10             ::VE::Key::F10
#define VE_KEY_F11             ::VE::Key::F11
#define VE_KEY_F12             ::VE::Key::F12
#define VE_KEY_F13             ::VE::Key::F13
#define VE_KEY_F14             ::VE::Key::F14
#define VE_KEY_F15             ::VE::Key::F15
#define VE_KEY_F16             ::VE::Key::F16
#define VE_KEY_F17             ::VE::Key::F17
#define VE_KEY_F18             ::VE::Key::F18
#define VE_KEY_F19             ::VE::Key::F19
#define VE_KEY_F20             ::VE::Key::F20
#define VE_KEY_F21             ::VE::Key::F21
#define VE_KEY_F22             ::VE::Key::F22
#define VE_KEY_F23             ::VE::Key::F23
#define VE_KEY_F24             ::VE::Key::F24
#define VE_KEY_F25             ::VE::Key::F25

/* Keypad */
#define VE_KEY_KP_0            ::VE::Key::KP0
#define VE_KEY_KP_1            ::VE::Key::KP1
#define VE_KEY_KP_2            ::VE::Key::KP2
#define VE_KEY_KP_3            ::VE::Key::KP3
#define VE_KEY_KP_4            ::VE::Key::KP4
#define VE_KEY_KP_5            ::VE::Key::KP5
#define VE_KEY_KP_6            ::VE::Key::KP6
#define VE_KEY_KP_7            ::VE::Key::KP7
#define VE_KEY_KP_8            ::VE::Key::KP8
#define VE_KEY_KP_9            ::VE::Key::KP9
#define VE_KEY_KP_DECIMAL      ::VE::Key::KPDecimal
#define VE_KEY_KP_DIVIDE       ::VE::Key::KPDivide
#define VE_KEY_KP_MULTIPLY     ::VE::Key::KPMultiply
#define VE_KEY_KP_SUBTRACT     ::VE::Key::KPSubtract
#define VE_KEY_KP_ADD          ::VE::Key::KPAdd
#define VE_KEY_KP_ENTER        ::VE::Key::KPEnter
#define VE_KEY_KP_EQUAL        ::VE::Key::KPEqual

#define VE_KEY_LEFT_SHIFT      ::VE::Key::LeftShift
#define VE_KEY_LEFT_CONTROL    ::VE::Key::LeftControl
#define VE_KEY_LEFT_ALT        ::VE::Key::LeftAlt
#define VE_KEY_LEFT_SUPER      ::VE::Key::LeftSuper
#define VE_KEY_RIGHT_SHIFT     ::VE::Key::RightShift
#define VE_KEY_RIGHT_CONTROL   ::VE::Key::RightControl
#define VE_KEY_RIGHT_ALT       ::VE::Key::RightAlt
#define VE_KEY_RIGHT_SUPER     ::VE::Key::RightSuper
#define VE_KEY_MENU            ::VE::Key::Menu

#define VE_MOUSE_BUTTON_LEFT    0
#define VE_MOUSE_BUTTON_RIGHT   1
#define VE_MOUSE_BUTTON_MIDDLE  2
