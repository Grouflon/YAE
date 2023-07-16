#include "editor.h"

#include <yae/Application.h>
#include <yae/im3d_extension.h>
#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>
#include <yae/math_3d.h>
#include <yae/memory.h>
#include <yae/program.h>
#include <yae/rendering/Renderer.h>
#include <yae/ResourceManager.h>
#include <yae/resource.h>
#include <yae/resources/Mesh.h>
#include <yae/resources/Resource.h>
#include <yae/resources/ShaderFile.h>
#include <yae/resources/ShaderProgram.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/Serializer.h>
#include <yae/string.h>

#include <editor/ResourceEditor.h>

#include <imgui/imgui.h>
#include <im3d/im3d.h>
#include <mirror/mirror.h>

using namespace yae;

enum InputMode
{
	INPUTMODE_NONE,
	INPUTMODE_KEYBOARD,
	INPUTMODE_MOUSE,
	INPUTMODE_GAMEPAD
};

struct EditorInstance
{
	bool showMemoryProfiler = false;
	bool showFrameRate = false;
	bool showInputDebugWindow = false;
	bool showMirrorDebugWindow = false;
	bool showRendererDebugWindow = false;
	bool showDemoWindow = false;

	ShaderProgram* wireframeShader = nullptr;
	ShaderProgram* normalsShader = nullptr;

	// resource inspector
	editor::ResourceEditor resourceEditor;

	// input window
	InputMode inputMode = INPUTMODE_NONE;
	GamepadID selectedGamepad = GAMEPAD_INVALID;

	// mirror window
	mirror::TypeID selectedTypeID = mirror::UNDEFINED_TYPEID;

	MIRROR_CLASS_NOVIRTUAL(EditorInstance)
	(
		MIRROR_MEMBER(showMemoryProfiler)();
		MIRROR_MEMBER(showFrameRate)();
		MIRROR_MEMBER(showInputDebugWindow)();
		MIRROR_MEMBER(showRendererDebugWindow)();
		MIRROR_MEMBER(showMirrorDebugWindow)();
		MIRROR_MEMBER(showDemoWindow)();

		MIRROR_MEMBER(resourceEditor)();
	);
};
MIRROR_CLASS_DEFINITION(EditorInstance);

void displayTypeTreeNode(EditorInstance* _editorInstance, mirror::TypeDesc* _type, mirror::TypeDesc* _parent = nullptr)
{
	mirror::TypeID id = _type->getTypeID();
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (_editorInstance->selectedTypeID == id ? ImGuiTreeNodeFlags_Selected : 0);
	mirror::Class* clss = _type->asClass();

	if (clss != nullptr && clss->getParents().size() > 0)
	{
		if (_parent == nullptr)
			return;

		bool parentMatch = false;
		for (mirror::TypeID parentID : clss->getParents())
		{
			if (parentID == _parent->getTypeID())
			{
				parentMatch = true;
				break;
			}
		}
		if (!parentMatch)
			return;
	}

	bool hasChildren = clss != nullptr && clss->getChildren().size() > 0;
	if(!hasChildren)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
	}

	bool isNodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)id, nodeFlags, "%s", _type->getName());
	if (ImGui::IsItemClicked())
	{
		_editorInstance->selectedTypeID = id;
	}
    if (hasChildren && isNodeOpen)
    {
    	for (mirror::TypeID childId : _type->asClass()->getChildren())
    	{
    		mirror::TypeDesc* childType = mirror::GetTypeSet().findTypeByID(childId);
    		if (childType == nullptr)
    			continue;

    		displayTypeTreeNode(_editorInstance, childType, _type);
    	}
    	
    	ImGui::TreePop();
    }
}

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{

}

void onModuleUnloaded(yae::Program* _program, yae::Module* _module)
{

}

void initModule(yae::Program* _program, yae::Module* _module)
{

}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{

}

void beforeInitApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = toolAllocator().create<EditorInstance>();
	_application->setUserData("editor", editorInstance);
}

void afterInitApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");

	{
		editorInstance->wireframeShader = resource::findOrCreate<ShaderProgram>("wireframeShader");

#if YAE_PLATFORM_WEB == 0
		Shader* shaders[] =
		{
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_vert.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_geom.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_frag.res")
		};

		editorInstance->wireframeShader->setShaderStages(shaders, countof(shaders));
#endif

		editorInstance->wireframeShader->load();
		//YAE_ASSERT(editorInstance->wireframeShader->isLoaded());
	}

	{
		editorInstance->normalsShader = resource::findOrCreate<ShaderProgram>("normalsShader");

#if YAE_PLATFORM_WEB == 0
		Shader* shaders[] =
		{
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_vert.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_geom.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_frag.res")
		};

		editorInstance->normalsShader->setShaderStages(shaders, countof(shaders));
		editorInstance->normalsShader->setPrimitiveMode(PrimitiveMode::POINTS);
#endif
		
		editorInstance->normalsShader->load();
		//YAE_ASSERT(editorInstance->normalsShader->isLoaded());
	}

	editorInstance->resourceEditor.init();
}

void beforeShutdownApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");

	editorInstance->resourceEditor.shutdown();

	editorInstance->wireframeShader->release();
	editorInstance->wireframeShader = nullptr;

	editorInstance->normalsShader->release();
	editorInstance->normalsShader = nullptr;
}

void afterShutdownApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");
	_application->setUserData("editor", nullptr);

	toolAllocator().destroy(editorInstance);
}

void updateApplication(yae::Application* _application, float _dt)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");

	bool changedSettings = false;

	if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
        	if (ImGui::MenuItem("Exit"))
        	{
        		_application->requestExit();
        	}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Display"))
        {
	        changedSettings = ImGui::MenuItem("Resources", NULL, &editorInstance->resourceEditor.opened) || changedSettings;

        	if (ImGui::BeginMenu("Profiling"))
	        {
	        	changedSettings = ImGui::MenuItem("Memory", NULL, &editorInstance->showMemoryProfiler) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Frame rate", NULL, &editorInstance->showFrameRate) || changedSettings;

	            ImGui::EndMenu();
	        }

	        if (ImGui::BeginMenu("Debug"))
	        {
	        	changedSettings = ImGui::MenuItem("Input", NULL, &editorInstance->showInputDebugWindow) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Mirror", NULL, &editorInstance->showMirrorDebugWindow) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Renderer", NULL, &editorInstance->showRendererDebugWindow) || changedSettings;
	            ImGui::EndMenu();
	        }

	        if (ImGui::BeginMenu("Misc"))
	        {
	        	changedSettings = ImGui::MenuItem("ImGui Demo Window", NULL, &editorInstance->showDemoWindow) || changedSettings;
	            ImGui::EndMenu();
	        }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    editorInstance->resourceEditor.update();

    if (editorInstance->showMemoryProfiler)
    {
    	auto showAllocatorInfo = [](const char* _name, const Allocator& _allocator)
    	{
    		char allocatedSizeBuffer[32];
	    	char allocableSizeBuffer[32];
	    	auto formatSize = [](size_t _size, char _buf[32])
	    	{
	    		if (_size == Allocator::SIZE_NOT_TRACKED)
	    		{
	    			snprintf(_buf, 31, "???");
	    		}
	    		else
	    		{
	    			const char* units[] =
	    			{
	    				"b",
	    				"Kb",
	    				"Mb",
	    				"Gb",
	    				"Tb"
	    			};

	    			u8 unit = 0;
	    			u32 mod = 1024 * 10;
	    			while (_size > mod)
	    			{
	    				_size = _size / mod;
	    				++unit;
	    			}

	    			snprintf(_buf, 31, "%zu %s", _size, units[unit]);
	    		}
	    	};

    		formatSize(_allocator.getAllocatedSize(), allocatedSizeBuffer);
    		formatSize(_allocator.getAllocableSize(), allocableSizeBuffer);

    		ImGui::Text("%s: %s / %s, %zu allocations",
	    		_name,
	    		allocatedSizeBuffer,
	    		allocableSizeBuffer,
	    		_allocator.getAllocationCount()
	    	);
    	};

    	bool previousOpen = editorInstance->showMemoryProfiler;
    	if (ImGui::Begin("Memory Profiler", &editorInstance->showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize))
    	{
    		showAllocatorInfo("Default", defaultAllocator());
	    	showAllocatorInfo("Scratch", scratchAllocator());
	    	showAllocatorInfo("Tool", toolAllocator());
	    	showAllocatorInfo("Malloc", mallocAllocator());
    	}
    	ImGui::End();
		changedSettings = changedSettings || (previousOpen != editorInstance->showMemoryProfiler);
    }

    if (editorInstance->showFrameRate)
    {
    	bool previousOpen = editorInstance->showFrameRate;
    	if (ImGui::Begin("Frame Rate", &editorInstance->showFrameRate, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs))
    	{
    		ImGui::Text("dt: %.2f ms", _dt*1000.f);
    		ImGui::Text("%.2f fps", _dt != 0.f ? (1.f/_dt) : 0.f);	
    	}
    	ImGui::End();
		changedSettings = changedSettings || (previousOpen != editorInstance->showFrameRate);
    }

    if (editorInstance->showInputDebugWindow)
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

    	bool previousOpen = editorInstance->showInputDebugWindow;
    	u32 flags = ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_AlwaysAutoResize;
		if (ImGui::Begin("Input", &editorInstance->showInputDebugWindow, flags))
    	{
    		ImGui::BeginChild("InputChildL", ImVec2(240.f, 300.f), false);

    		const u32 leafFlags = ImGuiTreeNodeFlags_Leaf|ImGuiTreeNodeFlags_NoTreePushOnOpen;
			ImGui::TreeNodeEx("Keyboard", leafFlags|(editorInstance->inputMode == INPUTMODE_KEYBOARD ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked())
			{
				editorInstance->inputMode = INPUTMODE_KEYBOARD;
				editorInstance->selectedGamepad = GAMEPAD_INVALID;
			}
			ImGui::TreeNodeEx("Mouse", leafFlags|(editorInstance->inputMode == INPUTMODE_MOUSE ? ImGuiTreeNodeFlags_Selected : 0));
			if (ImGui::IsItemClicked())
			{
				editorInstance->inputMode = INPUTMODE_MOUSE;
				editorInstance->selectedGamepad = GAMEPAD_INVALID;
			}
			if (ImGui::TreeNodeEx("gamepads", 0, "Gamepads(%d)", input().getConnectedGamepadCount()))
			{
				for (i32 i = 0; i < GAMEPAD_COUNT; ++i)
				{
					GamepadID id = GamepadID(i);
					if (input().isGamepadConnected(id))
					{
						ImGui::TreeNodeEx((void*)i, leafFlags|(editorInstance->selectedGamepad == id ? ImGuiTreeNodeFlags_Selected : 0), "%d:%s", id, input().getGamepadName(id));
						if (ImGui::IsItemClicked())
						{
							editorInstance->inputMode = INPUTMODE_GAMEPAD;
							editorInstance->selectedGamepad = id;
						}
					}
				}
				ImGui::TreePop();
			}

    		ImGui::EndChild();

    		ImGui::SameLine();

    		ImGui::BeginChild("InputChildR", ImVec2(470.f, 300.f), true);
    		switch(editorInstance->inputMode)
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
					if (!input().isGamepadConnected(editorInstance->selectedGamepad))
					{
						editorInstance->inputMode = INPUTMODE_NONE;
						editorInstance->selectedGamepad = GAMEPAD_INVALID;
						break;
					}

					GamepadID padId = editorInstance->selectedGamepad;

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

		changedSettings = changedSettings || (previousOpen != editorInstance->showInputDebugWindow);
    }

    if (editorInstance->showRendererDebugWindow)
    {
    	bool previousOpen = editorInstance->showRendererDebugWindow;
		if (ImGui::Begin("Renderer", &editorInstance->showRendererDebugWindow))
    	{
    		/*
    		ImGui::Text("view matrix:");
			imgui_matrix4(renderer().m_viewMatrix);
    		ImGui::Text("projection matrix:");
			imgui_matrix4(renderer().m_projMatrix);
			*/
    	}
    	ImGui::End();

		changedSettings = changedSettings || (previousOpen != editorInstance->showRendererDebugWindow);
    }

    if (editorInstance->showMirrorDebugWindow)
    {
    	bool previousOpen = editorInstance->showMirrorDebugWindow;
		if (ImGui::Begin("Mirror", &editorInstance->showMirrorDebugWindow))
    	{
    		ImVec2 regionSize = ImVec2(ImGui::GetWindowContentRegionWidth(), 0);
    		ImGui::BeginChild("MirrorChildL", ImVec2(regionSize.x * 0.5f, regionSize.y), false);
    		for (mirror::TypeDesc* type : mirror::GetTypeSet().getTypes())
    		{
				displayTypeTreeNode(editorInstance, type, nullptr);
    		}
    		ImGui::EndChild();

    		ImGui::SameLine();

    		ImGui::BeginChild("MirrorChildR", ImVec2(0, regionSize.y), true);
    		if (editorInstance->selectedTypeID != mirror::UNDEFINED_TYPEID)
    		{
    			mirror::TypeDesc* type = mirror::FindTypeByID(editorInstance->selectedTypeID);
    			if (type != nullptr)
    			{
    				ImGui::Text("name: %s", type->getName());
    				ImGui::Text("type: %s", mirror::TypeToString(type->getType()));
    				ImGui::Text("size: %lld", type->getSize());
    				if (mirror::Class* clss = type->asClass())
    				{
    					size_t membersCount = clss->getMembersCount();
    					if (membersCount > 0)
    					{
    						ImGui::Text("members:");

    						DataArray<mirror::ClassMember*> members(&scratchAllocator());
    						members.resize(membersCount);
							clss->getMembers(members.data(), members.size());

    						for (mirror::ClassMember* member : members)
    						{
    							mirror::TypeDesc* memberType = member->getType();
    							ImGui::BulletText("%s (%s)", member->getName(), memberType ? memberType->getName() : "unknown");
    						}
    					}
    					else
    					{
    						ImGui::Text("no members");
    					}
    				}
    				else if (mirror::Enum* enm = type->asEnum())
    				{
    					if (enm->getSubType() != nullptr)
    					{
    						ImGui::Text("subtype: %s", enm->getSubType()->getName());
    					}
    					for (const mirror::EnumValue* value : enm->getValues())
    					{
    						ImGui::BulletText("%s (%lld)", value->getName(), value->getValue());
    					}
    				}
    			}
    		}
    		ImGui::EndChild();
    	}
    	ImGui::End();

		changedSettings = changedSettings || (previousOpen != editorInstance->showMirrorDebugWindow);
    }

    if (editorInstance->showDemoWindow)
    {
    	bool previousOpen = editorInstance->showDemoWindow;
		ImGui::ShowDemoWindow(&editorInstance->showDemoWindow);
		changedSettings = changedSettings || (previousOpen != editorInstance->showDemoWindow);
    }

    if (changedSettings)
    {
    	app().saveSettings();
    }
}

bool onSerializeApplicationSettings(Application* _application, Serializer* _serializer)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");
	return serialization::serializeMirrorType(_serializer, *editorInstance, "editor");
}
