#include "ResourceManager.h"

#include <yae/string.h>

#include <cstring>

namespace yae {

void ResourceManager2::registerMesh(const char* _name, Mesh* _mesh)
{
	MeshResource meshResource;

	YAE_VERIFY(string::safeCopyToBuffer(meshResource.name, _name, countof(meshResource.name)) < countof(meshResource.name));

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
