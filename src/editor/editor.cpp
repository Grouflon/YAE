#include "editor.h"

#include <yae/memory.h>
#include <yae/Application.h>
#include <yae/serialization/serialization.h>
#include <yae/serialization/Serializer.h>
#include <yae/imgui_extension.h>
#include <yae/rendering/Renderer.h>
#include <yae/ResourceManager.h>
#include <yae/program.h>
#include <yae/Mesh.h>
#include <yae/math_3d.h>
#include <yae/im3d_extension.h>

#include <imgui/imgui.h>
#include <im3d/im3d.h>
#include <mirror/mirror.h>

using namespace yae;

class MeshInspector
{
public:
	String meshName;
	bool opened = true;
	RenderTarget* renderTarget = nullptr;
};

struct EditorInstance
{
	bool showResources = false;
	bool showMemoryProfiler = false;
	bool showFrameRate = false;
	bool showMirrorDebugWindow = false;
	bool showRendererDebugWindow = false;
	bool showDemoWindow = false;

	// resource inspector
	DataArray<MeshInspector*> meshInspectors;

	// mirror window
	mirror::TypeID selectedTypeID = mirror::UNDEFINED_TYPEID;

	MIRROR_CLASS_NOVIRTUAL(EditorInstance)
	(
		MIRROR_MEMBER(showResources)();
		MIRROR_MEMBER(showMemoryProfiler)();
		MIRROR_MEMBER(showFrameRate)();
		MIRROR_MEMBER(showRendererDebugWindow)();
		MIRROR_MEMBER(showMirrorDebugWindow)();
		MIRROR_MEMBER(showDemoWindow)();
	);
};
MIRROR_CLASS_DEFINITION(EditorInstance);

MeshInspector* openMeshInspector(EditorInstance& _editorInstance, const char* _meshName)
{
	MeshInspector* inspector = nullptr;

	for (MeshInspector* meshInspector : _editorInstance.meshInspectors)
	{
		if (meshInspector->meshName == _meshName)
		{
			inspector = meshInspector;
			break;
		}
	}

	if (inspector == nullptr)
	{
		inspector = toolAllocator().create<MeshInspector>();
		inspector->meshName = _meshName;
		inspector->renderTarget = renderer().createRenderTarget(false, 8, 8);
		RenderScene* scene = renderer().createScene(_meshName);
		RenderCamera* camera = renderer().createCamera(_meshName);
		scene->addCamera(camera);
		camera->renderTarget = inspector->renderTarget;
		camera->clearColor = Vector4(.1f, .1f, .1f, 1.f);
		_editorInstance.meshInspectors.push_back(inspector);
	}


	return inspector;
}

void closeMeshInspector(EditorInstance& _editorInstance, MeshInspector* _meshInspector)
{
	auto it = _editorInstance.meshInspectors.find(_meshInspector);
	YAE_ASSERT(it != nullptr);
	_editorInstance.meshInspectors.erase(it);

	renderer().destroyRenderTarget(_meshInspector->renderTarget);
	renderer().destroyScene(_meshInspector->meshName.c_str());
	renderer().destroyCamera(_meshInspector->meshName.c_str());
	toolAllocator().destroy(_meshInspector);
}

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

	for (MeshInspector* meshInspector : editorInstance->meshInspectors)
	{
		toolAllocator().destroy(meshInspector);
	}

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
	        changedSettings = ImGui::MenuItem("Resources", NULL, &editorInstance->showResources) || changedSettings;

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

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (editorInstance->showResources)
    {
    	if (ImGui::Begin("Resources", &editorInstance->showResources, ImGuiWindowFlags_AlwaysAutoResize))
    	{
    		ResourceManager2& rm = program().resourceManager2();
    		for (const MeshResource& meshResource : rm.getMeshResources())
    		{
	            if (ImGui::Selectable(meshResource.name, false))
	            {
	            	openMeshInspector(*editorInstance, meshResource.name);
	            }
	            ImGui::SameLine(150);
	            ImGui::Text("%d vertices", meshResource.mesh->vertices.size());
	            ImGui::SameLine(300);
	            ImGui::Text("%d indices", meshResource.mesh->indices.size());
    		}
    	}
    	ImGui::End();

    	DataArray<MeshInspector*> tempInspectors(&scratchAllocator());
    	tempInspectors = editorInstance->meshInspectors;
    	for (MeshInspector* meshInspector : tempInspectors)
    	{
    		Mesh* mesh = program().resourceManager2().getMesh(meshInspector->meshName.c_str());
    		YAE_ASSERT(mesh != nullptr);

    		RenderCamera* camera = renderer().getCamera(meshInspector->meshName.c_str());
    		Vector3 cameraArm = Vector3::FORWARD * -4.f + Vector3::RIGHT * -4.f + Vector3::UP * 2.f; 
			camera->position = Quaternion::FromAngleAxis(app().getTime() * PI * 0.2f, Vector3::UP) * cameraArm;
			Matrix4 cameraTransform = math::inverse(Matrix4::FromLookAt(camera->position, Vector3::ZERO, -Vector3::UP));
			camera->rotation = Quaternion::FromMatrix4(cameraTransform);

    		renderer().pushScene(meshInspector->meshName.c_str());
    		{
				Im3d::SetSize(4.0f);
    			Im3d::DrawRotation(Quaternion::IDENTITY, 1.5f);

    			static int gridSize = 20;
				const float gridHalf = (float)gridSize * 0.5f;
				Im3d::SetAlpha(1.0f);
				Im3d::SetSize(2.0f);
				Im3d::BeginLines();
					for (int x = 0; x <= gridSize; ++x)
					{
						Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
						Im3d::Vertex( gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(1.0f, 0.0f, 0.0f));
					}
					for (int z = 0; z <= gridSize; ++z)
					{
						Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf,  Im3d::Color(0.0f, 0.0f, 0.0f));
						Im3d::Vertex((float)z - gridHalf, 0.0f,  gridHalf,  Im3d::Color(0.0f, 0.0f, 1.0f));
					}
				Im3d::End();

				renderer().drawMesh(Matrix4::IDENTITY, *mesh, 0, 0);
    		}
    		renderer().popScene();

    		String name(&scratchAllocator());
    		name = "Mesh: " + meshInspector->meshName;
    		if (ImGui::Begin(name.c_str(), &meshInspector->opened))
    		{
    			ImVec2 windowSize = ImGui::GetContentRegionAvail();
    			renderer().resizeRenderTarget(meshInspector->renderTarget, windowSize.x , windowSize.y);
    			ImGui::Image((void*)meshInspector->renderTarget->m_renderTexture, windowSize);
    		}
    		ImGui::End();

    		if (!meshInspector->opened)
    		{
    			closeMeshInspector(*editorInstance, meshInspector);
    		}
    	}
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
