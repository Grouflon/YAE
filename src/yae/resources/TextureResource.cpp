#include "TextureResource.h"

#include "yae/profiling.h"
#include "yae/vulkan/VulkanRenderer.h"
#include "yae/log.h"

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
	: Resource(ResourceID(Path(_path, true, &scratchAllocator()).c_str()))
	, m_path(_path, true, &defaultAllocator())
{
	setName(m_path.c_str());
}


TextureResource::~TextureResource()
{

}


Resource::Error TextureResource::onLoaded(String& _outErrorDescription)
{
	YAE_CAPTURE_FUNCTION();

	// Create Texture Image
	YAE_VERBOSEF_CAT("resource", "Loading texture \"%s\"...", m_path.c_str());
	stbi_uc* pixels = stbi_load(m_path.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
	if (pixels == nullptr)
	{
		_outErrorDescription = "Failed to open file";
		return ERROR_LOAD;
	}

	YAE_ASSERT(pixels);
	bool result = renderer().createTexture(pixels, m_width, m_height, m_channels, m_textureHandle);
	stbi_image_free(pixels);

	if (!result)
	{
		_outErrorDescription = "Failed to create texture";
		return ERROR_LOAD;	
	}

	YAE_VERBOSEF_CAT("resource", "Succesfully loaded texture \"%s\".", m_path.c_str());
	return ERROR_NONE;
}


void TextureResource::onUnloaded()
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
