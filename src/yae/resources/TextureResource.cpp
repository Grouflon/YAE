#include "TextureResource.h"

#include <yae/filesystem.h>
#include <yae/rendering/Renderer.h>

#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

namespace yae {

MIRROR_CLASS_DEFINITION(TextureResource);

TextureResource::TextureResource(const char* _path)
	: Resource(filesystem::normalizePath(_path).c_str())
{
}


TextureResource::~TextureResource()
{

}

void TextureResource::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	// Create Texture Image
	stbi_uc* pixels;
	{
		YAE_CAPTURE_SCOPE("open_file");

		YAE_VERBOSEF_CAT("resource", "Loading texture \"%s\"...", getName());
		pixels = stbi_load(getName(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			_log(RESOURCELOGTYPE_ERROR, "Could not open file.");
			return;
		}	
	}
	

	YAE_ASSERT(pixels);
	bool result = renderer().createTexture(pixels, m_width, m_height, GL_RGBA, m_textureHandle);
	stbi_image_free(pixels);

	if (!result)
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create texture.");
		return;
	}

	YAE_VERBOSEF_CAT("resource", "Succesfully loaded texture \"%s\".", getName());
	return;
}


void TextureResource::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyTexture(m_textureHandle);
	m_width = 0;
	m_height = 0;
	m_channels = 0;
}


const TextureHandle& TextureResource::getTextureHandle() const
{
	YAE_ASSERT(isLoaded());
	return m_textureHandle;
}


} // namespace yae
