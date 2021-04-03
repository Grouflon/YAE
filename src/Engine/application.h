#pragma once

#include <types.h>
#include <yae_string.h>
#include <filesystem.h>

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

	VulkanRenderer* m_vulkanWrapper = nullptr;
	GLFWwindow* m_window = nullptr;
	ImGuiContext* m_imgui = nullptr;
};

} // namespace yae
