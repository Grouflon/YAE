#include "TextureFile.h"

#include <yae/filesystem.h>
#include <yae/rendering/Renderer.h>

#include <stb/stb_image.h>

namespace yae {

MIRROR_CLASS_DEFINITION(TextureFile);

TextureFile::TextureFile()
{
}


TextureFile::~TextureFile()
{

}

void TextureFile::setPath(const char* _path)
{
	YAE_ASSERT(!isLoaded());
	m_path = filesystem::normalizePath(_path);
}

const char* TextureFile::getPath() const
{
	return m_path.c_str();
}

void TextureFile::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	// Create Texture Image
	i32 width, height, channelCount;
	stbi_uc* pixels = nullptr;
	{
		YAE_CAPTURE_SCOPE("open_file");

		YAE_VERBOSEF_CAT("resource", "Loading texture \"%s\"...", m_path.c_str());
		pixels = stbi_load(m_path.c_str(), &width, &height, &channelCount, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			_log(RESOURCELOGTYPE_ERROR, "Could not open file.");
			return;
		}	
	}
	setPixelData(pixels, width, height, channelCount);
	Texture::_doLoad();
}


void TextureFile::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	Texture::_doUnload();

	stbi_image_free((void*)m_pixelData);
	setPixelData(nullptr, 0, 0, 0);
}

} // namespace yae