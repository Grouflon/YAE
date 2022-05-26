#pragma once

#include <yae/types.h>
#include <yae/render_types.h>

namespace yae {

struct YAELIB_API ShaderCompilationParams
{
	ShaderType type = ShaderType::UNDEFINED;
	const char* shaderName = "";
	const char* entryPoint = "main";
	const char** defines = nullptr;
	u16 defineCount = 0;
};

typedef void* ShaderCompilationResult;

class YAELIB_API ShaderCompiler
{
public:

	void init();
	void shutdown();

	ShaderCompilationResult compile(const char* _source, size_t _sourceSize, const ShaderCompilationParams& _params = ShaderCompilationParams());

	bool getResultData(ShaderCompilationResult _result, const void*& _outData, size_t& _outDataLength);
	void releaseResult(ShaderCompilationResult _result);
private:

	void* m_compiler = nullptr;
};

} // namespace yae
