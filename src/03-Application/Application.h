#pragma once

#include <string>

#include <00-Type/IntTypes.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

class ResourceManager;
class VulkanRenderer;

class Application
{
public:
	void init(const char* _name, u32 _width, u32 _height);
	void run();
	void shutdown();

private:
	std::string m_name;

	ResourceManager* m_resourceManager = nullptr;
	VulkanRenderer* m_vulkanWrapper = nullptr;
	GLFWwindow* m_window = nullptr;
	ImGuiContext* m_imgui = nullptr;
};

}
