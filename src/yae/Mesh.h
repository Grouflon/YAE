#pragma once

#include <yae/types.h>
#include <yae/containers/Array.h>
#include <yae/rendering/render_types.h>

namespace yae {

struct YAE_API Mesh
{
	Mesh(Allocator* _allocator = nullptr) : vertices(_allocator), indices(_allocator) {}

	static bool LoadFromObjFile(Mesh& _mesh, const char* _objFilePath);

	DataArray<Vertex> vertices;
	DataArray<u32> indices;
};

} // namespace yae
