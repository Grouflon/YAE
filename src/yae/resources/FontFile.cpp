#include "FontFile.h"

#include <yae/filesystem.h>
#include <yae/rendering/Renderer.h>
#include <yae/resource.h>
#include <yae/resources/File.h>
#include <yae/string.h>

// this is helpful: https://github.com/0xc0dec/demos

namespace yae {

MIRROR_CLASS_DEFINITION(FontFile);

FontFile::FontFile()
{
}

FontFile::~FontFile()
{

}

void FontFile::setPath(const char* _path)
{
	YAE_ASSERT(!isLoaded());
	m_path = filesystem::normalizePath(_path);
}

const char* FontFile::getPath() const
{
	return m_path.c_str();
}

void FontFile::setSize(u32 _size)
{
	YAE_ASSERT(!isLoaded());
	m_fontSize = _size;
}

u32 FontFile::getSize() const
{
	return m_fontSize;
}

void FontFile::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	FileReader reader(m_path.c_str(), &scratchAllocator());
	if (!reader.load())
	{
		_log(RESOURCELOGTYPE_ERROR, string::format("Could not load file \"%s\".", m_path.c_str()).c_str());
		return;
	}
	stbtt_InitFont(&m_font, (const u8*)reader.getContent(), 0);

	m_atlasWidth = 512;
	m_atlasHeight = 512;
	u8* fontBitmap = (u8*)scratchAllocator().allocate(m_atlasWidth*m_atlasHeight);
	stbtt_pack_context pc;
	YAE_VERIFY(stbtt_PackBegin(&pc, fontBitmap, m_atlasWidth, m_atlasHeight, 0, 1, nullptr) == 1);
	YAE_VERIFY(stbtt_PackFontRange(&pc, (const u8*)reader.getContent(), 0, float(m_fontSize), 0, 256, m_packedChar) == 1);
	stbtt_PackEnd(&pc);

	YAE_VERIFY(renderer().createTexture(fontBitmap, m_atlasWidth, m_atlasHeight, 1, m_fontTexture) == true);

	scratchAllocator().deallocate(fontBitmap);
}

void FontFile::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyTexture(m_fontTexture);
	m_fontTexture = 0;
}

} // namespace yae

