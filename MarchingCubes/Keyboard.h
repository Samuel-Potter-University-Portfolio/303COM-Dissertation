#pragma once
#include "Common.h"


/**
* Stores and processes keyboard input
*/
class Keyboard
{
public:
	///
	/// Supported key code
	///
	enum Key : uint16
	{
		KV_Unknown = 0,

		KV_SPACE = 32,
		KV_APOSTROPHE = 39,
		KV_COMMA = 44,
		KV_MINUS = 45,
		KV_PERIOD = 46,
		KV_SLASH = 47,

		KV_0 = 48,
		KV_1 = 49,
		KV_2 = 50,
		KV_3 = 51,
		KV_4 = 52,
		KV_5 = 53,
		KV_6 = 54,
		KV_7 = 55,
		KV_8 = 56,
		KV_9 = 57,

		KV_SEMICOLON = 59,
		KV_EQUAL = 61,

		KV_A = 65,
		KV_B = 66,
		KV_C = 67,
		KV_D = 68,
		KV_E = 69,
		KV_F = 70,
		KV_G = 71,
		KV_H = 72,
		KV_I = 73,
		KV_J = 74,
		KV_K = 75,
		KV_L = 76,
		KV_M = 77,
		KV_N = 78,
		KV_O = 79,
		KV_P = 80,
		KV_Q = 81,
		KV_R = 82,
		KV_S = 83,
		KV_T = 84,
		KV_U = 85,
		KV_V = 86,
		KV_W = 87,
		KV_X = 88,
		KV_Y = 89,
		KV_Z = 90,


		KV_ESCAPE = 256,
		KV_ENTER = 257,
		KV_TAB = 258,
		KV_BACKSPACE = 259,
		KV_INSERT = 259,
		KV_DELETE = 261,

		KV_RIGHT = 262,
		KV_LEFT = 263,
		KV_DOWN = 265,
		KV_UP = 266,


		KV_F1 = 290,
		KV_F2 = 291,
		KV_F3 = 292,
		KV_F4 = 293,
		KV_F5 = 294,
		KV_F6 = 295,
		KV_F7 = 296,
		KV_F8 = 297,
		KV_F9 = 298,
		KV_F10 = 299,
		KV_F11 = 300,
		KV_F12 = 301,


		KV_LSHIFT = 340,
		KV_LCONTROL = 341,
		KV_LALT = 342,
		KV_LSUPER = 343,
		KV_RSHIFT = 344,
		KV_RCONTROL = 345,
		KV_RALT = 346,
		KV_RSUPER = 347,


		Last
	};

private:
	enum KeyState : uint8
	{
		Released,
		Pressed,
		Repeating
	};

	struct GLFWwindow* m_glfwWindow;
	KeyState m_states[Key::Last]{ Released };
	KeyState m_prevStates[Key::Last]{ Released };

private:
	///
	/// Window update callback
	///
	friend class Window;

	/**
	* Link the keyboard to this window
	* @param glfwWindow		The window to read update from
	*/
	inline void Link(struct GLFWwindow* glfwWindow) { m_glfwWindow = glfwWindow; }

	/** Update the currently stored key states */
	void UpdateStates();


	///
	/// Getters & Setters
	///
public:
	/// Is key pressed
	inline bool IsKeyDown(const Key& key) const { return m_states[key] != KeyState::Released; }
	/// Is key not being pressed
	inline bool IsKeyUp(const Key& key) const { return m_states[key] == KeyState::Released; }

	/// Was the key pressed this frame
	inline bool IsKeyPressed(const Key& key) const { return m_states[key] != KeyState::Released && m_prevStates[key] == KeyState::Released; }
	/// Was the key released this frame
	inline bool IsKeyReleased(const Key& key) const { return m_states[key] == KeyState::Released && m_prevStates[key] != KeyState::Released; }
};

