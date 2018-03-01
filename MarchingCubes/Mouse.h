#pragma once
#include "Common.h"


/**
* Stores and processes mouse input
*/
class Mouse
{
public:
	///
	/// Supported button code
	///
	enum Button : uint16
	{
		MB_Unknown = 0,

		MB_LEFT = 0,
		MB_RIGHT = 1,
		MB_MIDDLE = 2,

		MB_ACTION0 = 3,
		MB_ACTION1 = 4,
		MB_ACTION2 = 5,
		MB_ACTION3 = 6,
		MB_ACTION4 = 7,

		Last
	};

private:
	enum ButtonState : uint8
	{
		Released,
		Pressed,
		Repeating
	};

	///
	/// Mouse vars
	///
	struct GLFWwindow* m_glfwWindow;
	ButtonState m_states[Button::Last]{ Released };
	ButtonState m_prevStates[Button::Last]{ Released };

	vec2 m_location;
	vec2 m_velocity;
	bool bIsGrabbed = false;

private:
	///
	/// Window update callback
	///
	friend class Window;

	/**
	* Link the mouse to this window
	* @param glfwWindow		The window to read update from
	*/
	inline void Link(struct GLFWwindow* glfwWindow) { m_glfwWindow = glfwWindow; }

	/** Update the currently stored key states */
	void UpdateStates();


public:
	/**
	* Set whether the cursor should be considered as grabbed or not
	* @param grabbed		Trus if cursor should be grabbed
	*/
	void SetGrabbed(const bool& grabbed);

	///
	/// Getters & Setters
	///
public:
	/// Is key pressed
	inline bool IsButtonDown(const Button& button) const { return m_states[button] != ButtonState::Released; }
	/// Is key not being pressed
	inline bool IsButtonUp(const Button& button) const { return m_states[button] == ButtonState::Released; }

	/// Was the key pressed this frame
	inline bool IsButtonPressed(const Button& button) const { return m_states[button] != ButtonState::Released && m_prevStates[button] == ButtonState::Released; }
	/// Was the key released this frame
	inline bool IsButtonReleased(const Button& button) const { return m_states[button] == ButtonState::Released && m_prevStates[button] != ButtonState::Released; }


	inline vec2 GetLocation() const { return m_location; }
	inline vec2 GetVelocity() const { return m_velocity; }
	inline bool IsGrabbed() const { return bIsGrabbed; }
};

