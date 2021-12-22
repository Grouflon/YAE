#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/containers/array.h>
#include <yae/render_types.h>

#include <mirror/mirror.h>

namespace yae {

class YAELIB_API MeshResource : public Resource
{
	MIRROR_CLASS(MeshResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	MeshResource(const char* _path);
	virtual ~MeshResource();

	const MeshHandle& getMeshHandle() const { return m_meshHandle; }

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	MeshHandle m_meshHandle;
	DataArray<Vertex> m_vertices;
	DataArray<u32> m_indices;
};

template <>
struct YAELIB_API ResourceIDGetter<MeshResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};

} // namespace yae
