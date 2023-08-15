#pragma once

#include <yae/types.h>

#include <yae/rendering/render_types.h>
#include <yae/resources/Resource.h>
#include <yae/resources/ResourcePtr.h>

namespace yae {

class Shader;

class YAE_API ShaderProgram : public Resource
{
	MIRROR_GETCLASS_VIRTUAL();
	MIRROR_FRIEND();

public:
	ShaderProgram();
	virtual~ ShaderProgram();

	void setShaderStages(Shader** _shaders, u32 _shaderCount);
	const DataArray<ResourcePtr<Shader>>& getShaderStages();

	void setPrimitiveMode(PrimitiveMode _primitiveMode);
	PrimitiveMode getPrimitiveMode() const;

	ShaderProgramHandle getShaderProgramHandle() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	DataArray<ResourcePtr<Shader>> m_shaderStages;
	DataArray<Shader*> m_loadedShaders;
	ShaderProgramHandle m_shaderProgramHandle = 0;
	PrimitiveMode m_primitiveMode = PrimitiveMode::TRIANGLES;
};

} // namespace yae
