#pragma once

#include <yae/types.h>
#include <yae/math_types.h>

#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace yae {

class InputSystem
{
public:
	void init(GLFWwindow* _window);
	void update();
	void shutdown();

	bool wasKeyJustPressed(int _key) const;
	bool wasKeyJustReleased(int _key) const;
	bool isKeyDown(int _key) const;

	bool isGamepadConnected(int _gamepadId);
	bool wasGamepadButtonJustPressed(int _gamepadId, int _button) const;
	bool wasGamepadButtonJustReleased(int _gamepadId, int _button) const;
	bool isGamepadButtonDown(int _gamepadId, int _button) const;
	float getGamepadAxisValue(int _gamepadId, int _axis) const;
	float getGamepadAxisDelta(int _gamepadId, int _axis) const;

	bool isCtrlDown() const;
	bool isShiftDown() const;
	bool isAltDown() const;

	Vector2 getMousePosition() const;
	Vector2 getMouseDelta() const;
	Vector2 getMouseScrollDelta() const;
	bool wasMouseButtonJustPressed(int _button) const;
	bool wasMouseButtonJustReleased(int _button) const;
	bool isMouseButtonDown(int _button) const;

	void notifyKeyEvent(int _key, int _scancode, int _action, int _mods);
	void notifyMouseButtonEvent(int _button, int _action, int _mods);
	void notifyMouseScrollEvent(double _xOffset, double _yOffset);

private:

	struct KeyState
	{
		bool down;
		u8 changesCount;
	};
	struct AxisState
	{
		float value;
		float delta;
	};
	struct GamepadState
	{
		KeyState buttonStates[GLFW_GAMEPAD_BUTTON_LAST + 1];
		AxisState axisStates[GLFW_GAMEPAD_AXIS_LAST + 1];
		i8 glfwGamepadId;
	};

	GLFWwindow* m_window = nullptr;

	KeyState m_keyStates[GLFW_KEY_LAST + 1];
	GamepadState m_gamepadStates[GLFW_JOYSTICK_LAST + 1];
	int m_glfwGamepadToGamepad[GLFW_JOYSTICK_LAST + 1];

	KeyState m_mouseButtonStates[GLFW_MOUSE_BUTTON_LAST + 1];
	AxisState m_mouseXAxis;
	AxisState m_mouseYAxis;
	Vector2 m_mouseScrollDelta = Vector2::ZERO;
};

} // namespace yae
