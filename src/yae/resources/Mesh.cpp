#include "Mesh.h"

MIRROR_CLASS(yae::Mesh)
(
	MIRROR_PARENT(yae::Resource)
);

namespace yae {

Mesh::Mesh()
{
}


Mesh::~Mesh()
{
}

void Mesh::setVertices(const Vertex* _vertices, u32 _vertexCount)
{
	YAE_ASSERT(!isLoaded());
	m_vertices.resize(_vertexCount);
	memcpy(m_vertices.data(), _vertices, sizeof(Vertex) * _vertexCount);
}

const BaseArray<Vertex>& Mesh::getVertices() const
{
	return m_vertices;
}

void Mesh::setIndices(const u32* _indices, u32 _indexCount)
{
	YAE_ASSERT(!isLoaded());
	m_indices.resize(_indexCount);
	memcpy(m_indices.data(), _indices, sizeof(u32) * _indexCount);
}

const BaseArray<u32>& Mesh::getIndices() const
{
	return m_indices;
}

void Mesh::_doLoad()
{
}


void Mesh::_doUnload()
{
}

} // namespace yae
