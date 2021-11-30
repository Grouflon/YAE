#include "TextureResource.h"

#include <yae/filesystem.h>
#include <yae/vulkan/VulkanRenderer.h>

void* yae_stbi_malloc(size_t _sz)
{
	return yae::scratchAllocator().allocate(_sz);
}

void* yae_stbi_realloc(void* _p, size_t _newsz)
{
	if (_p == nullptr)
		return yae_stbi_malloc(_newsz);

	yae::Allocator& a = yae::scratchAllocator();
	void* p = a.allocate(_newsz);
	memcpy(p, _p, a.getAllocationSize(_p));
	a.deallocate(_p);
	return p;
}

void yae_stbi_free(void* _p)
{
	yae::scratchAllocator().deallocate(_p);
}

#define STBI_ASSERT(x)            YAE_ASSERT(x)
#define STBI_MALLOC(sz)           yae_stbi_malloc(sz);
#define STBI_REALLOC(p,newsz)     yae_stbi_realloc(p, newsz);
#define STBI_FREE(p)              yae_stbi_free(p);
#define STB_IMAGE_IMPLEMENTATION
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
	bool result = renderer().createTexture(pixels, m_width, m_height, m_channels, m_textureHandle);
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


const TextureHandle& TextureResource::getHandle() const
{
	YAE_ASSERT(isLoaded());
	return m_textureHandle;
}


} // namespace yae
