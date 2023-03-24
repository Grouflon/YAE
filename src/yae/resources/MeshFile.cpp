#include "MeshFile.h"

#include <yae/filesystem.h>
#include <yae/hash.h>
#include <yae/containers/HashMap.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

namespace yae {

MIRROR_CLASS_DEFINITION(MeshFile);

MeshFile::MeshFile()
{

}


MeshFile::~MeshFile()
{

}

void MeshFile::setPath(const char* _path)
{
	YAE_ASSERT(!isLoaded());
	m_path = filesystem::normalizePath(_path);
}

const char* MeshFile::getPath() const
{
	return m_path.c_str();
}

void MeshFile::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_vertices.size() == 0);
	YAE_ASSERT(m_indices.size() == 0);

	tinyobj::ObjReader reader;
	{
		YAE_CAPTURE_SCOPE("open_file");
		if (!reader.ParseFromFile(m_path.c_str()))
		{
			if (reader.Warning().size() > 0)
			{
				_log(RESOURCELOGTYPE_WARNING, reader.Warning().c_str());
			}
			if (reader.Error().size() > 0)
			{
				_log(RESOURCELOGTYPE_ERROR, reader.Error().c_str());
			}
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
					uniqueVertexIndexPtr = &uniqueVertices.set(vertexHash, m_vertices.size());
					m_vertices.push_back(v);
				}
				m_indices.push_back(*uniqueVertexIndexPtr);
			}
		}
	}

	Mesh::_doLoad();
}


void MeshFile::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	Mesh::_doUnload();

	m_vertices.resize(0);
	m_indices.resize(0);
}

} // namespace yae
