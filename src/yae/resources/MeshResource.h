#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/containers/Array.h>
#include <yae/rendering/render_types.h>

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

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	DataArray<Vertex> m_vertices;
	DataArray<u32> m_indices;
};

template <>
struct YAELIB_API ResourceIDGetter<MeshResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};

} // namespace yae
