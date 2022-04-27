#include "ImGuiSystem.h"

#include <yae/Renderer.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <yae/renderers/vulkan/vulkan.h>
#include <yae/renderers/vulkan/VulkanRenderer.h>
#include <yae/renderers/vulkan/VulkanSwapChain.h>
#include <yae/renderers/vulkan/imgui_impl_vulkan.h>
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
	{
		bool ret = ImGui_ImplGlfw_InitForVulkan(_window, true);
		YAE_ASSERT(ret);
	}

#if YAE_IMPLEMENTS_RENDERER_VULKAN
	if (VulkanRenderer* vulkanRenderer = mirror::Cast<VulkanRenderer*>(m_renderer))
	{
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

		bool ret = ImGui_ImplVulkan_Init(&initInfo, vulkanRenderer->m_swapChain->m_renderPass);
		YAE_ASSERT(ret);

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

#if YAE_IMPLEMENTS_RENDERER_VULKAN
	if (VulkanRenderer* vulkanRenderer = mirror::Cast<VulkanRenderer*>(m_renderer))
	{
		ImGui_ImplVulkan_Shutdown();
		YAE_VERBOSE_CAT("imgui", "Shutdown ImGui");
	}
#endif

	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext(m_imguiContext);
	m_imguiContext = nullptr;
}

void ImGuiSystem::newFrame()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiSystem::render(FrameHandle _frameHandle)
{
	YAE_CAPTURE_FUNCTION();

#if YAE_IMPLEMENTS_RENDERER_VULKAN
	if (VulkanRenderer* vulkanRenderer = mirror::Cast<VulkanRenderer*>(m_renderer))
	{
		VkCommandBuffer commandBuffer = (VkCommandBuffer)_frameHandle;

		ImGui::Render();
		ImDrawData* imguiDrawData = ImGui::GetDrawData();
		const bool isMinimized = (imguiDrawData->DisplaySize.x <= 0.0f || imguiDrawData->DisplaySize.y <= 0.0f);
		if (!isMinimized)
		{
			ImGui_ImplVulkan_RenderDrawData(imguiDrawData, commandBuffer);
		}	
	}
#endif
}

} // namespace yae
