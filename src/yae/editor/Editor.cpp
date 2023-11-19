#include "Editor.h"

#include <core/memory.h>
#include <core/program.h>
#include <core/serialization/serialization.h>
#include <core/serialization/Serializer.h>
#include <core/string.h>

#include <yae/im3d_extension.h>
#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>
#include <yae/math_3d.h>
#include <yae/rendering/Renderer.h>
#include <yae/ResourceManager.h>
#include <yae/resource.h>
#include <yae/resources/Mesh.h>
#include <yae/resources/Resource.h>
#include <yae/resources/ShaderFile.h>
#include <yae/resources/ShaderProgram.h>

#include <imgui/imgui.h>
#include <im3d/im3d.h>
#include <mirror/mirror.h>

namespace yae {
namespace editor {

void Editor::reload()
{
	resourceEditor.reload();
}

void Editor::init()
{	
	{
		wireframeShader = resource::findOrCreate<ShaderProgram>("wireframeShader");

#if YAE_PLATFORM_WEB == 0
		Shader* shaders[] =
		{
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_vert.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_geom.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/wireframe_frag.res")
		};

		wireframeShader->setShaderStages(shaders, countof(shaders));
#endif

		wireframeShader->load();
		//YAE_ASSERT(wireframeShader->isLoaded());
	}

	{
		normalsShader = resource::findOrCreate<ShaderProgram>("normalsShader");

#if YAE_PLATFORM_WEB == 0
		Shader* shaders[] =
		{
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_vert.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_geom.res"),
			resource::findOrCreateFromFile<ShaderFile>("./data/shaders/normals_frag.res")
		};

		normalsShader->setShaderStages(shaders, countof(shaders));
		normalsShader->setPrimitiveMode(PrimitiveMode::POINTS);
#endif
		
		normalsShader->load();
		//YAE_ASSERT(normalsShader->isLoaded());
	}

	resourceEditor.init();

	RenderScene* scene = renderer().createScene("editorScene");
	RenderCamera* camera = renderer().createCamera("editorCamera");
	scene->addCamera(camera);
}

void Editor::shutdown()
{
	renderer().destroyCamera("editorCamera");	
	renderer().destroyScene("editorScene");

	resourceEditor.shutdown();

	wireframeShader->unload();
	wireframeShader = nullptr;

	normalsShader->unload();
	normalsShader = nullptr;
}

void Editor::update(float _dt)
{
	bool changedSettings = false;

	if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
        	if (ImGui::MenuItem("Exit"))
        	{
        		app().requestStop();
        	}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Display"))
        {
	        changedSettings = ImGui::MenuItem("Resources", NULL, &resourceEditor.opened) || changedSettings;

        	if (ImGui::BeginMenu("Profiling"))
	        {
	        	changedSettings = ImGui::MenuItem("Memory", NULL, &showMemoryProfiler) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Frame rate", NULL, &showFrameRate) || changedSettings;

	            ImGui::EndMenu();
	        }

	        if (ImGui::BeginMenu("Debug"))
	        {
	        	changedSettings = ImGui::MenuItem("Input", NULL, &inputInspector.opened) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Mirror", NULL, &showMirrorDebugWindow) || changedSettings;
	        	changedSettings = ImGui::MenuItem("Renderer", NULL, &showRendererDebugWindow) || changedSettings;
	            ImGui::EndMenu();
	        }

	        if (ImGui::BeginMenu("Misc"))
	        {
	        	changedSettings = ImGui::MenuItem("Mirror Inspector", NULL, &mirrorInspector.opened) || changedSettings;
	        	changedSettings = ImGui::MenuItem("ImGui Demo Window", NULL, &showDemoWindow) || changedSettings;
	            ImGui::EndMenu();
	        }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    changedSettings = changedSettings || resourceEditor.update();
    changedSettings = changedSettings || mirrorInspector.update();
    changedSettings = changedSettings || inputInspector.update();

    if (showMemoryProfiler)
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

    	bool previousOpen = showMemoryProfiler;
    	if (ImGui::Begin("Memory Profiler", &showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize))
    	{
    		showAllocatorInfo("Default", defaultAllocator());
	    	showAllocatorInfo("Scratch", scratchAllocator());
	    	showAllocatorInfo("Tool", toolAllocator());
	    	showAllocatorInfo("Malloc", mallocAllocator());
    	}
    	ImGui::End();
		changedSettings = changedSettings || (previousOpen != showMemoryProfiler);
    }

    if (showFrameRate)
    {
    	bool previousOpen = showFrameRate;
    	if (ImGui::Begin("Frame Rate", &showFrameRate, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs))
    	{
    		ImGui::Text("dt: %.2f ms", _dt*1000.f);
    		ImGui::Text("%.2f fps", _dt != 0.f ? (1.f/_dt) : 0.f);	
    	}
    	ImGui::End();
		changedSettings = changedSettings || (previousOpen != showFrameRate);
    }

    if (showRendererDebugWindow)
    {
    	bool previousOpen = showRendererDebugWindow;
		if (ImGui::Begin("Renderer", &showRendererDebugWindow))
    	{
    		/*
    		ImGui::Text("view matrix:");
			imgui_matrix4(renderer().m_viewMatrix);
    		ImGui::Text("projection matrix:");
			imgui_matrix4(renderer().m_projMatrix);
			*/
			ImGui::Text("TGaa");
    	}
    	ImGui::End();

		changedSettings = changedSettings || (previousOpen != showRendererDebugWindow);
    }

    if (showMirrorDebugWindow)
    {
    	bool previousOpen = showMirrorDebugWindow;
		if (ImGui::Begin("Mirror", &showMirrorDebugWindow))
    	{
    		ImVec2 regionSize = ImVec2(ImGui::GetContentRegionAvail().x, 0);
    		ImGui::BeginChild("MirrorChildL", ImVec2(regionSize.x * 0.5f, regionSize.y), false);
    		for (mirror::Type* type : mirror::GetTypeSet().getTypes())
    		{
				_displayTypeTreeNode(type, nullptr);
    		}
    		ImGui::EndChild();

    		ImGui::SameLine();

    		ImGui::BeginChild("MirrorChildR", ImVec2(0, regionSize.y), true);
    		if (selectedTypeID != mirror::UNDEFINED_TYPEID)
    		{
    			mirror::Type* type = mirror::FindTypeByID(selectedTypeID);
    			if (type != nullptr)
    			{
    				ImGui::Text("name: %s", type->getName());
    				ImGui::Text("typeInfo: %s", mirror::TypeInfoToString(type->getTypeInfo()));
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
    							mirror::Type* memberType = member->getType();
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

		changedSettings = changedSettings || (previousOpen != showMirrorDebugWindow);
    }

    if (showDemoWindow)
    {
    	bool previousOpen = showDemoWindow;
		ImGui::ShowDemoWindow(&showDemoWindow);
		changedSettings = changedSettings || (previousOpen != showDemoWindow);
    }

    if (changedSettings)
    {
    	app().saveSettings();
    }

    // RELOAD
    if (input().isCtrlDown() && input().isKeyDown(SDL_SCANCODE_R))
    {
    	program().reloadModule("yae");
    }

    // EXIT PROGRAM
	if (input().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		app().requestStop();
	}
}

bool Editor::serialize(Serializer* _serializer)
{
	return serialization::serializeMirrorType(_serializer, *this, "editor");
}

void Editor::_displayTypeTreeNode(mirror::Type* _type, mirror::Type* _parent)
{
	mirror::TypeID id = _type->getTypeID();
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selectedTypeID == id ? ImGuiTreeNodeFlags_Selected : 0);
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
		selectedTypeID = id;
	}
    if (hasChildren && isNodeOpen)
    {
    	for (mirror::TypeID childId : _type->asClass()->getChildren())
    	{
    		mirror::Type* childType = mirror::GetTypeSet().findTypeByID(childId);
    		if (childType == nullptr)
    			continue;

    		_displayTypeTreeNode(childType, _type);
    	}
    	
    	ImGui::TreePop();
    }
}

} // namespace editor
} // namespace yae

MIRROR_CLASS(yae::editor::Editor)
(
	MIRROR_MEMBER(showMemoryProfiler);
	MIRROR_MEMBER(showFrameRate);
	MIRROR_MEMBER(showRendererDebugWindow);
	MIRROR_MEMBER(showMirrorDebugWindow);
	MIRROR_MEMBER(showDemoWindow);

	MIRROR_MEMBER(resourceEditor);
	MIRROR_MEMBER(inputInspector);
	MIRROR_MEMBER(mirrorInspector);
);
