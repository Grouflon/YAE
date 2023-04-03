#include "input.h"

#include <yae/math.h>

#include <yae/yae_sdl.h>

//#define YAE_GAMEPAD_SUPPORTED (YAE_PLATFORM_WEB == 0)
#define YAE_GAMEPAD_SUPPORTED 1

namespace yae {

void InputSystem::init(SDL_Window* _window)
{
	YAE_CAPTURE_FUNCTION();

	m_window = _window;

	memset(m_keyStates, 0, sizeof(m_keyStates));
	memset(m_gamepadStates, 0, sizeof(m_gamepadStates));
	memset(m_mouseButtonStates, 0, sizeof(m_mouseButtonStates));
	m_mouseXAxis = {};
	m_mouseYAxis = {};
	m_mouseScrollDelta = Vector2::ZERO;

	for (size_t i = 0; i < countof(m_glfwGamepadToGamepad); ++i)
	{
		m_gamepadStates[i].glfwGamepadId = -1;
		m_glfwGamepadToGamepad[i] = -1;
	}

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
	m_mouseXAxis.delta = 0.f;
	m_mouseYAxis.delta = 0.f;
	m_mouseScrollDelta = Vector2::ZERO;
}

void InputSystem::processEvent(SDL_Event& _event)
{
	switch(_event.type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			notifyKeyEvent(
				_event.key.keysym.scancode,
				_event.key.state,
				_event.key.keysym.mod
			);
		}
		break;

		case SDL_MOUSEMOTION:
		{
			notifyMouseMotionEvent(_event.motion.x, _event.motion.xrel, _event.motion.y, _event.motion.yrel);
		}
		break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			notifyMouseButtonEvent(_event.button.button, _event.button.state, 0);
		}
		break;

		case SDL_MOUSEWHEEL:
		{
			notifyMouseScrollEvent(_event.wheel.x, _event.wheel.y);
		}
		break;

		JOYSTICKS!!
	}
}

void InputSystem::update()
{
	YAE_CAPTURE_FUNCTION();

#if 0
	// Gamepads
	//  Connections / disconnections
	// @NOTE(2021/04/24|remi): Gamepads mapping is really erratic in terms of Id inside GLFW, that's why we put an indirection between GLFW and our api
	
	GLFWgamepadstate glfwGamepadState;
	{
		for (size_t i = 0; i < countof(m_glfwGamepadToGamepad); ++i)
		{
			glfwGamepadState = {};
			if (glfwGetGamepadState(i, &glfwGamepadState) != GLFW_FALSE)
			{
				if (m_glfwGamepadToGamepad[i] == -1)
				{
					for (size_t j = 0; j < countof(m_gamepadStates); ++j)
					{
						if (m_gamepadStates[j].glfwGamepadId == -1)
						{
							m_glfwGamepadToGamepad[i] = j;
							m_gamepadStates[j].glfwGamepadId = i;
							YAE_LOGF_CAT("input", "Gamepad %d(%s:%d) connected", j, glfwGetGamepadName(i), i);
							break;
						}
					}
					YAE_ASSERT(m_glfwGamepadToGamepad[i] != -1);
				}
			}
			else
			{
				int gamepadId = m_glfwGamepadToGamepad[i];
				if (gamepadId != -1)
				{
					YAE_ASSERT(m_gamepadStates[gamepadId].glfwGamepadId == i8(i));

					YAE_LOGF_CAT("input", "Gamepad %d(%d) disconnected", gamepadId, i);
					m_gamepadStates[gamepadId].glfwGamepadId = -1;
					m_glfwGamepadToGamepad[i] = -1;
				}
			}
		}
	}
	
	//  Button states
	for (size_t i = 0; i < countof(m_gamepadStates); ++i)
	{
		GamepadState& gamepadState = m_gamepadStates[i];
		glfwGamepadState = {};
		if (gamepadState.glfwGamepadId >= 0)
		{
			int result = glfwGetGamepadState(gamepadState.glfwGamepadId, &glfwGamepadState);
			YAE_ASSERT(result != GLFW_FALSE);
		}

		// buttons
		for (size_t j = 0; j < countof(gamepadState.buttonStates); ++j)
		{
			KeyState& buttonState = gamepadState.buttonStates[j];
			buttonState.changesCount = 0;
			if (buttonState.down != bool(glfwGamepadState.buttons[j]))
			{
				buttonState.down = bool(glfwGamepadState.buttons[j]);
				++buttonState.changesCount;

				YAE_VERBOSEF_CAT("input", "gamepad %d button event: %d, %d", i, j, buttonState.down);
			}
		}

		// axis
		for (size_t j = 0; j < countof(gamepadState.axisStates); ++j)
		{
			AxisState& axisState = gamepadState.axisStates[j];
			axisState.delta = glfwGamepadState.axes[j] - axisState.value;
			axisState.value = glfwGamepadState.axes[j];


			if (axisState.delta != 0.f)
			{
				YAE_VERBOSEF_CAT("input", "gamepad %d axis event: %d, %.2f", i, j, axisState.value);
			}
		}
	}
#endif
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



bool InputSystem::wasGamepadButtonJustPressed(int _gamepadId, int _button) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	const KeyState& keyState = m_gamepadStates[_gamepadId].buttonStates[_button];
	return keyState.changesCount > 1 || (keyState.down && keyState.changesCount == 1);
}


bool InputSystem::wasGamepadButtonJustReleased(int _gamepadId, int _button) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	const KeyState& keyState = m_gamepadStates[_gamepadId].buttonStates[_button];
	return keyState.changesCount > 1 || (!keyState.down && keyState.changesCount == 1);
}


bool InputSystem::isGamepadButtonDown(int _gamepadId, int _button) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_button >= 0 && size_t(_button) < countof(m_gamepadStates[0].buttonStates));

	return m_gamepadStates[_gamepadId].buttonStates[_button].down;
}


float InputSystem::getGamepadAxisValue(int _gamepadId, int _axis) const
{
	YAE_ASSERT(_gamepadId >= 0 && size_t(_gamepadId) < countof(m_gamepadStates));
	YAE_ASSERT(_axis >= 0 && size_t(_axis) < countof(m_gamepadStates[0].axisStates));

	return m_gamepadStates[_gamepadId].axisStates[_axis].value;
}


float InputSystem::getGamepadAxisDelta(int _gamepadId, int _axis) const
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


void InputSystem::notifyKeyEvent(int _scancode, int _action, int _mods)
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


void InputSystem::notifyMouseButtonEvent(int _button, int _action, int _mods)
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


void InputSystem::notifyMouseScrollEvent(double _xOffset, double _yOffset)
{
	m_mouseScrollDelta = Vector2(float(_xOffset), float(_yOffset));

	YAE_VERBOSEF_CAT("input", "mouse scroll event: %.2f, %.2f", _xOffset, _yOffset);
}

void InputSystem::notifyMouseMotionEvent(int _x, int _xDelta, int _y, int _yDelta)
{
	m_mouseXAxis.delta += _xDelta;
	m_mouseXAxis.value = _x;
	m_mouseYAxis.delta += _yDelta;
	m_mouseYAxis.value = _y;

	YAE_VERBOSEF_CAT("input", "mouse motion event: pos:%d,%d / movement:%d,%d", _x, _y, _xDelta, _yDelta);
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
		case CURSORMODE_NORMAL:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_FALSE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_ENABLE));
		}
		break;

		case CURSORMODE_HIDDEN:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_FALSE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_DISABLE));
		}
		break;

		case CURSORMODE_LOCKED:
		{
			YAE_SDL_VERIFY(SDL_SetRelativeMouseMode(SDL_TRUE));
			YAE_SDL_VERIFY(SDL_ShowCursor(SDL_DISABLE));
		}
		break;
	}
}

} // namespace yae
