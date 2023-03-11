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

void ImGuiSystem::drawIm3dTextDrawLists(const Im3d::TextDrawList _textDrawLists[], u32 _count)
{
	Vector2 windowSize = renderer().getFrameBufferSize();
	Matrix4 viewProj = renderer().getViewProjectionMatrix();
	// Using ImGui here as a simple means of rendering text draw lists, however as with primitives the application is free to draw text in any conceivable  manner.

	// Invisible ImGui window which covers the screen.
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2((float)windowSize.x, (float)windowSize.y));
	ImGui::Begin("Invisible", nullptr, 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoFocusOnAppearing
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		);

	ImDrawList* imDrawList = ImGui::GetWindowDrawList();
	for (u32 i = 0; i < _count; ++i) 
	{
		const Im3d::TextDrawList& textDrawList = Im3d::GetTextDrawLists()[i];
		
		if (textDrawList.m_layerId == Im3d::MakeId("NamedLayer")) 
		{
			// The application may group primitives into layers, which can be used to change the draw state (e.g. enable depth testing, use a different shader)
		}

		for (u32 j = 0; j < textDrawList.m_textDataCount; ++j)
		{
			const Im3d::TextData& textData = textDrawList.m_textData[j];
			if (textData.m_positionSize.w == 0.0f || textData.m_color.getA() == 0.0f)
			{
				continue;
			}

			// Project world -> screen space.
			Vector4 clip = viewProj * Vector4(textData.m_positionSize.x, textData.m_positionSize.y, textData.m_positionSize.z, 1.0f);
			Vector2 screen = Vector2(clip.x / clip.w, clip.y / clip.w);
	
			// Cull text which falls offscreen. Note that this doesn't take into account text size but works well enough in practice.
			if (clip.w < 0.0f || screen.x >= 1.0f || screen.y >= 1.0f)
			{
				continue;
			}

			// Pixel coordinates for the ImGuiWindow ImGui.
			screen = screen * Vector2(0.5f) + Vector2(0.5f);
			screen.y = 1.0f - screen.y; // screen space origin is reversed by the projection.
			screen = screen * Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

			// All text data is stored in a single buffer; each textData instance has an offset into this buffer.
			const char* text = textDrawList.m_textBuffer + textData.m_textBufferOffset;

			// Calculate the final text size in pixels to apply alignment flags correctly.
			ImGui::SetWindowFontScale(textData.m_positionSize.w); // NB no CalcTextSize API which takes a font/size directly...
			Vector2 textSize = ImGui::CalcTextSize(text, text + textData.m_textLength);
			ImGui::SetWindowFontScale(1.0f);

			// Generate a pixel offset based on text flags.
			Vector2 textOffset = Vector2(-textSize.x * 0.5f, -textSize.y * 0.5f); // default to center
			if ((textData.m_flags & Im3d::TextFlags_AlignLeft) != 0)
			{
				textOffset.x = -textSize.x;
			}
			else if ((textData.m_flags & Im3d::TextFlags_AlignRight) != 0)
			{
				textOffset.x = 0.0f;
			}

			if ((textData.m_flags & Im3d::TextFlags_AlignTop) != 0)
			{
				textOffset.y = -textSize.y;
			}
			else if ((textData.m_flags & Im3d::TextFlags_AlignBottom) != 0)
			{
				textOffset.y = 0.0f;
			}

			// Add text to the window draw list.
			screen = screen + textOffset;
			imDrawList->AddText(nullptr, textData.m_positionSize.w * ImGui::GetFontSize(), screen, textData.m_color.getABGR(), text, text + textData.m_textLength);
		}
	}

	ImGui::End();
	ImGui::PopStyleColor(1);
}

} // namespace yae
