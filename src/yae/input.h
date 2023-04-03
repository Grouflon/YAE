#pragma once

#include <yae/types.h>
#include <yae/math_types.h>

#include <SDL_scancode.h>
#include <SDL_gamecontroller.h>
#include <SDL_mouse.h>

struct SDL_Window;
typedef union SDL_Event SDL_Event;

#define YAE_MAX_GAMEPAD_COUNT 16
#define YAE_MOUSE_BUTTON_COUNT 8

namespace yae {

enum CursorMode
{
	CURSORMODE_NORMAL,
	CURSORMODE_HIDDEN,
	CURSORMODE_LOCKED
};

class YAE_API InputSystem
{
public:
	void init(SDL_Window* _window);
	void beginFrame();
	void processEvent(SDL_Event& _event);
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

	void notifyKeyEvent(int _scancode, int _action, int _mods);
	void notifyMouseButtonEvent(int _button, int _action, int _mods);
	void notifyMouseScrollEvent(double _xOffset, double _yOffset);
	void notifyMouseMotionEvent(int _x, int _xDelta, int _y, int _yDelta);

	CursorMode getCursorMode() const;
	void setCursorMode(CursorMode _mode);

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
		KeyState buttonStates[SDL_CONTROLLER_BUTTON_MAX];
		AxisState axisStates[SDL_CONTROLLER_AXIS_MAX];
		i8 glfwGamepadId;
	};

	SDL_Window* m_window = nullptr;

	KeyState m_keyStates[SDL_NUM_SCANCODES];
	GamepadState m_gamepadStates[YAE_MAX_GAMEPAD_COUNT];
	int m_glfwGamepadToGamepad[YAE_MAX_GAMEPAD_COUNT];

	KeyState m_mouseButtonStates[YAE_MOUSE_BUTTON_COUNT];
	AxisState m_mouseXAxis;
	AxisState m_mouseYAxis;
	Vector2 m_mouseScrollDelta = Vector2::ZERO;

	CursorMode m_cursorMode = CURSORMODE_NORMAL;
};

} // namespace yae
