#pragma once

#include <yae/types.h>
#include <yae/input_types.h>

#include <mirror/mirror.h>

namespace yae {
namespace editor {

enum InputMode
{
	INPUTMODE_NONE,
	INPUTMODE_KEYBOARD,
	INPUTMODE_MOUSE,
	INPUTMODE_GAMEPAD
};

class InputInspector
{
public:

	bool update();

	bool opened = false;
	InputMode inputMode = INPUTMODE_NONE;
	GamepadID selectedGamepad = GAMEPAD_INVALID;
};

} // namespace editor
} // namespace yae
