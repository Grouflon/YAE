#include "yae_module.h"

#include <core/memory.h>
#include <core/program.h>
#include <core/Module.h>
#include <core/containers/Array.h>

#include <yae/ApplicationRegistry.h>
#include <yae/Application.h>

#include <imgui/imgui.h>

void* ImGuiMemAlloc(size_t _size, void* _userData)
{
    return yae::toolAllocator().allocate(_size);
}

void ImGuiMemFree(void* _ptr, void* _userData)
{
    return yae::toolAllocator().deallocate(_ptr);
}

// Module Interface

void initModule(yae::Program* _program, yae::Module* _module)
{
	yae::ApplicationRegistry* applicationRegistry = yae::defaultAllocator().create<yae::ApplicationRegistry>(); 
	_module->userData = applicationRegistry;

	// Init ImGui
	{
    	ImGui::SetAllocatorFunctions(&ImGuiMemAlloc, &ImGuiMemFree, nullptr);
	}
}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{
	// ImGui shutdown
	{
		ImGui::SetAllocatorFunctions(nullptr, nullptr, nullptr);
	}

	yae::defaultAllocator().destroy((yae::ApplicationRegistry*)_module->userData);
	_module->userData = nullptr;
}

void onModuleReloaded(yae::Program* _program, yae::Module* _module)
{

}

void startProgram(yae::Program* _program, yae::Module* _module)
{
}

void stopProgram(yae::Program* _program, yae::Module* _module)
{
}

void updateProgram(yae::Program* _program, yae::Module* _module)
{
	yae::ApplicationRegistry::Update();
}
