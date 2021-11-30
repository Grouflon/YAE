#pragma once

#include <yae/types.h>
#include <yae/resource.h>
#include <yae/render_types.h>

namespace yae {

enum ShaderType
{
	SHADERTYPE_VERTEX,
	SHADERTYPE_FRAGMENT,
};

class YAELIB_API ShaderResource : public Resource
{
	MIRROR_CLASS(ShaderResource)
	(
		MIRROR_PARENT(Resource)
	);

public:
	ShaderResource(const char* _path, ShaderType _type, const char* _entryPoint = "main");
	virtual~ ShaderResource();

	const ShaderHandle& getShaderHandle() const { return m_shaderHandle; }
	ShaderType getShaderType() const { return m_shaderType; }
	const char* getEntryPoint() const { return m_entryPoint.c_str(); }

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	ShaderHandle m_shaderHandle;
	ShaderType m_shaderType;
	String m_path;
	String m_entryPoint;
};

template <>
struct ResourceIDGetter<ShaderResource>
{
	static ResourceID GetId(const char* _path, ShaderType _type, const char* _entryPoint = "main");
};

} // namespace yae
