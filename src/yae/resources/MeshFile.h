#pragma once

#include <yae/types.h>
#include <yae/resources/Mesh.h>

namespace yae {

class YAE_API MeshFile : public Mesh
{
	MIRROR_CLASS(MeshFile)
	(
		MIRROR_PARENT(Mesh);

		MIRROR_MEMBER(m_path)();
		MIRROR_MEMBER(m_offset)();
	);

public:
	MeshFile();
	virtual ~MeshFile();

	void setPath(const char* _path);
	const char* getPath() const;

	void setOffset(const Transform& _offset);
	const Transform& getOffset() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
	Transform m_offset = Transform::IDENTITY;
};

} // namespace yae
