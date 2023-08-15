#pragma once

#include <yae/types.h>
#include <yae/resources/Resource.h>
#include <yae/containers/Array.h>
#include <yae/rendering/render_types.h>

namespace yae {

class YAE_API Mesh : public Resource
{
	MIRROR_GETCLASS_VIRTUAL();
	MIRROR_FRIEND();

public:
	Mesh();
	virtual ~Mesh();

	void setVertices(const Vertex* _vertices, u32 _vertexCount);
	const BaseArray<Vertex>& getVertices() const;

	void setIndices(const u32* _indices, u32 _indexCount);
	const BaseArray<u32>& getIndices() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	DataArray<Vertex> m_vertices;
	DataArray<u32> m_indices;
};

} // namespace yae
