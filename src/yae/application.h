#pragma once

#include <yae/types.h>
#include <yae/math_types.h>
#include <yae/time.h>

struct GLFWwindow;

namespace Im3d
{
	struct Context;
}

namespace yae {

class Renderer;
class InputSystem;
class ImGuiSystem;

class YAELIB_API Application
{
public:
	Application(const char* _name, u32 _width, u32 _height);

	void init(char** _args, int _argCount);
	bool doFrame();
	void run();
	void shutdown();

	void requestExit();

	InputSystem& input() const;
	Renderer& renderer() const;

	Vector3 getCameraPosition() const { return m_cameraPosition; }
	Quaternion getCameraRotation() const { return m_cameraRotation; }
	void setCameraPosition(const Vector3& _position) { m_cameraPosition = _position; }
	void setCameraRotation(const Quaternion& _rotation) { m_cameraRotation = _rotation; }

	void* getUserData() const { return m_userData; }
	void setUserData(void* _userData) { m_userData = _userData; }

private:

	static void _glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void _glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void _glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void _glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	String m_name;
	u32 m_width = 0;
	u32 m_height = 0;

	InputSystem* m_inputSystem = nullptr;
	ImGuiSystem* m_imGuiSystem = nullptr;
	Renderer* m_renderer = nullptr;

	GLFWwindow* m_window = nullptr;
	Im3d::Context* m_im3d = nullptr;

	Clock m_clock;

	Vector3 m_cameraPosition = Vector3::ZERO;
	Quaternion m_cameraRotation = Quaternion::IDENTITY;
	void* m_userData = nullptr;

	bool m_showMemoryProfiler = false;
	bool m_showFrameRate = false;
};

} // namespace yae
