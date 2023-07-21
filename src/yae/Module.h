#pragma once

#include <yae/types.h>
#include <yae/Date.h>
#include <yae/inline_string.h>

namespace yae {

class Serializer;

class Module
{
public:
	String256 name;
	void* libraryHandle = nullptr;
	Date lastLibraryWriteTime = 0;
	void* userData = nullptr;

	void (*onModuleLoadedFunction)(Program* _program, Module* _module) = nullptr;
	void (*onModuleUnloadedFunction)(Program* _program, Module* _module) = nullptr;
	void (*onModuleReloadedFunction)(Program* _program, Module* _module) = nullptr;
	void (*initModuleFunction)(Program* _program, Module* _module) = nullptr;
	void (*shutdownModuleFunction)(Program* _program, Module* _module) = nullptr;

	void (*beforeInitApplicationFunction)(Application* _app) = nullptr;
	void (*afterInitApplicationFunction)(Application* _app) = nullptr;
	void (*onApplicationModuleReloadedFunction)(Application* _app, Module* _module) = nullptr;
	void (*updateApplicationFunction)(Application* _app, float _dt) = nullptr;
	bool (*onSerializeApplicationSettingsFunction)(Application* _app, Serializer* _serializer) = nullptr;
	void (*beforeShutdownApplicationFunction)(Application* _app) = nullptr;
	void (*afterShutdownApplicationFunction)(Application* _app) = nullptr;
};

} // namespace yae
