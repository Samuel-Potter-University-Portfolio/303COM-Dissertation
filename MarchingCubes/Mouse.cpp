#include "Mouse.h"

#include <glfw3.h>


void Mouse::UpdateStates()
{
	std::memcpy(m_prevStates, m_states, sizeof(m_prevStates));

	// Update button states
	for (uint16 b = 0; b < Button::Last; ++b)
	{
		const int32 state = glfwGetMouseButton(m_glfwWindow, b);
		if (state == GLFW_PRESS)
			m_states[b] = ButtonState::Pressed;
		else if (state == GLFW_RELEASE)
			m_states[b] = ButtonState::Released;
		else if (state == GLFW_REPEAT)
			m_states[b] = ButtonState::Repeating;
	}

	// Update mouse info
	double px, py;
	glfwGetCursorPos(m_glfwWindow, &px, &py);

	m_velocity.x = px - m_location.x;
	m_velocity.y = py - m_location.y;
	m_location.x = px;
	m_location.y = py;
}


void Mouse::SetGrabbed(const bool& grabbed)
{
	if (bIsGrabbed == grabbed)
		return;

	if (grabbed)
		glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	bIsGrabbed = grabbed;
}