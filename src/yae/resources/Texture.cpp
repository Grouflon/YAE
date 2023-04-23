#include "Texture.h"

#include <yae/rendering/Renderer.h>

#include <stb/stb_image.h>

namespace yae {

MIRROR_CLASS_DEFINITION(Texture);

Texture::Texture()
{
}


Texture::~Texture()
{
}

void Texture::setPixelData(const void* _data, u32 _width, u32 _height, u32 _channelCount)
{
	YAE_ASSERT(m_textureHandle == 0);
	m_pixelData = _data;
	m_width = _width;
	m_height = _height;
	m_channelCount = _channelCount;
}

const void* Texture::getPixelData() const
{
	return m_pixelData;
}

void Texture::setFilter(TextureFilter _filter)
{
	YAE_ASSERT(m_textureHandle == 0);

	m_parameters.filter = _filter;
}

TextureFilter Texture::getFilter() const
{
	return m_parameters.filter;
}

u32 Texture::getWidth() const
{
	return m_width;
}

u32 Texture::getHeight() const
{
	return m_height;
}

u32 Texture::getChannelCount() const
{
	return m_channelCount;
}

const TextureHandle& Texture::getTextureHandle() const
{
	return m_textureHandle;
}


void Texture::_doLoad()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_pixelData != nullptr);
	bool result = renderer().createTexture(m_pixelData, m_width, m_height, m_channelCount, m_textureHandle);
	if (!result)
	{
		_log(RESOURCELOGTYPE_ERROR, "Failed to create texture.");
		return;
	}

	renderer().applyTextureParameters(m_textureHandle, m_parameters);

	YAE_VERBOSEF_CAT("resource", "Succesfully loaded texture \"%s\".", getName());
	return;
}


void Texture::_doUnload()
{
	YAE_CAPTURE_FUNCTION();

	renderer().destroyTexture(m_textureHandle);
	m_textureHandle = 0;
}

} // namespace yae
