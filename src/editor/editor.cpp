#include "editor.h"

#include <yae/memory.h>
#include <yae/Application.h>

#include <imgui/imgui.h>

using namespace yae;

struct EditorInstance
{
	bool showMemoryProfiler = false;
	bool showFrameRate = false;
	bool showDemoWindow = false;
};

void onModuleLoaded(yae::Program* _program, yae::Module* _module)
{

}

void onModuleUnloaded(yae::Program* _program, yae::Module* _module)
{

}

void initModule(yae::Program* _program, yae::Module* _module)
{

}

void shutdownModule(yae::Program* _program, yae::Module* _module)
{

}

void initApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = toolAllocator().create<EditorInstance>();
	_application->setUserData("editor", editorInstance);
}

void shutdownApplication(yae::Application* _application)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");
	_application->setUserData("editor", nullptr);
	toolAllocator().destroy(editorInstance);
}

void updateApplication(yae::Application* _application, float _dt)
{
	EditorInstance* editorInstance = (EditorInstance*)_application->getUserData("editor");

	if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
        	if (ImGui::MenuItem("Exit"))
        	{
        		_application->requestExit();
        	}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Profiling"))
        {
        	ImGui::MenuItem("Memory", NULL, &editorInstance->showMemoryProfiler);
        	ImGui::MenuItem("Frame rate", NULL, &editorInstance->showFrameRate);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Misc"))
        {
        	ImGui::MenuItem("ImGui Demo Window", NULL, &editorInstance->showDemoWindow);
            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();
    }

    if (editorInstance->showMemoryProfiler)
    {
    	auto showAllocatorInfo = [](const char* _name, const Allocator& _allocator)
    	{
    		char allocatedSizeBuffer[32];
	    	char allocableSizeBuffer[32];
	    	auto formatSize = [](size_t _size, char _buf[32])
	    	{
	    		if (_size == Allocator::SIZE_NOT_TRACKED)
	    		{
	    			snprintf(_buf, 31, "???");
	    		}
	    		else
	    		{
	    			const char* units[] =
	    			{
	    				"b",
	    				"Kb",
	    				"Mb",
	    				"Gb",
	    				"Tb"
	    			};

	    			u8 unit = 0;
	    			u32 mod = 1024 * 10;
	    			while (_size > mod)
	    			{
	    				_size = _size / mod;
	    				++unit;
	    			}

	    			snprintf(_buf, 31, "%zu %s", _size, units[unit]);
	    		}
	    	};

    		formatSize(_allocator.getAllocatedSize(), allocatedSizeBuffer);
    		formatSize(_allocator.getAllocableSize(), allocableSizeBuffer);

    		ImGui::Text("%s: %s / %s, %zu allocations",
	    		_name,
	    		allocatedSizeBuffer,
	    		allocableSizeBuffer,
	    		_allocator.getAllocationCount()
	    	);
    	};

    	ImGui::Begin("Memory Profiler", &editorInstance->showMemoryProfiler, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize);
    	showAllocatorInfo("Default", defaultAllocator());
    	showAllocatorInfo("Scratch", scratchAllocator());
    	showAllocatorInfo("Tool", toolAllocator());
    	showAllocatorInfo("Malloc", mallocAllocator());
    	ImGui::End();
    }

    if (editorInstance->showFrameRate)
    {
    	ImGui::Begin("Frame Rate", &editorInstance->showFrameRate, ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoInputs);
    	ImGui::Text("dt: %.2f ms", _dt*1000.f);
    	ImGui::Text("%.2f fps", _dt != 0.f ? (1.f/_dt) : 0.f);
    	ImGui::End();
    }

    if (editorInstance->showDemoWindow)
    {
		ImGui::ShowDemoWindow(&editorInstance->showDemoWindow);
    }
}
