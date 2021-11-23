#pragma once

#include <yae/resource.h>
#include <yae/filesystem.h>
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

	virtual Error onLoaded(String& _outErrorDescription) override;
	virtual void onUnloaded() override;

	const TextureHandle& getHandle() const;

// private:
	Path m_path;
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
