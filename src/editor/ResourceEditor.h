#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>

#include <mirror/mirror.h>

namespace yae {

class RenderTarget;

namespace editor {

typedef void* (*ResourceInspectorInitFunction)(Resource* _resource);
typedef bool (*ResourceInspectorUpdateFunction)(Resource* _resource, void* _userData);
typedef void (*ResourceInspectorShutdownFunction)(Resource* _resource, void* _userData);

struct ResourceInspectorDefinition
{
	ResourceInspectorInitFunction initFunction = nullptr;
	ResourceInspectorUpdateFunction updateFunction = nullptr;
	ResourceInspectorShutdownFunction shutdownFunction = nullptr;
};

class ResourceEditor
{
public:
	void init();
	void shutdown();
	void reload();

	void update();

	bool opened = false;
//private:
	void _registerInspectorDefinitions();

	void _registerInspectorDefinition(mirror::TypeID _type, const ResourceInspectorDefinition& _inspector);
	void _unregisterInspectorDefinition(mirror::TypeID _type);
	ResourceInspectorDefinition _getInspectorDefinition(Resource* _resource) const;

	void _openInspector(Resource* _resource);
	void _closeInspector(Resource* _resource);

	Resource* m_currentResource = nullptr;
	RenderTarget* m_renderTarget = nullptr;
	HashMap<mirror::TypeID, ResourceInspectorDefinition> m_inspectorDefinitions;

	struct ResourceInspector
	{
		Resource* resource;
		void* userData;
	};
	DataArray<ResourceInspector> m_inspectors;

	MIRROR_CLASS_NOVIRTUAL(ResourceEditor)
	(
		MIRROR_MEMBER(opened)();
	);
};

} // namespace editor
} // namespace yae