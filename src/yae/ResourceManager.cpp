#include "ResourceManager.h"

#include <cstring>

namespace yae {

void ResourceManager2::registerMesh(const char* _name, Mesh* _mesh)
{
	MeshResource meshResource;

	size_t nameLength = strlen(_name);
	YAE_ASSERT(nameLength < countof(meshResource.name));
	strncpy(meshResource.name, _name, math::min(nameLength, countof(meshResource.name)));
	meshResource.name[countof(meshResource.name) - 1] = 0;

	YAE_ASSERT(_mesh != nullptr);
	meshResource.mesh = _mesh;

	StringHash hash = StringHash(_name);
	YAE_ASSERT(m_meshesByName.get(hash) == nullptr);
	m_meshesByName.set(hash, meshResource);
	m_meshes.push_back(meshResource);
}

void ResourceManager2::unregisterMesh(const char* _name)
{
	StringHash hash = StringHash(_name);
	MeshResource* meshResourcePtr = m_meshesByName.get(hash);
	YAE_ASSERT(meshResourcePtr != nullptr);
	for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		if (it->mesh == meshResourcePtr->mesh)
		{
			m_meshes.erase(it);
			break;
		}
	}
	m_meshesByName.remove(hash);
}

Mesh* ResourceManager2::getMesh(const char* _name) const
{
	const MeshResource* meshResourcePointer = m_meshesByName.get(StringHash(_name));
	return meshResourcePointer != nullptr ? meshResourcePointer->mesh : nullptr;
}

const DataArray<MeshResource>& ResourceManager2::getMeshResources() const
{
	return m_meshes;
}

} // namespace yae
