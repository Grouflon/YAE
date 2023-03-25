#pragma once

#include <yae/types.h>

#include <yae/rendering/render_types.h>
#include <yae/resources/Resource.h>

namespace yae {

class YAE_API Shader : public Resource
{
	MIRROR_CLASS(Shader)
	(
		MIRROR_PARENT(Resource)
	);

public:
	Shader();
	virtual~ Shader();

	void setShaderData(const void* _data, u32 _dataSize); // Makes a copy, you can free the data afterwards
	void getShaderData(const void*& _data, u32& _dataSize) const;

	void setShaderType(ShaderType _type);
	ShaderType getShaderType() const;

	const ShaderHandle& getShaderHandle();

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	void* m_data = nullptr;
	u32 m_dataSize = 0;

	ShaderHandle m_shaderHandle;
	ShaderType m_shaderType;
};

} // namespace yae
