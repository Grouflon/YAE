#pragma once

#include <yae/types.h>
#include <yae/string.h>
#include <yae/filesystem.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class ResourceManager;
class VulkanRenderer;
class InputSystem;

// @TODO: 2 notions en fait à partir de celle ci: Process/Application/Program qui est lié à l'instance unique de ce processus, et Instance/Contexte qui peut être en plusieurs exemplaires dans le programme.

class YAELIB_API Application
{
public:
	void init(const char* _name, u32 _width, u32 _height, char** _args, int _arg_count);
	void run();
	void shutdown();

	const Path& getExePath();
	const Path& getBaseDirectoryPath();

	InputSystem& getInputSystem() const;
	VulkanRenderer& getRenderer() const;

private:

	static void _glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void _glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void _glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void _glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	String m_name;

	Path m_exePath;
	Path m_baseDirectoryPath;

	InputSystem* m_inputSystem = nullptr;
	VulkanRenderer* m_vulkanRenderer = nullptr;

	GLFWwindow* m_window = nullptr;
	ImGuiContext* m_imgui = nullptr;

	bool m_showMemoryProfiler = false;
};

} // namespace yae
