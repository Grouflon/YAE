#include "Application.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

#include <00-Macro/Assert.h>
#include <01-Serialization/JsonSerializer.h>
#include <02-Log/Log.h>
#include <03-Resource/ResourceManager.h>
#include <03-Renderer/VulkanRenderer.h>
#include <03-Resource/FileResource.h>


namespace yae {

	void Application::init(const char* _name, u32 _width, u32 _height)
	{
		m_name = _name;

		// Init Window
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_window = glfwCreateWindow(_width, _height, m_name.c_str(), nullptr, nullptr);
		YAE_ASSERT(m_window);
		YAE_VERBOSE_CAT("glfw", "Created glfw window");

		m_resourceManager = new ResourceManager();

		// Init Vulkan
		m_vulkanWrapper = new VulkanRenderer();

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		m_vulkanWrapper->init(m_window, enableValidationLayers);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		m_imgui = ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		{
			bool ret = ImGui_ImplGlfw_InitForVulkan(m_window, true);
			YAE_ASSERT(ret);
		}
		m_vulkanWrapper->initImGui();

		//
		{
			FileResource* configFile = FindOrCreateResource<FileResource>("./config.json");
			configFile->useLoad();

			//for (int i = 0; i < 10000; ++i)
			ConfigData config;
			JsonSerializer serializer;
			serializer.beginRead(configFile->getContent(), configFile->getContentSize());
			serializer.serialize(&config, ConfigData::GetClass());
			serializer.endRead();

			const void* buffer;
			size_t bufferSize;
			serializer.beginWrite();
			serializer.serialize(&config, ConfigData::GetClass());
			serializer.endWrite(&buffer, &bufferSize);

			printf("%s", (const char*)buffer);

			configFile->releaseUnuse();
		}
	}

	void Application::run()
	{
		static bool s_showDemoWindow = true;
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();

			ImGui_ImplGlfw_NewFrame();
			m_vulkanWrapper->beginFrame();
			ImGui::NewFrame();

			ImGui::ShowDemoWindow(&s_showDemoWindow);

			m_vulkanWrapper->drawMesh();

			// Rendering
			ImGui::Render();
			ImDrawData* imguiDrawData = ImGui::GetDrawData();
			const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
			if (!isMinimized)
			{
				m_vulkanWrapper->drawImGui(imguiDrawData);
			}

			m_vulkanWrapper->endFrame();
		}
		m_vulkanWrapper->waitIdle();
	}

	void Application::shutdown()
	{
		m_vulkanWrapper->shutdownImGui();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext(m_imgui);
		m_imgui = nullptr;

		m_vulkanWrapper->shutdown();
		delete m_vulkanWrapper;
		m_vulkanWrapper = nullptr;

		delete m_resourceManager;
		m_resourceManager = nullptr;

		glfwDestroyWindow(m_window);
		m_window = nullptr;
		YAE_VERBOSE_CAT("glfw", "Destroyed glfw window");
	}

}
