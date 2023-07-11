#pragma once

#include <yae/types.h>

#include <yae/resources/Shader.h>

namespace yae {

class YAE_API ShaderFile : public Shader
{
	MIRROR_CLASS(ShaderFile)
	(
		MIRROR_PARENT(Shader);

		MIRROR_MEMBER(m_path)();
	);

public:
	ShaderFile();
	virtual~ ShaderFile();

	void setPath(const char* _path);
	const char* getPath() const;

// private:
	virtual void _doLoad() override;
	virtual void _doUnload() override;

	String m_path;
};

} // namespace yae
