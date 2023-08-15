#pragma once

#include <yae/types.h>
#include <yae/resources/Resource.h>
#include <yae/rendering/render_types.h>

#include <mirror/mirror.h>

#include <stb_truetype.h>

namespace yae {

class File;

class YAE_API FontFile : public Resource
{
	MIRROR_GETCLASS_VIRTUAL();
	MIRROR_FRIEND();

public:
	FontFile();
	virtual ~FontFile();

	void setPath(const char* _path);
	const char* getPath() const;

	void setSize(u32 _size);
	u32 getSize() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
	u32 m_fontSize = 0;
	u32 m_atlasWidth;
	u32 m_atlasHeight;

	stbtt_fontinfo m_font;
	stbtt_packedchar m_packedChar[256];
	TextureHandle m_fontTexture;
};

} // namespace yae
