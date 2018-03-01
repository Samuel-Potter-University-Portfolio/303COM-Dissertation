#include "Window.h"
#include "Logger.h"

#include <glfw3.h>


bool Window::InitAPI()
{
	LOG("Initalizing OpenGL");

	glewExperimental = true;
	glfwSetErrorCallback(
		[](int error, const char* message)
	{
		LOG_ERROR("GLFW Error(%i): '%s'", error, message);
	}
	);

	if (!glfwInit())
	{
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	return true;
}

void Window::DestroyAPI()
{
	LOG("Destroying OpenGL");

	glfwTerminate();
}

bool Window::Open(const WindowInit& settings)
{
	// Set desired OpenGL context settings
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, settings.Major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, settings.Minor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, settings.bForwardCompatibility);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// Open window
	m_glfwWindow = glfwCreateWindow(settings.Width, settings.Height, settings.Title.c_str(), nullptr, nullptr);
	if (m_glfwWindow == nullptr)
	{
		glfwTerminate();
		LOG_ERROR("Failed to create GLFW Window");
		return false;
	}
	else
	{
		LOG("GLFW Window created with desired settings");
		glfwMakeContextCurrent(m_glfwWindow);
	}


	// Initialize glew
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		LOG_ERROR("GLEW Error(%i): 'Failed to initialize'", error);
		return false;
	}
	else
		LOG("GLEW intialized succesfully");


	// Output useful information
	LOG("OpenGL Vendor:");
	LOG("\t-GL_RENDERER: \t%s", glGetString(GL_RENDERER));
	LOG("\t-GL_VERSION: \t%s", glGetString(GL_VERSION));
	LOG("\t-GL_VENDOR: \t%s", glGetString(GL_VENDOR));


	// Set other settings
	glfwSwapInterval(settings.bVerticalSync);
	m_keyboard.Link(m_glfwWindow);
	m_mouse.Link(m_glfwWindow);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void Window::LaunchMainLoop(WindowCallback callback)
{
	LOG("Launching into main loop");
	float lastTime = glfwGetTime();


	while (!glfwWindowShouldClose(m_glfwWindow))
	{
		// Update controllers
		glfwPollEvents();
		m_keyboard.UpdateStates();
		m_mouse.UpdateStates();


		// Update size window if needed
		glfwGetFramebufferSize(m_glfwWindow, &m_width, &m_height);
		m_aspectRatio = (float)m_width / (float)m_height;
		if (m_width != m_prevWidth || m_height != m_prevHeight)
		{
			m_prevWidth = m_width;
			m_prevHeight = m_height;
			glViewport(0, 0, m_width, m_height);
		}


		// Render/Logic
		float frameTime = glfwGetTime();
		const float deltaTime = frameTime - lastTime;
		lastTime = frameTime;

		if (callback)
			callback(*this, deltaTime);


		glfwSwapBuffers(m_glfwWindow);
	}

	glfwDestroyWindow(m_glfwWindow);
	m_glfwWindow = nullptr;
	LOG("GLFW Window destroyed");
}