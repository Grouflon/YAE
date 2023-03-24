#pragma once

#include <yae/types.h>
#include <yae/resources/Mesh.h>

namespace yae {

class YAE_API MeshFile : public Mesh
{
	MIRROR_CLASS(MeshFile)
	(
		MIRROR_PARENT(Mesh)
	);

public:
	MeshFile();
	virtual ~MeshFile();

	void setPath(const char* _path);
	const char* getPath() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
};

} // namespace yae
