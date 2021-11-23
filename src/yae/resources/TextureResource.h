#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/vulkan/VulkanRenderer.h>

#include <mirror/mirror.h>

namespace yae {

class YAELIB_API TextureResource : public Resource
{
	MIRROR_CLASS(TextureResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	TextureResource(const char* _path);
	virtual ~TextureResource();

	const TextureHandle& getHandle() const;

// private:
	virtual Error _doLoad(String& _outErrorDescription) override;
	virtual void _doUnload() override;

	int m_width = 0;
	int m_height = 0;
	int m_channels = 0;
	TextureHandle m_textureHandle;
};

template <>
struct ResourceIDGetter<TextureResource>
{
	static ResourceID GetId(const char* _path) { return ResourceID(_path); }
};

} // namespace yae
