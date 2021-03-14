#pragma once

#include <string>

#include <mirror.h>

#include <00-Type/IntTypes.h>

struct GLFWwindow;
struct ImGuiContext;

namespace yae {

struct ConfigData
{
	u16 windowX;
	u16 windowY;
	u16 windowWidth;
	u16 windowHeight;
	bool fullscreen;
	float someFloat;
	double someDouble;
	char someChar;

	MIRROR_CLASS_NOVIRTUAL(yae::ConfigData)
	(
		MIRROR_MEMBER(windowX)()
		MIRROR_MEMBER(windowY)()
		MIRROR_MEMBER(windowWidth)()
		MIRROR_MEMBER(windowHeight)()
		MIRROR_MEMBER(fullscreen)()
		MIRROR_MEMBER(someFloat)()
		MIRROR_MEMBER(someDouble)()
		MIRROR_MEMBER(someChar)()
	);
};

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
