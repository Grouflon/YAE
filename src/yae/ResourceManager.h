#pragma once

#include <yae/types.h>
#include <yae/containers/HashMap.h>
#include <yae/hash.h>

namespace yae {

struct Mesh;

struct MeshResource
{
	char name[256] = {};
	Mesh* mesh;
};

class YAE_API ResourceManager2
{
public:
	ResourceManager2() {}

	void registerMesh(const char* _name, Mesh* _mesh);
	void unregisterMesh(const char* _name);
	Mesh* getMesh(const char* _name) const;
	const DataArray<MeshResource>& getMeshResources() const;

//private:
	HashMap<StringHash, MeshResource> m_meshesByName;
	DataArray<MeshResource> m_meshes;
};

} // namespace yae
