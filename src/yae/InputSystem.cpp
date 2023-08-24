#include "InputSystem.h"

#include <core/math.h>

#include <core/yae_sdl.h>

namespace yae {

void InputSystem::init(SDL_Window* _window)
{
	YAE_CAPTURE_FUNCTION();

	m_window = _window;

	YAE_VERBOSE_CAT("input", "Initialized Input System");
}

void InputSystem::beginFrame()
{
	YAE_CAPTURE_FUNCTION();

	for (size_t i = 0; i < countof(m_keyStates); ++i)
	{
		m_keyStates[i].changesCount = 0;
	}
	for (size_t i = 0; i < countof(m_mouseButtonStates); ++i)
	{
		m_mouseButtonStates[i].changesCount = 0;
	}
	for (size_t i = 0; i < countof(m_gamepadStates); ++i)
	{
		for (size_t j = 0; j < countof(m_gamepadStates[i].buttonStates); ++j)
		{
			m_gamepadStates[i].buttonStates[j].changesCount = 0;
		}

		for (size_t j = 0; j < countof(m_gamepadStates[i].axisStates); ++j)
		{
			m_gamepadStates[i].axisStates[j].delta = 0.f;
		}
	}
	m_mouseXAxis.delta = 0.f;
	m_mouseYAxis.delta = 0.f;
	m_mouseScrollDelta = Vector2::ZERO();
}

void InputSystem::processEvent(const SDL_Event& _event)
{
	switch(_event.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			_notifyKeyEvent(
				_event.key.keysym.scancode,
				_event.key.state,
				_event.key.keysym.mod
			);
		}
		break;

		case SDL_MOUSEMOTION:
		{
			_notifyMouseMotionEvent(_event.motion.x, _event.motion.xrel, _event.motion.y, _event.motion.yrel);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			// SDL MouseButton to Yae MouseButton
			MouseButton button = MOUSEBUTTON_INVALID;
			switch(_event.button.button)
			{
				case SDL_BUTTON_LEFT: button = MOUSEBUTTON_LEFT; break;
				case SDL_BUTTON_MIDDLE: button = MOUSEBUTTON_MIDDLE; break;
				case SDL_BUTTON_RIGHT: button = MOUSEBUTTON_RIGHT; break;
				case SDL_BUTTON_X1: button = MOUSEBUTTON_X1; break;
				case SDL_BUTTON_X2: button = MOUSEBUTTON_X2; break;
			}

			if (button >= 0 && button < MOUSEBUTTON_COUNT)
			{
				_notifyMouseButtonEvent(button, _event.button.state, 0);
			}
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			_notifyMouseScrollEvent(_event.wheel.x, _event.wheel.y);
		}
		break;

		case SDL_CONTROLLERDEVICEADDED:
		{
			int index = _event.cdevice.which;
			if (index >= GAMEPAD_COUNT)
			{
				YAE_ASSERT_MSG(index < GAMEPAD_COUNT, "Controller index exceeds max controller count.");
				break;
			}

			SDL_JoystickID instanceId = SDL_JoystickGetDeviceInstanceID(index);
			const char* name = SDL_JoystickNameForIndex(index);

			YAE_ASSERT(m_gamepadStates[index].isConnected == false);
			m_gamepadStates[index].isConnected = true;
			m_gamepadStates[index].instanceId = instanceId;

			YAE_VERIFY(SDL_GameControllerOpen(index) != nullptr);

			YAE_LOGF_CAT("input", "Controller \"%s\" connected (index=%d instanceId=%d)", name, index, instanceId);
		}
		break;

		case SDL_CONTROLLERDEVICEREMAPPED:
		{
			SDL_JoystickID instanceId = _event.cdevice.which;
			YAE_LOGF_CAT("input", "Controller remapped (instanceId=%d)", instanceId);
			YAE_ASSERT_MSG(false, "What does remapping means ?");
		}
		break;

		case SDL_CONTROLLERDEVICEREMOVED:
		{
			SDL_JoystickID instanceId = _event.cdevice.which;
			GamepadID index = _gamepadInstanceIdToGamepadId(instanceId);
			if (index < 0)
			{
				YAE_ASSERT_MSGF(index >= 0, "Gamepad with instanceId=%d not found in gamepad list", instanceId);
				break;
			}
			const char* name = SDL_JoystickNameForIndex(index);

			SDL_GameControllerClose(SDL_GameControllerFromInstanceID(instanceId));

			YAE_ASSERT(m_gamepadStates[index].isConnected);
			m_gamepadStates[index].isConnected = false;
			m_gamepadStates[index].instanceId = -1;
			memset(m_gamepadStates[index].buttonStates, 0, sizeof(m_gamepadStates[index].buttonStates));
			memset(m_gamepadStates[index].axisStates, 0, sizeof(m_gamepadStates[index].axisStates));

			YAE_LOGF_CAT("input", "Controller \"%s\" disconnected (index=%d instanceId=%d)", name, index, instanceId);
		}
		break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
		{
			SDL_JoystickID instanceId = _event.cbutton.which;
			GamepadID index = _gamepadInstanceIdToGamepadId(instanceId);
			YAE_ASSERT_MSGF(index >= 0, "Gamepad with instanceId=%d not found in gamepad list", instanceId);
			_notifyGamepadButtonEvent(index, GamepadButton(_event.cbutton.button), _event.cbutton.state);
		}
		break;

		case SDL_CONTROLLERAXISMOTION:
		{
			SDL_JoystickID instanceId = _event.caxis.which;
			GamepadID index = _gamepadInstanceIdToGamepadId(instanceId);
			YAE_ASSERT_MSGF(index >= 0, "Gamepad with instanceId=%d not found in gamepad list", instanceId);
			float value = _event.caxis.value >= 0 ? float(_event.caxis.value) / float(32767) : float(_event.caxis.value) / float(32768);
			_notifyGamepadAxisEvent(index, GamepadAxis(_event.caxis.axis), value);
		}
		break;
	}
}

void InputSystem::update()
{
	YAE_CAPTURE_FUNCTION();
}

void InputSystem::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	YAE_VERBOSE_CAT("input", "Shutdown Input System");
}

bool InputSystem::wasKeyJustPressed(int _key) const
{
	YAE_ASSERT(_key >= 0 && size_t(_key) < countof(m_keyStates));

	const KeyState& keyState = m_keyStates[_key];
	return keyState.changesCount > 1 || (keyState.down && keyState.changesCount == 1);
}

bool InputSystem::wasKeyJustReleased(int _key) const
{
	YAE_ASSERT(_key >= 0 && size_t(_key) < countof(m_keyStates));

	const KeyState& keyState = m_keyStates[_key];
	return keyState.changesCount > 1 || (!keyState.down && keyState.changesCount == 1);
}

bool InputSystem::isKeyDown(int _key) const
{
	YAE_ASSERT(_key >= 0 && size_t(_key) < countof(m_keyStates));

	return m_keyStates[_key].down;
}

bool InputSystem::isGamepadConnected(GamepadID _gamepadId) const
{
	YAE_ASSERT(_gamepadId < GAMEPAD_COUNT);
	if (_gamepadId == GAMEPAD_ANY)
	{
		return getConnectedGamepadCount() > 0;
	}
	if (_gamepadId < 0)
	{
		return 0;
	}

	return m_gamepadStates[_gamepadId].isConnected;
}

u32 InputSystem::getConnectedGamepadCount() const
{
	u32 count = 0;
	for (u32 i = 0; i < countof(m_gamepadStates); ++i)
	{
		if (m_gamepadStates[i].isConnected)
		{
			count += 1;
		}
	}
	return count;
}

const char* InputSystem::getGamepadName(GamepadID _gamepadId) const
{
	YAE_ASSERT(_gamepadId < GAMEPAD_COUNT);
	if (_gamepadId < 0)
	{
		return "Invalid";
	}
	
	if (!m_gamepadStates[_gamepadId].isConnected)
	{
		return "Disconnected";
	}

	return SDL_JoystickNameForIndex(_gamepadId);
}

bool InputSystem::wasGamepadButtonJustPressed(GamepadID _gamepadId, GamepadButton _button) const
{
	if (_button == GAMEPADBUTTON_ANY)
	{
		for (u32 i = 0; i < GAMEPADBUTTON_COUNT; ++i)
		{
			if (wasGamepadButtonJustPressed(_gamepadId, GamepadButton(i)))
			{
				return true;
			}
		}
		return false;
	}

	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	const KeyState& keyState = m_gamepadStates[_gamepadId].buttonStates[_button];
	return keyState.changesCount > 1 || (keyState.down && keyState.changesCount == 1);
}

bool InputSystem::wasGamepadButtonJustReleased(GamepadID _gamepadId, GamepadButton _button) const
{
	if (_button == GAMEPADBUTTON_ANY)
	{
		for (u32 i = 0; i < GAMEPADBUTTON_COUNT; ++i)
		{
			if (wasGamepadButtonJustReleased(_gamepadId, GamepadButton(i)))
			{
				return true;
			}
		}
		return false;
	}

	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	const KeyState& keyState = m_gamepadStates[_gamepadId].buttonStates[_button];
	return keyState.changesCount > 1 || (!keyState.down && keyState.changesCount == 1);
}

bool InputSystem::isGamepadButtonDown(GamepadID _gamepadId, GamepadButton _button) const
{
	if (_button == GAMEPADBUTTON_ANY)
	{
		for (u32 i = 0; i < GAMEPADBUTTON_COUNT; ++i)
		{
			if (isGamepadButtonDown(_gamepadId, GamepadButton(i)))
			{
				return true;
			}
		}
		return false;
	}
	
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	return m_gamepadStates[_gamepadId].buttonStates[_button].down;
}

float InputSystem::getGamepadAxisValue(GamepadID _gamepadId, GamepadAxis _axis) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_axis >= 0 && size_t(_axis) < countof(m_gamepadStates[0].axisStates));

	return m_gamepadStates[_gamepadId].axisStates[_axis].value;
}

float InputSystem::getGamepadAxisDelta(GamepadID _gamepadId, GamepadAxis _axis) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_axis >= 0 && size_t(_axis) < countof(m_gamepadStates[0].axisStates));	

	return m_gamepadStates[_gamepadId].axisStates[_axis].delta;
}

bool InputSystem::isCtrlDown() const
{
	return m_keyStates[SDL_SCANCODE_LCTRL].down || m_keyStates[SDL_SCANCODE_RCTRL].down;
}

bool InputSystem::isShiftDown() const
{
	return m_keyStates[SDL_SCANCODE_LSHIFT].down || m_keyStates[SDL_SCANCODE_RSHIFT].down;
}

bool InputSystem::isAltDown() const
{
	return m_keyStates[SDL_SCANCODE_LALT].down || m_keyStates[SDL_SCANCODE_RALT].down;
}

Vector2 InputSystem::getMousePosition() const
{
	return Vector2(m_mouseXAxis.value, m_mouseYAxis.value);
}

Vector2 InputSystem::getMouseDelta() const
{
	return Vector2(m_mouseXAxis.delta, m_mouseYAxis.delta);
}

Vector2 InputSystem::getMouseScrollDelta() const
{
	return m_mouseScrollDelta;
}

bool InputSystem::wasMouseButtonJustPressed(int _button) const
{
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_mouseButtonStates));
	
	const KeyState& keyState = m_mouseButtonStates[_button];
	return keyState.changesCount > 1 || (keyState.down && keyState.changesCount == 1);
}

bool InputSystem::wasMouseButtonJustReleased(int _button) const
{
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_mouseButtonStates));

	const KeyState& keyState = m_mouseButtonStates[_button];
	return keyState.changesCount > 1 || (!keyState.down && keyState.changesCount == 1);
}

bool InputSystem::isMouseButtonDown(int _button) const
{
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_mouseButtonStates));

	return m_mouseButtonStates[_button].down;
}

CursorMode InputSystem::getCursorMode() const
{
	return m_cursorMode;
}

void InputSystem::setCursorMode(CursorMode _mode)
{
	m_cursorMode = _mode;
	switch(m_cursorMode)
	{
		case CursorMode::NORMAL:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_FALSE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_ENABLE));
		}
		break;

		case CursorMode::HIDDEN:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_FALSE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_DISABLE));
		}
		break;

		case CursorMode::LOCKED:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_TRUE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_DISABLE));
		}
		break;
	}
}

GamepadID InputSystem::_gamepadInstanceIdToGamepadId(i32 _instanceId) const
{
	for (u32 i = 0; i < GAMEPAD_COUNT; ++i)
	{
		if (m_gamepadStates[i].instanceId == _instanceId)
		{
			return GamepadID(i);
			break;
		}
	}
	return GamepadID(-1);
}

void InputSystem::_notifyKeyEvent(int _scancode, int _action, int _mods)
{
	YAE_ASSERT(_scancode >= 0 && size_t(_scancode) < countof(m_keyStates));

	KeyState& keyState = m_keyStates[_scancode];
	++keyState.changesCount;

	if (_action == SDL_PRESSED)
	{
        keyState.down = true;
	}
    else if (_action == SDL_RELEASED)
    {
        keyState.down = false;
    }

	YAE_VERBOSEF_CAT("input", "key event: %d, %d, %d", _scancode, _action, _mods);
}

void InputSystem::_notifyMouseButtonEvent(MouseButton _button, int _action, int _mods)
{
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_mouseButtonStates));

	KeyState& buttonState = m_mouseButtonStates[_button];
	++buttonState.changesCount;

	if (_action == SDL_PRESSED)
	{
        buttonState.down = true;
	}
    else if (_action == SDL_RELEASED)
    {
        buttonState.down = false;
    }

	YAE_VERBOSEF_CAT("input", "mouse button event: %d, %d, %d", _button, _action, _mods);
}


void InputSystem::_notifyMouseScrollEvent(double _xOffset, double _yOffset)
{
	m_mouseScrollDelta = Vector2(float(_xOffset), float(_yOffset));

	YAE_VERBOSEF_CAT("input", "mouse scroll event: %.2f, %.2f", _xOffset, _yOffset);
}

void InputSystem::_notifyMouseMotionEvent(int _x, int _xDelta, int _y, int _yDelta)
{
	m_mouseXAxis.delta += _xDelta;
	m_mouseXAxis.value = _x;
	m_mouseYAxis.delta += _yDelta;
	m_mouseYAxis.value = _y;

	YAE_VERBOSEF_CAT("input", "mouse motion event: pos:%d,%d / movement:%d,%d", _x, _y, _xDelta, _yDelta);
}

void InputSystem::_notifyGamepadButtonEvent(GamepadID _gamepadId, GamepadButton _button, int _action)
{
	YAE_ASSERT(_gamepadId < GAMEPAD_COUNT);
	YAE_ASSERT(_button >= 0 && _button < GAMEPADBUTTON_COUNT);
	YAE_ASSERT(m_gamepadStates[_gamepadId].isConnected);

	KeyState& buttonState = m_gamepadStates[_gamepadId].buttonStates[_button];
	++buttonState.changesCount;
	if (_action == SDL_PRESSED)
	{
        buttonState.down = true;
	}
    else if (_action == SDL_RELEASED)
    {
        buttonState.down = false;
    }

	YAE_VERBOSEF_CAT("input", "gamepad button event: id=%d button=%d, action=%d", _gamepadId, _button, _action);	
}

void InputSystem::_notifyGamepadAxisEvent(GamepadID _gamepadId, GamepadAxis _axis, float _value)
{
	YAE_ASSERT(_gamepadId < GAMEPAD_COUNT);
	YAE_ASSERT(_axis >= 0 && _axis < GAMEPADAXIS_COUNT);
	YAE_ASSERT(m_gamepadStates[_gamepadId].isConnected);

	AxisState& axisState = m_gamepadStates[_gamepadId].axisStates[_axis];
	axisState.delta = _value - axisState.value;
	axisState.value = _value;

	YAE_VERBOSEF_CAT("input", "gamepad axis event: id=%d axis=%d, value=%.3f", _gamepadId, _axis, _value);	
}

} // namespace yae
