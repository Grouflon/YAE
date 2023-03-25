#pragma once

#include <yae/types.h>

#include <yae/rendering/render_types.h>
#include <yae/resources/Resource.h>

namespace yae {

class Shader;

class YAE_API ShaderProgram : public Resource
{
	MIRROR_CLASS(ShaderProgram)
	(
		MIRROR_PARENT(Resource)
	);

public:
	ShaderProgram();
	virtual~ ShaderProgram();

	void setShaderStages(Shader** _shaders, u32 _shaderCount);
	const DataArray<Shader*>& getShaderStages();

	ShaderProgramHandle getShaderProgramHandle() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	DataArray<Shader*> m_shaderStages;
	ShaderProgramHandle m_shaderProgramHandle = 0;
};

} // namespace yae
