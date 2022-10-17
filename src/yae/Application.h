#pragma once

#include <yae/types.h>
#include <yae/time.h>
#include <yae/math_types.h>
#include <yae/containers/HashMap.h>
#include <yae/hash.h>

struct GLFWwindow;

namespace yae {

class Renderer;
class InputSystem;
class ImGuiSystem;
class Im3dSystem;
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

	Vector3 getCameraPosition() const { return m_cameraPosition; }
	Quaternion getCameraRotation() const { return m_cameraRotation; }
	void setCameraPosition(const Vector3& _position) { m_cameraPosition = _position; }
	void setCameraRotation(const Quaternion& _rotation) { m_cameraRotation = _rotation; }

	void* getUserData(const char* _name) const;
	void setUserData(const char* _name, void* _userData);

	void loadSettings();
	void saveSettings();

private:

	static void _glfw_windowPosCallback(GLFWwindow* _window, int _x, int _y);
	static void _glfw_framebufferSizeCallback(GLFWwindow* _window, int _width, int _height);
	static void _glfw_keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void _glfw_mouseButtonCallback(GLFWwindow* _window, int _button, int _action, int _mods);
	static void _glfw_scrollCallback(GLFWwindow* _window, double _xOffset, double _yOffset);

	Matrix4 _computeCameraView() const;
	Matrix4 _computeCameraProj() const;

	String m_name;
	u32 m_baseWidth = 0;
	u32 m_baseHeight = 0;

	InputSystem* m_inputSystem = nullptr;
	ImGuiSystem* m_imGuiSystem = nullptr;
	Im3dSystem* m_im3dSystem = nullptr;
	Renderer* m_renderer = nullptr;

	GLFWwindow* m_window = nullptr;

	Clock m_clock;

	Vector3 m_cameraPosition = Vector3::ZERO;
	Quaternion m_cameraRotation = Quaternion::IDENTITY;
	float m_cameraFov = 45.f;

	HashMap<StringHash, void*> m_userData;
};

} // namespace yae
