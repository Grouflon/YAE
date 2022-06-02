#include "FontResource.h"

#include <yae/filesystem.h>
#include <yae/string.h>
#include <yae/rendering/Renderer.h>
#include <yae/resources/FileResource.h>

#include <GLFW/glfw3.h>
// this is helpful: https://github.com/0xc0dec/demos

namespace yae {

MIRROR_CLASS_DEFINITION(FontResource);

FontResource::FontResource(const char* _path, u32 _size)
	: Resource(generateStringId(_path, _size).c_str())
	, m_path(_path)
	, m_fontSize(_size)
{
}


FontResource::~FontResource()
{

}


void FontResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	m_fontFile = findOrCreateResource<FileResource>(m_path.c_str());
	m_fontFile->useLoad();

	stbtt_InitFont(&m_font, (const u8*)m_fontFile->getContent(), 0);

	m_atlasWidth = 512;
	m_atlasHeight = 512;
	u8* fontBitmap = (u8*)scratchAllocator().allocate(m_atlasWidth*m_atlasHeight);
	stbtt_pack_context pc;
	YAE_VERIFY(stbtt_PackBegin(&pc, fontBitmap, m_atlasWidth, m_atlasHeight, 0, 1, nullptr) == 1);
	YAE_VERIFY(stbtt_PackFontRange(&pc, (const u8*)m_fontFile->getContent(), 0, float(m_fontSize), 0, 256, m_packedChar) == 1);
	stbtt_PackEnd(&pc);

	YAE_VERIFY(renderer().createTexture(fontBitmap, m_atlasWidth, m_atlasHeight, GL_ALPHA, m_fontTexture) == true);

	scratchAllocator().deallocate(fontBitmap);
}


void FontResource::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyTexture(m_fontTexture);
	m_fontTexture = 0;

	m_fontFile->releaseUnuse();
	m_fontFile = nullptr;
}

String FontResource::generateStringId(const char* _path, u32 _size)
{
	return string::format("%s_%d", filesystem::normalizePath(_path).c_str(), _size);
}


} // namespace yae

