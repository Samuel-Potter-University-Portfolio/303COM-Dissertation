#include "Keyboard.h"

#include <glfw3.h>


void Keyboard::UpdateStates()
{
	std::memcpy(m_prevStates, m_states, sizeof(m_prevStates));

	for (uint16 k = 32; k < Key::Last; ++k)
	{
		const int32 state = glfwGetKey(m_glfwWindow, k);
		if (state == GLFW_PRESS)
			m_states[k] = KeyState::Pressed;
		else if (state == GLFW_RELEASE)
			m_states[k] = KeyState::Released;
		else if (state == GLFW_REPEAT)
			m_states[k] = KeyState::Repeating;
	}
}