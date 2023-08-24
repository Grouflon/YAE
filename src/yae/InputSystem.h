#pragma once

#include <yae/types.h>
#include <yae/math_types.h>
#include <yae/input_types.h>

#include <SDL_scancode.h>

struct SDL_Window;
typedef union SDL_Event SDL_Event;

namespace yae {

class YAE_API InputSystem
{
public:
	void init(SDL_Window* _window);
	void beginFrame();
	void processEvent(const SDL_Event& _event);
	void update();
	void shutdown();

	bool wasKeyJustPressed(int _key) const;
	bool wasKeyJustReleased(int _key) const;
	bool isKeyDown(int _key) const;

	bool isGamepadConnected(GamepadID _gamepadId) const;
	u32 getConnectedGamepadCount() const;
	const char* getGamepadName(GamepadID _gamepadId) const;
	bool wasGamepadButtonJustPressed(GamepadID _gamepadId, GamepadButton _button) const;
	bool wasGamepadButtonJustReleased(GamepadID _gamepadId, GamepadButton _button) const;
	bool isGamepadButtonDown(GamepadID _gamepadId, GamepadButton _button) const;
	float getGamepadAxisValue(GamepadID _gamepadId, GamepadAxis _axis) const;
	float getGamepadAxisDelta(GamepadID _gamepadId, GamepadAxis _axis) const;

	bool isCtrlDown() const;
	bool isShiftDown() const;
	bool isAltDown() const;

	Vector2 getMousePosition() const;
	Vector2 getMouseDelta() const;
	Vector2 getMouseScrollDelta() const;
	bool wasMouseButtonJustPressed(int _button) const;
	bool wasMouseButtonJustReleased(int _button) const;
	bool isMouseButtonDown(int _button) const;

	CursorMode getCursorMode() const;
	void setCursorMode(CursorMode _mode);

//private:
	GamepadID _gamepadInstanceIdToGamepadId(i32 _instanceId) const;

	void _notifyKeyEvent(int _scancode, int _action, int _mods);
	void _notifyMouseButtonEvent(MouseButton _button, int _action, int _mods);
	void _notifyMouseScrollEvent(double _xOffset, double _yOffset);
	void _notifyMouseMotionEvent(int _x, int _xDelta, int _y, int _yDelta);
	void _notifyGamepadButtonEvent(GamepadID _gamepadId, GamepadButton _button, int _action);
	void _notifyGamepadAxisEvent(GamepadID _gamepadId, GamepadAxis _axis, float _value);

	struct KeyState
	{
		bool down = false;
		u8 changesCount = 0;
	};
	struct AxisState
	{
		float value = 0.f;
		float delta = 0.f;
	};
	struct GamepadState
	{
		KeyState buttonStates[GAMEPADBUTTON_COUNT];
		AxisState axisStates[GAMEPADAXIS_COUNT];
		bool isConnected = false;
		i32 instanceId = -1;
	};

	SDL_Window* m_window = nullptr;

	KeyState m_keyStates[SDL_NUM_SCANCODES];
	GamepadState m_gamepadStates[GAMEPAD_COUNT];

	KeyState m_mouseButtonStates[MOUSEBUTTON_COUNT];
	AxisState m_mouseXAxis;
	AxisState m_mouseYAxis;
	Vector2 m_mouseScrollDelta = Vector2::ZERO();

	CursorMode m_cursorMode = CursorMode::NORMAL;
};

} // namespace yae
