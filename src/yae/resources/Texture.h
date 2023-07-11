#pragma once

#include <yae/types.h>

#include <yae/resources/Resource.h>
#include <yae/rendering/render_types.h>

namespace yae {

class YAE_API Texture : public Resource
{
	MIRROR_CLASS(Texture)
	(
		MIRROR_PARENT(Resource)

		MIRROR_MEMBER(m_parameters)();
	);

public:
	Texture();
	virtual ~Texture();

	void setPixelData(const void* _data, u32 _width, u32 _height, u32 _channelCount);
	const void* getPixelData() const;

	void setFilter(TextureFilter _filter);
	TextureFilter getFilter() const;

	u32 getWidth() const;
	u32 getHeight() const;
	u32 getChannelCount() const;

	const TextureHandle& getTextureHandle() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	const void* m_pixelData = nullptr;
	u32 m_width = 0;
	u32 m_height = 0;
	u32 m_channelCount = 0;
	TextureHandle m_textureHandle;
	
	TextureParameters m_parameters;
};

} // namespace yae
