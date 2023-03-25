#pragma once

#include <yae/types.h>
#include <yae/time.h>
#include <yae/math_types.h>
#include <yae/containers/HashMap.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class Renderer;
class InputSystem;
class Serializer;

class YAE_API Application
{
public:
	Application(const char* _name, u32 _width, u32 _height);
	~Application();

	void init(char** _args, int _argCount);
	void shutdown();
	bool doFrame();

	void run();

	void requestExit();

	const char* getName() const;

	InputSystem& input() const;
	Renderer& renderer() const;
	ResourceManager& resourceManager() const;

	void* getUserData(const char* _name) const;
	void setUserData(const char* _name, void* _userData);

	void loadSettings();
	void saveSettings();

	float getDeltaTime() const;
	float getTime() const;

//private:
	static void _glfw_windowPosCallback(GLFWwindow* _window, int _x, int _y);
	static void _glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void _glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void _glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void _glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	void _requestSaveSettings();

	String m_name;
	u32 m_baseWidth = 0;
	u32 m_baseHeight = 0;

	ResourceManager* m_resourceManager = nullptr;
	InputSystem* m_inputSystem = nullptr;
	Renderer* m_renderer = nullptr;
	ImGuiContext* m_imguiContext = nullptr;

	GLFWwindow* m_window = nullptr;

	Clock m_clock;
	float m_dt = 0.f;
	float m_time = 0.f;

	HashMap<StringHash, void*> m_userData;

	bool m_saveSettingsRequested = false;
};

} // namespace yae
