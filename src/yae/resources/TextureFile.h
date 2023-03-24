#pragma once

#include <yae/types.h>

#include <yae/resources/Texture.h>
#include <yae/rendering/render_types.h>

namespace yae {

class YAE_API TextureFile : public Texture
{
	MIRROR_CLASS(TextureFile)
	(
		MIRROR_PARENT(Texture)
	);

public:
	TextureFile();
	virtual ~TextureFile();

	void setPath(const char* _path);
	const char* getPath() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
};

} // namespace yae
