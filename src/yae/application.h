#pragma once

#include <yae/types.h>
#include <yae/string.h>
#include <yae/time.h>
#include <yae/filesystem.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class VulkanRenderer;
class InputSystem;

class YAELIB_API Application
{
public:
	Application(const char* _name, u32 _width, u32 _height);

	void init(char** _args, int _argCount);
	bool doFrame();
	void run();
	void shutdown();

	InputSystem& input() const;
	VulkanRenderer& renderer() const;

private:

	static void _glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void _glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void _glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void _glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	String m_name;
	u32 m_width = 0;
	u32 m_height = 0;

	InputSystem* m_inputSystem = nullptr;
	VulkanRenderer* m_vulkanRenderer = nullptr;

	GLFWwindow* m_window = nullptr;
	ImGuiContext* m_imgui = nullptr;

	Clock m_clock;

	bool m_showMemoryProfiler = false;
};

} // namespace yae
