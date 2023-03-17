#include "editor.h"

#include <yae/memory.h>
#include <yae/Application.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/Serializer.h>
#include <yae/imgui_extension.h>
#include <yae/rendering/Renderer.h>

#include <imgui/imgui.h>
#include <mirror/mirror.h>

using namespace yae;

struct EditorInstance
{
	bool showMemoryProfiler = false;
	bool showFrameRate = false;
	bool showMirrorDebugWindow = false;
	bool showRendererDebugWindow = false;
	bool showDemoWindow = false;

	// mirror window
	mirror::TypeID selectedTypeID = mirror::UNDEFINED_TYPEID;

	MIRROR_CLASS_NOVIRTUAL(EditorInstance)
	(
		MIRROR_MEMBER(showMemoryProfiler)();
		MIRROR_MEMBER(showFrameRate)();
		MIRROR_MEMBER(showRendererDebugWindow)();
		MIRROR_MEMBER(showMirrorDebugWindow)();
		MIRROR_MEMBER(showDemoWindow)();
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

        if (ImGui::BeginMenu("Profiling"))
        {
        	changedSettings = ImGui::MenuItem("Memory", NULL, &editorInstance->showMemoryProfiler) || changedSettings;
        	changedSettings = ImGui::MenuItem("Frame rate", NULL, &editorInstance->showFrameRate) || changedSettings;

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
        	changedSettings = ImGui::MenuItem("Mirror", NULL, &editorInstance->showMirrorDebugWindow) || changedSettings;
        	changedSettings = ImGui::MenuItem("Renderer", NULL, &editorInstance->showRendererDebugWindow) || changedSettings;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Misc"))
        {
        	changedSettings = ImGui::MenuItem("ImGui Demo Window", NULL, &editorInstance->showDemoWindow) || changedSettings;
            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();
    }

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

    if (editorInstance->showRendererDebugWindow)
    {
    	bool previousOpen = editorInstance->showRendererDebugWindow;
		if (ImGui::Begin("Renderer", &editorInstance->showRendererDebugWindow))
    	{
    		ImGui::Text("view matrix:");
			imgui_matrix4(renderer().m_viewMatrix);
    		ImGui::Text("projection matrix:");
			imgui_matrix4(renderer().m_projMatrix);
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
