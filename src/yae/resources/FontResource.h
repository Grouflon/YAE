#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/render_types.h>

#include <mirror/mirror.h>

#include <stb_truetype.h>

namespace yae {

class FileResource;

class YAELIB_API FontResource : public Resource
{
	MIRROR_CLASS(FontResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	FontResource(const char* _path, u32 _size);
	virtual ~FontResource();

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
	u32 m_fontSize = 0;

	stbtt_fontinfo m_font;
	stbtt_packedchar m_packedChar[256];
	FileResource* m_fontFile;
	TextureHandle m_fontTexture;

	static String generateStringId(const char* _path, u32 _size);
};

template <>
struct YAELIB_API ResourceIDGetter<FontResource>
{
	static ResourceID GetId(const char* _path, u32 _size) {
		return ResourceID(FontResource::generateStringId(_path, _size).c_str());
	}
};

} // namespace yae
