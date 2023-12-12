#pragma once

#include <core/types.h>
#include <core/Date.h>
#include <core/containers/Array.h>

namespace yae {

class Serializer;
class Application;

class Module
{
public:
	String256 name;
	void* libraryHandle = nullptr;
	Date lastLibraryWriteTime = 0;
	void* userData = nullptr;
	DataArray<StringHash> dependencies;
	DataArray<StringHash> dependents;

	void (*beforeModuleReloadFunction)(Program* _program, Module* _module) = nullptr;
	void (*afterModuleReloadFunction)(Program* _program, Module* _module) = nullptr;
	void (*initModuleFunction)(Program* _program, Module* _module) = nullptr;
	void (*shutdownModuleFunction)(Program* _program, Module* _module) = nullptr;

	void (*startProgramFunction)(Program* _program, Module* _module) = nullptr;
	void (*stopProgramFunction)(Program* _program, Module* _module) = nullptr;
	void (*updateProgramFunction)(Program* _program, Module* _module) = nullptr;

	bool (*serializeSettingsFunction)(Serializer& _module) = nullptr;

	void (*getDependenciesFunction)(const char*** _outModuleNames, int* _outModuleCount) = nullptr;
};

} // namespace yae
