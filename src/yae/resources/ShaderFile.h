#pragma once

#include <yae/types.h>

#include <yae/rendering/render_types.h>
#include <yae/resources/Resource.h>

namespace yae {

class YAE_API ShaderFile : public Resource
{
	MIRROR_CLASS(ShaderFile)
	(
		MIRROR_PARENT(Resource)
	);

public:
	ShaderFile();
	virtual~ ShaderFile();

	void setPath(const char* _path);
	const char* getPath() const;

	void setShaderType(ShaderType _type);
	ShaderType getShaderType() const;

	const ShaderHandle& getShaderHandle();

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
	ShaderHandle m_shaderHandle;
	ShaderType m_shaderType;
};

} // namespace yae
