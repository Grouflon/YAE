#include "InputInspector.h"

#include <yae/InputSystem.h>
#include <yae/containers/Array.h>
#include <yae/string.h>

#include <imgui/imgui.h>


MIRROR_CLASS(yae::editor::InputInspector)
(
	MIRROR_MEMBER(opened);
);

namespace yae {
namespace editor {

bool InputInspector::update()
{
    bool changedSettings = false;
	if (opened)
    {
		auto getMouseButtonState = [](MouseButton _button)
		{
			if (input().wasMouseButtonJustReleased(_button))
				return 1;
			else if (input().wasMouseButtonJustPressed(_button))
				return 2;
			else if (input().isMouseButtonDown(_button))
				return 3;
			return 0;
		};
		auto getGamepadButtonState = [](GamepadID _gamepadId, GamepadButton _button)
		{
			if (input().wasGamepadButtonJustReleased(_gamepadId, _button))
				return 1;
			else if (input().wasGamepadButtonJustPressed(_gamepadId, _button))
				return 2;
			else if (input().isGamepadButtonDown(_gamepadId, _button))
				return 3;
			return 0;
		};
		const char* stateStrings[] = {
			"up",
			"released",
			"pressed",
			"down"
		};
		const ImColor stateColors[] = {
			ImColor(127,127,127,255),
			ImColor(0,255,0,255),
			ImColor(0,0,255,255),
			ImColor(255,0,0,255),
		};

    	bool previousOpened = opened;
    	u32 flags = ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_AlwaysAutoResize;
		if (ImGui::Begin("Input", &opened, flags))
    	{
    		ImGui::BeginChild("InputChildL", ImVec2(240.f, 300.f), false);

    		const u32 leafFlags = ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx("Keyboard", leafFlags|(inputMode == INPUTMODE_KEYBOARD ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked())
			{
				inputMode = INPUTMODE_KEYBOARD;
				selectedGamepad = GAMEPAD_INVALID;
			}
			ImGui::TreeNodeEx("Mouse", leafFlags|(inputMode == INPUTMODE_MOUSE ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked())
			{
				inputMode = INPUTMODE_MOUSE;
				selectedGamepad = GAMEPAD_INVALID;
			}
			if (ImGui::TreeNodeEx("gamepads", 0, "Gamepads(%d)", input().getConnectedGamepadCount()))
			{
				for (i32 i = 0; i < GAMEPAD_COUNT; ++i)
				{
					GamepadID id = GamepadID(i);
					if (input().isGamepadConnected(id))
					{
						ImGui::TreeNodeEx((void*)i, leafFlags|(selectedGamepad == id ? ImGuiTreeNodeFlags_Selected : 0), "%d:%s", id, input().getGamepadName(id));
						if (ImGui::IsItemClicked())
						{
							inputMode = INPUTMODE_GAMEPAD;
							selectedGamepad = id;
						}
					}
				}
				ImGui::TreePop();
			}

    		ImGui::EndChild();

    		ImGui::SameLine();

    		ImGui::BeginChild("InputChildR", ImVec2(470.f, 300.f), true);
    		switch(inputMode)
    		{
    			case INPUTMODE_KEYBOARD:
				{
    				// Gather States
    				DataArray<u32> downKeys(&scratchAllocator());
    				DataArray<u32> pressedKeys(&scratchAllocator());
    				DataArray<u32> releasedKeys(&scratchAllocator());
    				for (i32 i = 0; i < SDL_NUM_SCANCODES; ++i)
					{
						bool down = input().isKeyDown(i);
						bool pressed = input().wasKeyJustPressed(i);
						bool released = input().wasKeyJustReleased(i);
						if (down) downKeys.push_back(i);
						if (pressed) pressedKeys.push_back(i);
						if (released) releasedKeys.push_back(i);
					}

					// Display table
    				const u32 headerFlags = ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::BeginTable("keyboardTable", 3);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					{
						char nameBuffer[128];
						string::format(nameBuffer, countof(nameBuffer), "Down(%d)", downKeys.size());
						ImGui::CollapsingHeader(nameBuffer, headerFlags);
						for(u32 key : downKeys)
						{
							ImGui::Text("%d", key);
						}
					}
					ImGui::TableNextColumn();
					{
						char nameBuffer[128];
						string::format(nameBuffer, countof(nameBuffer), "Pressed(%d)", pressedKeys.size());
						ImGui::CollapsingHeader(nameBuffer, headerFlags);
						for(u32 key : pressedKeys)
						{
							ImGui::Text("%d", key);
						}
					}
					ImGui::TableNextColumn();
					{
						char nameBuffer[128];
						string::format(nameBuffer, countof(nameBuffer), "Released(%d)", releasedKeys.size());
						ImGui::CollapsingHeader(nameBuffer, headerFlags);
						for(u32 key : releasedKeys)
						{
							ImGui::Text("%d", key);
						}
					}
					ImGui::EndTable();
				}
				break;

				case INPUTMODE_MOUSE:
				{
					Vector2 mousePosition = input().getMousePosition();
					ImGui::Text("Position: x=%d, y=%d", u32(mousePosition.x), u32(mousePosition.y));
					Vector2 mousePositionDelta = input().getMouseDelta();
					ImGui::Text("Delta: x=%d, y=%d", u32(mousePositionDelta.x), u32(mousePositionDelta.y));
					Vector2 mouseScrollDelta = input().getMouseScrollDelta();
					ImGui::Text("Scroll Delta: x=%d, y=%d", u32(mouseScrollDelta.x), u32(mouseScrollDelta.y));

    				const u32 headerFlags = ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
					ImGui::CollapsingHeader("Buttons", headerFlags);
					ImGui::BeginTable("MouseButtonTable", 3);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Left:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getMouseButtonState(MOUSEBUTTON_LEFT)], "%s", stateStrings[getMouseButtonState(MOUSEBUTTON_LEFT)]);
					ImGui::TableNextColumn();
					ImGui::Text("Middle:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getMouseButtonState(MOUSEBUTTON_MIDDLE)], "%s", stateStrings[getMouseButtonState(MOUSEBUTTON_MIDDLE)]);
					ImGui::TableNextColumn();
					ImGui::Text("Right:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getMouseButtonState(MOUSEBUTTON_RIGHT)], "%s", stateStrings[getMouseButtonState(MOUSEBUTTON_RIGHT)]);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("X1:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getMouseButtonState(MOUSEBUTTON_X1)], "%s", stateStrings[getMouseButtonState(MOUSEBUTTON_X1)]);
					ImGui::TableNextColumn();
					ImGui::Text("X2:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getMouseButtonState(MOUSEBUTTON_X2)], "%s", stateStrings[getMouseButtonState(MOUSEBUTTON_X2)]);
					ImGui::EndTable();
				}
				break;

				case INPUTMODE_GAMEPAD:
				{
					if (!input().isGamepadConnected(selectedGamepad))
					{
						inputMode = INPUTMODE_NONE;
						selectedGamepad = GAMEPAD_INVALID;
						break;
					}

					GamepadID padId = selectedGamepad;

					auto drawStick = [](float _radius, ImVec2 _input)
					{
                		ImDrawList* drawList = ImGui::GetWindowDrawList();
	                    const ImVec2 center = ImGui::GetCursorScreenPos() + _radius;

						auto drawCross = [](ImVec2 _pos, float _radius, float _thickness, ImU32 _col)
						{
                			ImDrawList* drawList = ImGui::GetWindowDrawList();
                			_pos.x = std::floor(_pos.x);
                			_pos.y = std::floor(_pos.y);
							drawList->AddLine(_pos + ImVec2(-_radius, 0.f), _pos + ImVec2(_radius + 1.f, 0.f), _col, _thickness);
                    		drawList->AddLine(_pos + ImVec2(0.f, -_radius - 1.f), _pos + ImVec2(0.f, _radius), _col, _thickness);
						};

                    	const float centerRadius = 3.f;
                    	const float crossRadius = 5.f;

						drawList->AddCircle(center, _radius, IM_COL32(255,255,255,255));
                    	drawCross(center, centerRadius, 1.f, IM_COL32(255,255,255,255));
                    	drawCross(center + _input*_radius, crossRadius, 3.f, IM_COL32(255,0,0,255));

                    	ImGui::Dummy(ImVec2(_radius*2.f, _radius*2.f));
					};

					auto drawTrigger = [](float _width, float _height, float _input)
					{
                		ImDrawList* drawList = ImGui::GetWindowDrawList();
					    float regionWidth = ImGui::GetContentRegionAvail().x;

	                    const ImVec2 topLeft = ImGui::GetCursorScreenPos() + ImVec2((regionWidth - _width) * 0.5 - 4.f, 0.f);
	                    drawList->AddRect(topLeft, topLeft + ImVec2(_width, _height), IM_COL32(255,255,255,255));

	                    const float margin = 3.f;
	                    const ImVec2 fillTopLeft = topLeft + margin;
	                    drawList->AddRectFilled(fillTopLeft, fillTopLeft + ImVec2(_width - margin*2.f, (_height - margin*2.f) * _input), IM_COL32(255,0,0,255));

                    	ImGui::Dummy(ImVec2(regionWidth, _height));
					};

					auto textCentered = [](const char* _text) {
					    ImGui::NewLine();
					    float windowWidth = ImGui::GetContentRegionAvail().x;
					    float textWidth   = ImGui::CalcTextSize(_text).x;
					    ImGui::SameLine((windowWidth - textWidth) * 0.5f + 4.f);
					    ImGui::Text("%s", _text);
					};

					const u32 headerFlags = ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
                    const float stickRadius = 40.f;
					ImGui::CollapsingHeader("Axis", headerFlags);
					ImGui::BeginTable("GamepadAxisTable", 4);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					{
						ImVec2 stickInput = ImVec2(
	                    	input().getGamepadAxisValue(padId, GAMEPADAXIS_LEFTX),
	                    	input().getGamepadAxisValue(padId, GAMEPADAXIS_LEFTY)
	                    );
	                    textCentered("Left Stick");
	                    drawStick(stickRadius, stickInput);
	                    char buffer[128];
	                    string::format(buffer, countof(buffer), "X:%.3f\nY:%.3f", stickInput.x, stickInput.y);
	                    textCentered(buffer);
					}                    
                    ImGui::TableNextColumn();
                    {
						ImVec2 stickInput = ImVec2(
	                    	input().getGamepadAxisValue(padId, GAMEPADAXIS_RIGHTX),
	                    	input().getGamepadAxisValue(padId, GAMEPADAXIS_RIGHTY)
	                    );
	                    textCentered("Right Stick");
	                    drawStick(stickRadius, stickInput);
	                    char buffer[128];
	                    string::format(buffer, countof(buffer), "X:%.3f\nY:%.3f", stickInput.x, stickInput.y);
	                    textCentered(buffer);
					}  
                    ImGui::TableNextColumn();
                    {
						float axisInput = input().getGamepadAxisValue(padId, GAMEPADAXIS_LEFTTRIGGER);
	                    textCentered("Left Trigger");
	                    drawTrigger(20.f, 80.f, axisInput);
	                    char buffer[128];
	                    string::format(buffer, countof(buffer), "%.3f", axisInput);
	                    textCentered(buffer);
					}  
                    ImGui::TableNextColumn();
                    {
						float axisInput = input().getGamepadAxisValue(padId, GAMEPADAXIS_RIGHTTRIGGER);
	                    textCentered("Right Trigger");
	                    drawTrigger(20.f, 80.f, axisInput);
	                    char buffer[128];
	                    string::format(buffer, countof(buffer), "%.3f", axisInput);
	                    textCentered(buffer);
					}  
                 	ImGui::EndTable();

                    ImGui::CollapsingHeader("Buttons", headerFlags);
                    ImGui::BeginTable("MouseButtonTable", 4);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Bottom:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_BOTTOM)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_BOTTOM)]);
					ImGui::TableNextColumn();
					ImGui::Text("Right:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHT)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHT)]);
					ImGui::TableNextColumn();
					ImGui::Text("Left:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_LEFT)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_LEFT)]);
					ImGui::TableNextColumn();
					ImGui::Text("Top:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_TOP)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_TOP)]);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("LShoulder:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_LEFTSHOULDER)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_LEFTSHOULDER)]);
					ImGui::TableNextColumn();
					ImGui::Text("RShoulder:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHTSHOULDER)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHTSHOULDER)]);
					ImGui::TableNextColumn();
					ImGui::Text("LStick:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_LEFTSTICK)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_LEFTSTICK)]);
					ImGui::TableNextColumn();
					ImGui::Text("RStick:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHTSTICK)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_RIGHTSTICK)]);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("DPad Up:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_UP)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_UP)]);
					ImGui::TableNextColumn();
					ImGui::Text("DPad Down:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_DOWN)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_DOWN)]);
					ImGui::TableNextColumn();
					ImGui::Text("DPad Left:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_LEFT)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_LEFT)]);
					ImGui::TableNextColumn();
					ImGui::Text("DPad Right:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_RIGHT)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_DPAD_RIGHT)]);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Back:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_BACK)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_BACK)]);
					ImGui::TableNextColumn();
					ImGui::Text("Guide:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_GUIDE)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_GUIDE)]);
					ImGui::TableNextColumn();
					ImGui::Text("Start:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_START)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_START)]);
					ImGui::TableNextColumn();
					ImGui::Text("Misc1:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_MISC1)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_MISC1)]);
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
					ImGui::Text("Touchpad:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_TOUCHPAD)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_TOUCHPAD)]);
					ImGui::TableNextColumn();
					ImGui::Text("Any:"); ImGui::SameLine();
					ImGui::TextColored(stateColors[getGamepadButtonState(padId, GAMEPADBUTTON_ANY)], "%s", stateStrings[getGamepadButtonState(padId, GAMEPADBUTTON_ANY)]);
					ImGui::EndTable();
				}
				break;
    		}

    		ImGui::EndChild();
    	}
    	ImGui::End();

		changedSettings = changedSettings || (previousOpened != opened);
    }

    return changedSettings;
}

} // namespace editor
} // namespace yae
