#pragma once

#include <yae/types.h>
#include <yae/Date.h>
#include <yae/inline_string.h>

namespace yae {

class Module
{
public:
	String256 name;
	void* libraryHandle = nullptr;
	Date lastLibraryWriteTime = 0;
	void* userData = nullptr;

	void (*onModuleLoadedFunction)(Program* _program, Module* _module) = nullptr;
	void (*onModuleUnloadedFunction)(Program* _program, Module* _module) = nullptr;
	void (*initModuleFunction)(Program* _program, Module* _module) = nullptr;
	void (*shutdownModuleFunction)(Program* _program, Module* _module) = nullptr;

	void (*initApplicationFunction)(Application* _app) = nullptr;
	void (*updateApplicationFunction)(Application* _app, float _dt) = nullptr;
	void (*shutdownApplicationFunction)(Application* _app) = nullptr;
};

} // namespace yae
