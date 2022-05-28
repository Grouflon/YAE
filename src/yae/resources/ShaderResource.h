#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/render_types.h>
#include <yae/containers/Array.h>

namespace yae {

class YAELIB_API ShaderResource : public Resource
{
	MIRROR_CLASS(ShaderResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	ShaderResource(const char* _path, ShaderType _type);
	virtual~ ShaderResource();

	const ShaderHandle& getShaderHandle() const { return m_shaderHandle; }
	ShaderType getShaderType() const { return m_shaderType; }

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	ShaderHandle m_shaderHandle;
	ShaderType m_shaderType;
	String m_path;
};

template <>
struct YAELIB_API ResourceIDGetter<ShaderResource>
{
	static ResourceID GetId(const char* _path, ShaderType _type);
};

} // namespace yae
