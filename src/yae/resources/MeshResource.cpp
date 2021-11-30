#include "MeshResource.h"

#include <yae/filesystem.h>
#include <yae/vulkan/VulkanRenderer.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include <set>
#include <vector>
#include <unordered_map>

namespace yae {

MIRROR_CLASS_DEFINITION(MeshResource);

MeshResource::MeshResource(const char* _path)
	: Resource(filesystem::normalizePath(_path).c_str())
{

}


MeshResource::~MeshResource()
{

}


void MeshResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	{
		YAE_CAPTURE_SCOPE("open_file");
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, getName()))
		{
			if (warn.size() > 0)
			{
				_log(RESOURCELOGTYPE_WARNING, warn.c_str());
			}
			if (err.size() > 0)
			{
				_log(RESOURCELOGTYPE_ERROR, err.c_str());
			}
			return;
		}	
	}
	
	{
		YAE_CAPTURE_SCOPE("remove_duplicates");
		std::unordered_map<Vertex, u32> uniqueVertices;

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex v{};
				v.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				v.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				v.color = {1.f, 1.f, 1.f};

				auto it = uniqueVertices.find(v);
				if (it == uniqueVertices.end())
				{
					it = uniqueVertices.insert(std::make_pair(v, u32(m_vertices.size()))).first;
					m_vertices.push_back(v);
				}
				m_indices.push_back(it->second);
			}
		}
	}

	bool result = renderer().createMesh(m_vertices.data(), m_vertices.size(), m_indices.data(), m_indices.size(), m_meshHandle);
	if (!result)
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create mesh with the renderer");
		return;
	}
}


void MeshResource::_doUnload()
{
	YAE_CAPTURE_FUNCTION();
	
	renderer().destroyMesh(m_meshHandle);

	m_vertices.clear();
	m_indices.clear();
}

} // namespace yae
