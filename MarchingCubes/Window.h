#pragma once
#include "Common.h"

#include "Keyboard.h"
#include "Mouse.h";

#include <functional>
#include <GL\glew.h>


typedef std::function<void(class Window& source, const float& deltaTime)> WindowCallback;


/**
* Initialization settings for a window
*/
struct WindowInit
{
	uint32	Major = 3;
	uint32	Minor = 2;
	bool	bForwardCompatibility = true;

	uint32	Width = 800;
	uint32	Height = 600;
	string	Title = "Window";

	bool	bVerticalSync = false;
};


/**
* Container for opengl (GLFW) window
*/
class Window
{
private:
	///
	/// Window varss
	///
	struct GLFWwindow* m_glfwWindow = nullptr;
	int32 m_width;
	int32 m_height;
	float m_aspectRatio;
	int32 m_prevWidth;
	int32 m_prevHeight;

	///
	/// Sub-controllers
	///
	Keyboard m_keyboard;
	Mouse m_mouse;


public:
	/**
	* Initialized everything that is needed for the API (OpenGL/GLEW/GLEW)
	* @returns If API initialized correctly
	*/
	static bool InitAPI();
	/**
	* Destroys everything created by the API correctly
	*/
	static void DestroyAPI();


	/**
	* Attemps to open a window with these settings
	* @param settings			The OpenGL & Window settings to use
	* @returns If the window openned succesfully
	*/
	bool Open(const WindowInit& settings);

	/**
	* Launches this window into it's main loop
	* (Must be from the same thread it was openned in)
	* @param callback			Callback function which will be called once per framce
	*/
	void LaunchMainLoop(WindowCallback callback);


	///
	/// Getters & Setters
	///
public:
	inline int32 GetWidth() const { return m_width; }
	inline int32 GetHeight() const { return m_height; }
	inline float GetAspectRatio() const { return m_aspectRatio; }

	inline Keyboard& GetKeyboard() { return m_keyboard; }
	inline Mouse& GetMouse() { return m_mouse; }
};

