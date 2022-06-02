#include "ImGuiSystem.h"

#include <yae/rendering/Renderer.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>

#if YAE_IMPLEMENTS_RENDERER_OPENGL
#include <imgui/backends/imgui_impl_opengl3.h>
#include <yae/rendering/renderers/opengl/OpenGLRenderer.h>
#endif
#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/rendering/renderers/vulkan/vulkan.h>
#include <yae/rendering/renderers/vulkan/VulkanRenderer.h>
#include <yae/rendering/renderers/vulkan/VulkanSwapChain.h>
#include <yae/rendering/renderers/vulkan/imgui_impl_vulkan.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#endif


namespace yae {

void ImGuiSystem::init(GLFWwindow* _window, Renderer* _renderer)
{
	YAE_CAPTURE_FUNCTION();
	YAE_VERBOSE_CAT("imgui", "Initializing ImGui...");

	m_renderer = _renderer;

	IMGUI_CHECKVERSION();
	m_imguiContext = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

#if YAE_IMPLEMENTS_RENDERER_OPENGL
	if (m_renderer->getType() == RendererType::OpenGL)
	{
		 OpenGLRenderer* openGLRenderer = static_cast<OpenGLRenderer*>(m_renderer);
		{
			bool ret = ImGui_ImplGlfw_InitForOpenGL(_window, true);
			YAE_ASSERT(ret);
		}

		{
			bool ret = ImGui_ImplOpenGL3_Init(openGLRenderer->getShaderVersion());
			YAE_ASSERT(ret);
		}
	}
#endif
#if YAE_IMPLEMENTS_RENDERER_VULKAN
	if (m_renderer->getType() == RendererType::Vulkan)
	{
		VulkanRenderer* vulkanRenderer = static_cast<VulkanRenderer*>(m_renderer);

		{
			bool ret = ImGui_ImplGlfw_InitForVulkan(_window, true);
			YAE_ASSERT(ret);	
		}
		

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = vulkanRenderer->m_instance;
		initInfo.PhysicalDevice = vulkanRenderer->m_physicalDevice;
		initInfo.Device = vulkanRenderer->m_device;
		initInfo.QueueFamily = vulkanRenderer->m_queueIndices.graphicsFamily;
		initInfo.Queue = vulkanRenderer->m_graphicsQueue;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = vulkanRenderer->m_descriptorPool;
		initInfo.Subpass = 0;
		initInfo.MinImageCount = VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;          // >= 2
		initInfo.ImageCount = u32(vulkanRenderer->m_swapChain->m_images.size());		// >= MinImageCount
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;           // >= VK_SAMPLE_COUNT_1_BIT
		initInfo.Allocator = nullptr;
		initInfo.VmaAllocator = vulkanRenderer->m_allocator;
		initInfo.CheckVkResultFn = [](VkResult _err) { VK_VERIFY(_err); };

		{
			bool ret = ImGui_ImplVulkan_Init(&initInfo, vulkanRenderer->m_swapChain->m_renderPass);
			YAE_ASSERT(ret);	
		}

		// Upload Fonts
		{
			// Use any command queue
			VkCommandBuffer commandBuffer = vulkan::beginSingleTimeCommands(vulkanRenderer->m_device, vulkanRenderer->m_commandPool);
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
			vulkan::endSingleTimeCommands(vulkanRenderer->m_device, vulkanRenderer->m_commandPool, vulkanRenderer->m_graphicsQueue, commandBuffer);
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}
#endif

	YAE_VERBOSE_CAT("imgui", "Initialized ImGui");
}


void ImGuiSystem::shutdown()
{
	YAE_CAPTURE_FUNCTION();

#if YAE_IMPLEMENTS_RENDERER_OPENGL
	if (m_renderer->getType() == RendererType::OpenGL)
	{
		ImGui_ImplOpenGL3_Shutdown();
	}
#endif
#if YAE_IMPLEMENTS_RENDERER_VULKAN
	if (m_renderer->getType() == RendererType::Vulkan)
	{
		ImGui_ImplVulkan_Shutdown();
	}
#endif

	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imguiContext);
	m_imguiContext = nullptr;

	YAE_VERBOSE_CAT("imgui", "Shutdown ImGui");
}

void ImGuiSystem::newFrame()
{
#if YAE_IMPLEMENTS_RENDERER_OPENGL
	if (m_renderer->getType() == RendererType::OpenGL)
	{
		ImGui_ImplOpenGL3_NewFrame();
	}
#endif
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiSystem::render(FrameHandle _frameHandle)
{
	YAE_CAPTURE_FUNCTION();

	ImGui::Render();

	ImDrawData* imguiDrawData = ImGui::GetDrawData();
	const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
	if (!isMinimized)
	{
#if YAE_IMPLEMENTS_RENDERER_OPENGL
		if (m_renderer->getType() == RendererType::OpenGL)
		{
			ImGui_ImplOpenGL3_RenderDrawData(imguiDrawData);
		}
#endif
#if YAE_IMPLEMENTS_RENDERER_VULKAN
		if (m_renderer->getType() == RendererType::Vulkan)
		{
			VkCommandBuffer commandBuffer = (VkCommandBuffer)_frameHandle;
			ImGui_ImplVulkan_RenderDrawData(imguiDrawData, commandBuffer);
		}
#endif
	}
}

} // namespace yae
