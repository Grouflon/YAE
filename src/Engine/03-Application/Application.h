#pragma once

#include <export.h>
#include <00-Type/IntTypes.h>
#include <00-String/String.h>
#include <01-FileSystem/Path.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class ResourceManager;
class VulkanRenderer;

class YAELIB_API Application
{
public:
	void init(const char* _name, u32 _width, u32 _height, char** _args, int _arg_count);
	void run();
	void shutdown();

	const Path& getExePath();
	const Path& getBaseDirectoryPath();

private:

	String m_name;

	Path m_exePath;
	Path m_baseDirectoryPath;

	ResourceManager* m_resourceManager = nullptr;
	VulkanRenderer* m_vulkanWrapper = nullptr;
	GLFWwindow* m_window = nullptr;
	ImGuiContext* m_imgui = nullptr;
};

}
