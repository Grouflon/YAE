#pragma once

#include <yae/types.h>
#include <yae/Date.h>

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

	void (*initApplicationFunction)(Application* _app, const char** _args, int _argCount) = nullptr;
	void (*updateApplicationFunction)(Application* _app, float _dt) = nullptr;
	void (*shutdownApplicationFunction)(Application* _app) = nullptr;
	void (*onApplicationModuleReloadedFunction)(Application* _app, Module* _module) = nullptr;
	bool (*onSerializeApplicationSettingsFunction)(Application* _app, Serializer* _serializer) = nullptr;
};

} // namespace yae
