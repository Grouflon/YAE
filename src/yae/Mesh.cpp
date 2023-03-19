#include "Mesh.h"

#include <yae/containers/HashMap.h>
#include <yae/hash.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace yae {

bool Mesh::LoadFromObjFile(Mesh& _mesh, const char* _objFilePath)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_mesh.vertices.size() == 0);
	YAE_ASSERT(_mesh.indices.size() == 0);

	tinyobj::ObjReader reader;
	{
		YAE_CAPTURE_SCOPE("open_file");
		if (!reader.ParseFromFile(_objFilePath))
		{

			if (reader.Warning().size() > 0)
			{
				YAE_WARNINGF_CAT("Resource", "%s", reader.Warning().c_str());
			}
			if (reader.Error().size() > 0)
			{
				YAE_ERRORF_CAT("Resource", "%s", reader.Error().c_str());
			}
			YAE_ERRORF_CAT("Resource", "Failed to load obj file \"%s\"", _objFilePath);
			return false;
		}
	}
	
	{
		YAE_CAPTURE_SCOPE("remove_duplicates");
		
		yae::HashMap<u32, u32> uniqueVertices(&yae::scratchAllocator());
		for (const auto& shape : reader.GetShapes())
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex v{};
				v.pos = {
					reader.GetAttrib().vertices[3 * index.vertex_index + 0],
					reader.GetAttrib().vertices[3 * index.vertex_index + 1],
					reader.GetAttrib().vertices[3 * index.vertex_index + 2]
				};

				v.texCoord = {
					reader.GetAttrib().texcoords[2 * index.texcoord_index + 0],
					1.f - reader.GetAttrib().texcoords[2 * index.texcoord_index + 1]
				};

				v.color = {1.f, 1.f, 1.f};

				u32 vertexHash = hash::hash32(&v, sizeof(Vertex));
				const u32* uniqueVertexIndexPtr = uniqueVertices.get(vertexHash);
				if (uniqueVertexIndexPtr == nullptr)
				{
					uniqueVertexIndexPtr = &uniqueVertices.set(vertexHash, _mesh.vertices.size());
					_mesh.vertices.push_back(v);
				}
				_mesh.indices.push_back(*uniqueVertexIndexPtr);
			}
		}
	}
	return true;
}


} // namespace yae
