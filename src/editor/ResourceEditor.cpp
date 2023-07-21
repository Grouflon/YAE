#include "ResourceEditor.h"

#include <yae/Application.h>
#include <yae/math_3d.h>
#include <yae/ResourceManager.h>
#include <yae/resources/Resource.h>
#include <yae/resources/Mesh.h>
#include <yae/resources/ShaderProgram.h>
#include <yae/rendering/Renderer.h>
#include <yae/resource.h>
#include <yae/imgui_extension.h>
#include <yae/InputSystem.h>
#include <yae/inline_string.h>
#include <yae/string.h>
#include <yae/im3d_extension.h>

#include <imgui/imgui.h>
#include <im3d/im3d.h>

namespace yae {
namespace editor {

MIRROR_CLASS_DEFINITION(ResourceEditor);

bool inspectResourceMembers(Resource* _resource)
{
	bool modified = false;
	if (ImGui::EditMirrorClassInstance(_resource, _resource->getClass()))
	{
		modified = true;
		if (!_resource->m_transient)
		{
			// NOTE: I don't know how to make a saving achitecture better than that yet
			// Maybe it will make some sense after having done several implementations
			// Maybe the disk file should be part of every resource, just as the transient flag (should replace the transient flag even ? that way any resource knows where it comes from)
			resource::saveToFile(_resource, _resource->getName());
		}
		resourceManager().flagResourceForReload(_resource);
	}
	return modified;
}

bool defaultResourceInspectorUpdate(Resource* _resource, void*)
{
	String128 str = string::format("%s: %s", _resource->getClass()->getName(), _resource->getName());
	bool opened = true;
	ImGui::SetNextWindowSize(ImVec2(300,200), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(str.c_str(), &opened, ImGuiWindowFlags_NoSavedSettings))
	{
		inspectResourceMembers(_resource);
	}
	ImGui::End();
	return opened;
}

class MeshInspector
{
public:
	RenderTarget* renderTarget = nullptr;
	RenderScene* scene = nullptr;
	RenderCamera* camera = nullptr;
	ShaderProgram* wireframeShader = nullptr;
	ShaderProgram* normalsShader = nullptr;
	float cameraYaw = 145.0f;
	float cameraPitch = 20.0f;
	float armLength = 5.0f;
	bool showNormals = false;
};

void* meshInspectorInit(Resource* _resource)
{
	Mesh* mesh = mirror::Cast<Mesh*>(_resource);
	YAE_ASSERT(mesh != nullptr);

	MeshInspector* inspector = toolAllocator().create<MeshInspector>();
	inspector->renderTarget = renderer().createRenderTarget(false, 128, 128);
	inspector->scene = renderer().createScene(mesh->getName());
	inspector->camera = renderer().createCamera(mesh->getName());
	inspector->scene->addCamera(inspector->camera);
	inspector->camera->renderTarget = inspector->renderTarget;
	inspector->camera->clearColor = Vector4(.1f, .1f, .1f, 1.f);

	inspector->wireframeShader = resource::find<ShaderProgram>("wireframeShader");
	YAE_ASSERT(inspector->wireframeShader != nullptr);
	if (inspector->wireframeShader)
	{
		inspector->wireframeShader->load();
	}

	inspector->normalsShader = resource::find<ShaderProgram>("normalsShader");
	YAE_ASSERT(inspector->normalsShader != nullptr);
	if (inspector->normalsShader)
	{
		inspector->normalsShader->load();
	}

	return inspector;
}

bool meshInspectorUpdate(Resource* _resource, void* _userData)
{
	Mesh* mesh = mirror::Cast<Mesh*>(_resource);
	YAE_ASSERT(mesh != nullptr);
	MeshInspector* inspector = (MeshInspector*)_userData;
	YAE_ASSERT(inspector != nullptr);

	// ImGui
	String name(string::format("Mesh: %s", mesh->getName()), &scratchAllocator());
	ImGui::SetNextWindowSize(ImVec2(700.f, 400.f), ImGuiCond_FirstUseEver);
	bool opened = true;
	if (ImGui::Begin(name.c_str(), &opened, ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Display"))
			{
        		ImGui::MenuItem("Show Normals", NULL, &inspector->showNormals);

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::BeginChild("MeshInspectorL", ImVec2(200.f, 0.f));
		{
			inspectResourceMembers(_resource);
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild("MeshInspectorR");
		{
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			renderer().resizeRenderTarget(inspector->renderTarget, windowSize.x , windowSize.y);
			ImGui::Image((void*)inspector->renderTarget->m_renderTexture, windowSize);
			if (ImGui::IsItemHovered())
			{
				float rotationSpeed = 0.75f;
				float zoomSpeed = 0.2f;

				if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
				{
					ImVec2 delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
					ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

					inspector->cameraYaw += rotationSpeed * delta.x;
					inspector->cameraPitch += rotationSpeed * delta.y;
				}

				inspector->armLength = math::max(inspector->armLength + ImGui::GetIO().MouseWheel * -zoomSpeed, 0.0f);
			}
			else
			{
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();

	// Scene
	renderer().pushScene(inspector->scene);
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

		renderer().drawMesh(Matrix4::IDENTITY, mesh, inspector->wireframeShader, nullptr);

		if (inspector->showNormals)
		{
			renderer().drawMesh(Matrix4::IDENTITY, mesh, inspector->normalsShader, nullptr);
		}
	}
	renderer().popScene();

	inspector->camera->rotation = Quaternion::FromEuler(D2R * inspector->cameraPitch, D2R * inspector->cameraYaw, 0.0f);
	inspector->camera->position = -(inspector->camera->rotation * (Vector3::FORWARD * inspector->armLength));

	return opened;
}

void meshInspectorShutdown(Resource* _resource, void* _userData)
{
	MeshInspector* inspector = (MeshInspector*)_userData;
	Mesh* mesh = mirror::Cast<Mesh*>(_resource);
	YAE_ASSERT(mesh != nullptr);

	if (inspector->wireframeShader != nullptr)
	{
		inspector->wireframeShader->release();
	}

	if (inspector->normalsShader != nullptr)
	{
		inspector->normalsShader->release();
	}

	renderer().destroyRenderTarget(inspector->renderTarget);
	renderer().destroyScene(inspector->scene);
	renderer().destroyCamera(inspector->camera);
	toolAllocator().destroy(inspector);
}

void ResourceEditor::init()
{
	_registerInspectorDefinitions();
}

void ResourceEditor::shutdown()
{
	DataArray<ResourceInspector> tempArray(m_inspectors, &scratchAllocator());
	for (const ResourceInspector& inspector : m_inspectors)
	{
		_closeInspector(inspector.resource);
	}
}

void ResourceEditor::reload()
{
	// NOTE: Storing functions pointers like this doesn't seem to do well with hot-reload, as I expected.
	// Using virtual inspectors, reflection and factories would probably circumvent the problem but meh.
	// We'll just re-register after reload for now. It's a bit manual, but not that much since once written once it doesn't add any overhead to adding new types

	m_inspectorDefinitions.clear();
	_registerInspectorDefinitions();
}

void ResourceEditor::update()
{
	if (opened)
    {
    	if (ImGui::Begin("Resources", &opened))
    	{
    		ResourceManager& rm = resourceManager();
    		for (Resource* resource : rm.getResources())
    		{
    			char buf[256];
    			string::format(buf, sizeof(buf), "%010u|%s", resource->getID(), resource->getName());
	            if (ImGui::Selectable(buf, resource == m_currentResource))
	            {
	            	_openInspector(resource);
	            }
    		}
  
    	}
    	ImGui::End();

    	DataArray<ResourceInspector> tempArray(m_inspectors, &scratchAllocator());
    	for (const ResourceInspector& inspector : m_inspectors)
    	{
    		ResourceInspectorDefinition definition = _getInspectorDefinition(inspector.resource);
    		if (definition.updateFunction != nullptr)
    		{
    			if (!definition.updateFunction(inspector.resource, inspector.userData))
    			{
    				_closeInspector(inspector.resource);
    			}
    		}
    	}
    }
}

void ResourceEditor::_registerInspectorDefinitions()
{
	_registerInspectorDefinition(mirror::GetTypeID<Resource>(), {nullptr, &defaultResourceInspectorUpdate, nullptr});
	_registerInspectorDefinition(mirror::GetTypeID<Mesh>(), {&meshInspectorInit, &meshInspectorUpdate, &meshInspectorShutdown});
}

void ResourceEditor::_registerInspectorDefinition(mirror::TypeID _type, const ResourceInspectorDefinition& _inspector)
{
	YAE_ASSERT(!m_inspectorDefinitions.has(_type));
	m_inspectorDefinitions.set(_type, _inspector);
}

void ResourceEditor::_unregisterInspectorDefinition(mirror::TypeID _type)
{
	YAE_ASSERT(m_inspectorDefinitions.has(_type));
	m_inspectorDefinitions.remove(_type);
}

ResourceInspectorDefinition ResourceEditor::_getInspectorDefinition(Resource* _resource) const
{
	mirror::Class* clss = _resource->getClass();
	do
	{
		const ResourceInspectorDefinition* definition = m_inspectorDefinitions.get(clss->getTypeID());
		if (definition != nullptr)
			return *definition;

		clss = mirror::AsClass(clss->getParent());
	} while (clss != nullptr);

	return ResourceInspectorDefinition();
}

void ResourceEditor::_openInspector(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	if (m_inspectors.find([](const ResourceInspector& _inspector, void* _resource) { return _inspector.resource == _resource; }, _resource) != nullptr)
		return;

	ResourceInspectorDefinition definition = _getInspectorDefinition(_resource);

	ResourceInspector inspector;
	inspector.resource = _resource;
	inspector.userData = definition.initFunction != nullptr ? definition.initFunction(_resource) : nullptr;

	m_inspectors.push_back(inspector);
}

void ResourceEditor::_closeInspector(Resource* _resource)
{
	YAE_ASSERT(_resource != nullptr);
	ResourceInspector* inspectorPtr = m_inspectors.find([](const ResourceInspector& _inspector, void* _resource) { return _inspector.resource == _resource; }, _resource);
	if (inspectorPtr == nullptr)
		return;

	ResourceInspectorDefinition definition = _getInspectorDefinition(_resource);
	if (definition.shutdownFunction != nullptr) definition.shutdownFunction(_resource, inspectorPtr->userData);

	m_inspectors.erase(inspectorPtr);
}


} // namespace editor
} // namespace yae