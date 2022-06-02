#include "VulkanRenderer.h"

#include "vulkan.h"
#include "VulkanSwapChain.h"
#include "im3d_impl_vulkan.h"
#include <yae/resources/ShaderResource.h>
#include <yae/resources/TextureResource.h>
#include <yae/math.h>

#include <glm/gtc/matrix_transform.hpp>
#include <im3d.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <GLFW/glfw3.h>

#include <set>
#include <vector>

namespace yae {

struct UniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct SimplePushConstantData
{
	alignas(16) glm::mat4 model;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT _messageType,
	const VkDebugUtilsMessengerCallbackDataEXT * _pCallbackData,
	void* _pUserData)
{
	if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		YAE_VERBOSEF_CAT("vulkan_internal", "%s", _pCallbackData->pMessage);
	}
	else if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		YAE_LOGF_CAT("vulkan_internal", "%s", _pCallbackData->pMessage);
	}
	else if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		YAE_WARNINGF_CAT("vulkan_internal", "%s", _pCallbackData->pMessage);
	}
	else if (_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		YAE_ERRORF_CAT("vulkan_internal", "%s", _pCallbackData->pMessage);
	}

	return VK_FALSE;
}

static void PopulateDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _createInfo)
{
	_createInfo = VkDebugUtilsMessengerCreateInfoEXT{};
	_createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_createInfo.pfnUserCallback = DebugCallback;
	_createInfo.pUserData = nullptr; // Optional
}

void VulkanRenderer::hintWindow()
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Disable OpenGL context creation
}

bool VulkanRenderer::init(GLFWwindow* _window)
{
	YAE_CAPTURE_FUNCTION();

	m_window = _window;

#if YAE_DEBUG
	m_validationLayersEnabled = true;
#else
	m_validationLayersEnabled = false;
#endif

	// Extensions
	std::vector<const char*> extensions;

	// GLFW extensions
	{
		u32 glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (u32 i = 0; i < glfwExtensionCount; ++i)
		{
			extensions.push_back(glfwExtensions[i]);
		}
	}

	// Message callback
	if (m_validationLayersEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	// Check extensions support
	{
		u32 extensionPropertiesCount;
		VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr));
		std::vector<VkExtensionProperties> extensionProperties(extensionPropertiesCount);
		VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties.data()));

		int16_t missingExtensionSupport = 0;
		for (size_t i = 0; i < extensions.size(); ++i)
		{
			const char* extensionName = extensions[i];
			bool found = false;
			for (u32 j = 0; j < extensionPropertiesCount; ++j)
			{
				if (strcmp(extensionName, extensionProperties[j].extensionName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				++missingExtensionSupport;
				YAE_ERRORF_CAT("vulkan", "Extension \"%s\" not supported.", extensionName);
			}
		}
		if (missingExtensionSupport > 0)
		{
			YAE_ERRORF_CAT("vulkan", "Can't create Vulkan m_instance, missing %d extensions support.", missingExtensionSupport);
			return false;
		}
	}

	// Validation Layers
	const char* validationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};
	const u32 VALIDATION_LAYERS_COUNT = u32(countof(validationLayers));

	// Check validation layers support
	if (m_validationLayersEnabled)
	{
		const u32 MAX_LAYER_COUNT = 64u;
		u32 layerCount = MAX_LAYER_COUNT;
		VkLayerProperties availableLayers[MAX_LAYER_COUNT];
		VK_VERIFY(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

		int16_t missingLayerSupport = 0;
		for (u32 i = 0; i < VALIDATION_LAYERS_COUNT; ++i)
		{
			const char* layerName = validationLayers[i];
			bool found = false;
			for (u32 j = 0; j < layerCount; ++j)
			{
				if (strcmp(layerName, availableLayers[j].layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				++missingLayerSupport;
				YAE_ERRORF_CAT("vulkan", "Layer \"%s\" not supported.", layerName);
			}
		}
		if (missingLayerSupport > 0)
		{
			YAE_ERRORF_CAT("vulkan", "Can't create Vulkan m_instance, missing %d layer support.", missingLayerSupport);
			return false;
		}
	}

	{
		YAE_VERBOSE_CAT("vulkan", "Creating Vulkan Instance...");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "yae";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = u32(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = 0;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		VkValidationFeaturesEXT features{};
		if (m_validationLayersEnabled)
		{
			createInfo.enabledLayerCount = VALIDATION_LAYERS_COUNT;
			createInfo.ppEnabledLayerNames = validationLayers;

			PopulateDebugUtilsMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;

			VkValidationFeatureEnableEXT enables[] = { VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT };
			features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			features.enabledValidationFeatureCount = 1;
			features.pEnabledValidationFeatures = enables;

			debugCreateInfo.pNext = &features;
		}

		VK_VERIFY(vkCreateInstance(&createInfo, nullptr, &m_instance));
		YAE_VERBOSE_CAT("vulkan", "Created Vulkan instance");
	}
	YAE_ASSERT(m_instance != VK_NULL_HANDLE);

	// Get extension functions pointers
	if (m_validationLayersEnabled)
	{
		vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (vkCreateDebugUtilsMessengerEXT == nullptr) {
			YAE_ERROR_CAT("vulkan", "Can't get \"vkCreateDebugUtilsMessengerEXT\". An extension is probably missing");
			return false;
		}

		vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (vkDestroyDebugUtilsMessengerEXT == nullptr) {
			YAE_ERROR_CAT("vulkan", "Can't get \"vkDestroyDebugUtilsMessengerEXT\". An extension is probably missing");
			return false;
		}
	}

	// Debug Messenger
	if (m_validationLayersEnabled)
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Debug Messenger...");
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugUtilsMessengerCreateInfo(createInfo);

		VK_VERIFY(vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
		YAE_VERBOSE_CAT("vulkan", "Created Debug Messenger");
	}

	// Create Surface
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Surface...");
		VK_VERIFY(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));
		YAE_VERBOSE_CAT("vulkan", "Created Surface");
	}
	YAE_ASSERT(m_surface != VK_NULL_HANDLE);

	// Select Physical Device
	const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const size_t deviceExtensionCount = countof(deviceExtensions);
	{
		YAE_VERBOSE_CAT("vulkan", "Picking Physical Device...");

		u32 availablePhysicalDeviceCount;
		VK_VERIFY(vkEnumeratePhysicalDevices(m_instance, &availablePhysicalDeviceCount, nullptr));
		YAE_ASSERT_MSG(availablePhysicalDeviceCount > 0, "Failed to find any GPU with Vulkan support");
		std::vector<VkPhysicalDevice> availablePhysicalDevices(availablePhysicalDeviceCount);
		VK_VERIFY(vkEnumeratePhysicalDevices(m_instance, &availablePhysicalDeviceCount, availablePhysicalDevices.data()));

		auto GetDeviceScore = [deviceExtensions](VkPhysicalDevice _device, VkSurfaceKHR _surface) -> u32
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(_device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(_device, &deviceFeatures);

			// Application can't function without geometry shaders
			if (deviceFeatures.geometryShader == 0)
				return 0;

			if (deviceFeatures.samplerAnisotropy == 0)
				return 0;

			QueueFamilyIndices queueFamilyIndices = vulkan::findQueueFamilies(_device, _surface);
			if (!queueFamilyIndices.isComplete())
				return 0;

			SwapChainSupportDetails swapChainSupport = vulkan::querySwapChainSupport(_device, _surface);
			if (!swapChainSupport.isValid())
				return 0;

			if (!CheckDeviceExtensionSupport(_device, deviceExtensions, countof(deviceExtensions)))
				return 0;

			u32 score = 0;
			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			return score;
		};

		u32 highestDeviceScore = 0;
		for (const VkPhysicalDevice& physicalDevice : availablePhysicalDevices)
		{
			u32 deviceScore = GetDeviceScore(physicalDevice, m_surface);
			if (deviceScore > 0 && deviceScore > highestDeviceScore)
			{
				m_physicalDevice = physicalDevice;
				highestDeviceScore = deviceScore;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			YAE_ERROR_CAT("vulkan", "Failed to find any suitable GPU");
			return false;
		}

		m_queueIndices = vulkan::findQueueFamilies(m_physicalDevice, m_surface);
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
		YAE_VERBOSEF_CAT("vulkan", "Picked physical device \"%s\"", m_physicalDeviceProperties.deviceName);
	}
	YAE_ASSERT(m_physicalDevice != VK_NULL_HANDLE);

	// Create Logical Device
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Logical Device...");

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.geometryShader = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pEnabledFeatures = &deviceFeatures;
		if (m_validationLayersEnabled)
		{
			createInfo.enabledLayerCount = VALIDATION_LAYERS_COUNT;
			createInfo.ppEnabledLayerNames = validationLayers;
		}
		createInfo.enabledExtensionCount = deviceExtensionCount;
		createInfo.ppEnabledExtensionNames = deviceExtensions;

		// Create queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<u32> uniqueQueueFamilies = { m_queueIndices.graphicsFamily, m_queueIndices.presentFamily };
		float queuePriority = 1.0f;
		for (u32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
		createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		VK_VERIFY(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
		YAE_VERBOSE_CAT("vulkan", "Created Logical Device");
	}
	YAE_ASSERT(m_device != VK_NULL_HANDLE);

	// Create Allocator
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Allocator...");

		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
		allocatorInfo.physicalDevice = m_physicalDevice;
		allocatorInfo.device = m_device;
		allocatorInfo.instance = m_instance;
		VK_VERIFY(vmaCreateAllocator(&allocatorInfo, &m_allocator));

		YAE_VERBOSE_CAT("vulkan", "Created Allocator");
	}

	// Get Queues
	vkGetDeviceQueue(m_device, m_queueIndices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, m_queueIndices.presentFamily, 0, &m_presentQueue);

	// Create Command Pools
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Command Pool...");
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_queueIndices.graphicsFamily;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;;
		VK_VERIFY(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
		YAE_VERBOSE_CAT("vulkan", "Created Command Pool");
	}

	// Create Descriptor Pools
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Descriptor Pools...");

		const u32 POOL_SIZE = 1000;
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, POOL_SIZE },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, POOL_SIZE }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = POOL_SIZE * u32(countof(pool_sizes));
		pool_info.poolSizeCount = u32(countof(pool_sizes));
		pool_info.pPoolSizes = pool_sizes;
		VK_VERIFY(vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool));
		YAE_VERBOSE_CAT("vulkan", "Created Descriptor Pools");
	}

	// Create Descriptors Set Layout
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Descriptors Set Layout...");

		VkDescriptorSetLayoutBinding uboLayoutBinding;
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding;
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding bindings[] =
		{
			uboLayoutBinding,
			samplerLayoutBinding
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = u32(countof(bindings));
		layoutInfo.pBindings = bindings;
		VK_VERIFY(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout));
		YAE_VERBOSE_CAT("vulkan", "Created Descriptors Set Layout");
	}

	int windowWidth, windowHeight;
	glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
	VkExtent2D windowExtent = {
		u32(windowWidth),
		u32(windowHeight)
	};
	// Create Swap Chain
	m_swapChain = defaultAllocator().create<VulkanSwapChain>();
	m_swapChain->init(
		m_physicalDevice,
		m_device,
		m_allocator,
		m_surface,
		windowExtent
	);

	// Frame Info
	m_texture = findOrCreateResource<TextureResource>("./data/textures/viking_room.png");
	m_texture->useLoad();
	YAE_ASSERT(m_texture->isLoaded());

	m_frameInfos.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);
	_createCommandBuffers();
	_createDescriptorSet();

	_createPipeline();

	return true;
}


void VulkanRenderer::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	_destroyPipeline();

	_destroyDescriptorSet();
	_destroyCommandBuffers();
	m_frameInfos.clear();

	m_texture->releaseUnuse();
	m_texture = nullptr;

	m_swapChain->shutdown();
	defaultAllocator().destroy(m_swapChain);
	m_swapChain = nullptr;
	//_destroySwapChain();

	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	m_descriptorSetLayout = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Descriptor Set Layout");

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	m_descriptorPool = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Descriptor Sets");

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	m_commandPool = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Command Pool");

	m_presentQueue = VK_NULL_HANDLE;
	m_graphicsQueue = VK_NULL_HANDLE;

	vmaDestroyAllocator(m_allocator);
	m_allocator = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Allocator");

	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;
	m_physicalDevice = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Device");

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	m_surface = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Surface");

	if (m_validationLayersEnabled)
	{
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		m_debugMessenger = VK_NULL_HANDLE;
		YAE_VERBOSE_CAT("vulkan", "Destroyed Debug Messenger");
	}

	vkDestroyInstance(m_instance, nullptr);
	m_instance = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Vulkan instance");

	m_window = nullptr;
}

FrameHandle VulkanRenderer::beginFrame()
{
	YAE_ASSERT_MSG(m_currentFlightImageIndex == ~0u, "Can't call beginFrame while already in progress");

	VkResult result = m_swapChain->acquireNextImage(&m_currentFlightImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		//shutdownImGui();
		_recreateSwapChain();
		//initImGui();
		return nullptr;
	}
	YAE_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	// Update uniform buffers
	{
		UniformBufferObject ubo{};
		ubo.view = m_viewMatrix;
		ubo.proj = m_projMatrix;
		void* data;
		VK_VERIFY(vmaMapMemory(m_allocator, m_frameInfos[m_currentFrameIndex].uniformBufferMemory, &data));
		memcpy(data, &ubo, sizeof(ubo));
		vmaUnmapMemory(m_allocator, m_frameInfos[m_currentFrameIndex].uniformBufferMemory);	
	}

	VkCommandBuffer commandBuffer = m_frameInfos[m_currentFrameIndex].commandBuffer;
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	beginSwapChainRenderPass(commandBuffer);

	return commandBuffer;
}

void VulkanRenderer::endFrame()
{
	YAE_ASSERT_MSG(m_currentFlightImageIndex != ~0u, "Can't call endFrame while frame is not in progress");
	VkCommandBuffer commandBuffer = m_frameInfos[m_currentFrameIndex].commandBuffer;

	endSwapChainRenderPass(commandBuffer);

	VK_VERIFY(vkEndCommandBuffer(commandBuffer));

	VkResult result = m_swapChain->submitCommandBuffers(&commandBuffer, 1, &m_currentFlightImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		m_framebufferResized = false;
		//shutdownImGui();
		_recreateSwapChain();
		//initImGui();
	}
	else
	{
		YAE_ASSERT_MSG(result == VK_SUCCESS, "failed to present swap chain image!");
	}

	m_currentFlightImageIndex = ~0;
	m_currentFrameIndex = (m_currentFrameIndex + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::beginSwapChainRenderPass(VkCommandBuffer _commandBuffer)
{
	YAE_ASSERT_MSG(m_currentFlightImageIndex != ~0u, "Can't call beginSwapChainRenderPass if frame is not in progress");
	YAE_ASSERT_MSG(_commandBuffer == m_frameInfos[m_currentFrameIndex].commandBuffer, "Can't begin render pass on command buffer from a different frame");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain->getRenderPass();
	renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentFlightImageIndex);

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = m_swapChain->getExtent();

	VkClearValue clearValues[2] = {};
	clearValues[0].color = {{0.01f, 0.01f, 0.01f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};
	renderPassInfo.clearValueCount = u32(countof(clearValues));
	renderPassInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChain->getExtent().width);
	viewport.height = static_cast<float>(m_swapChain->getExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{{0, 0}, m_swapChain->getExtent()};
	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void VulkanRenderer::endSwapChainRenderPass(VkCommandBuffer _commandBuffer)
{
	YAE_ASSERT_MSG(m_currentFlightImageIndex != ~0u, "Can't call endSwapChainRenderPass if frame is not in progress");
	YAE_ASSERT_MSG(_commandBuffer == m_frameInfos[m_currentFrameIndex].commandBuffer, "Can't end render pass on command buffer from a different frame");

	vkCmdEndRenderPass(_commandBuffer);
}

void VulkanRenderer::drawMesh(const Matrix4& _transform, const MeshHandle& _meshHandle)
{
	DrawCommand command;
	command.transform = _transform;
	command.mesh = _meshHandle;
	m_drawCommands.push_back(command);
}

void VulkanRenderer::drawCommands(FrameHandle _frameHandle)
{
	YAE_CAPTURE_FUNCTION();

	VkCommandBuffer commandBuffer = (VkCommandBuffer) _frameHandle;

	FrameInfo& frame = m_frameInfos[m_currentFrameIndex];
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	for (u32 i = 0; i < m_drawCommands.size(); ++i)
	{
		DrawCommand& command = m_drawCommands[i];

		// Matrix
		SimplePushConstantData push{};
		push.model = command.transform;
		vkCmdPushConstants(
			commandBuffer,
			m_pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(SimplePushConstantData),
			&push
		);

		// Vertices
		VkBuffer vertexBuffers[] = { command.mesh.vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, command.mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &frame.descriptorSet, 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, u32(command.mesh.indicesCount), 1, 0, 0, 0);
	}

	m_drawCommands.clear();
}

void VulkanRenderer::drawIm3d(VkCommandBuffer _commandBuffer)
{
	YAE_CAPTURE_FUNCTION();

	im3d_Render(m_im3dInstance, _commandBuffer, m_currentFrameIndex, Matrix4(m_projMatrix * m_viewMatrix), getFrameBufferSize());
}

void VulkanRenderer::notifyFrameBufferResized(int _width, int _height)
{
	m_framebufferResized = true;
}


bool VulkanRenderer::createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle)
{
	YAE_CAPTURE_FUNCTION();

	_outTextureHandle = {};

	VkDeviceSize imageSize = _width * _height * 4;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferMemory = VK_NULL_HANDLE;
	vulkan::createOrResizeBuffer(
		m_allocator,
		stagingBuffer,
		stagingBufferMemory,
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);

	void* data;
	VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
	memcpy(data, _data, static_cast<size_t>(imageSize));
	vmaUnmapMemory(m_allocator, stagingBufferMemory);

	vulkan::createImage(
		m_allocator,
		_width,
		_height,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_outTextureHandle.image,
		_outTextureHandle.memory
	);

	_transitionImageLayout(_outTextureHandle.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	_copyBufferToImage(stagingBuffer, _outTextureHandle.image, _width, _height);
	_transitionImageLayout(_outTextureHandle.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vulkan::destroyBuffer(m_allocator, stagingBuffer, stagingBufferMemory);

	_outTextureHandle.view = vulkan::createImageView(m_device, _outTextureHandle.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	// Create Texture Sampler
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = m_physicalDeviceProperties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.f;
		samplerInfo.minLod = 0.f;
		samplerInfo.maxLod = 0.f;

		VK_VERIFY(vkCreateSampler(m_device, &samplerInfo, nullptr, &_outTextureHandle.sampler));
		YAE_VERBOSE_CAT("vulkan", "Created Texture Sampler");
	}

	return true;
}


void VulkanRenderer::destroyTexture(TextureHandle& _inTextureHandle)
{
	YAE_CAPTURE_FUNCTION();

	vkDestroySampler(m_device, _inTextureHandle.sampler, nullptr);
	_inTextureHandle.sampler = VK_NULL_HANDLE;
	vkDestroyImageView(m_device, _inTextureHandle.view, nullptr);
	_inTextureHandle.view = VK_NULL_HANDLE;
	vulkan::destroyImage(m_allocator, _inTextureHandle.image, _inTextureHandle.memory);
}


bool VulkanRenderer::createMesh(Vertex* _vertices, u32 _verticesCount, u32* _indices, u32 _indicesCount, MeshHandle& _outMeshHandle)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(_vertices != nullptr || _verticesCount == 0);
	YAE_ASSERT(_indices != nullptr || _indicesCount == 0);

	_outMeshHandle = {};

	// Create Vertex Buffer
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Vertex Buffer...");

		VkDeviceSize bufferSize = sizeof(*_vertices) * _verticesCount;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferMemory = VK_NULL_HANDLE;
		vulkan::createOrResizeBuffer(
			m_allocator,
			stagingBuffer,
			stagingBufferMemory,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		{
			void* data;
			VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
			memcpy(data, _vertices, bufferSize);
			vmaUnmapMemory(m_allocator, stagingBufferMemory);
		}
		vulkan::createOrResizeBuffer(
			m_allocator,
			_outMeshHandle.vertexBuffer,
			_outMeshHandle.vertexMemory,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		vulkan::copyBuffer(m_device, m_commandPool, m_graphicsQueue, stagingBuffer, _outMeshHandle.vertexBuffer, bufferSize);

		vulkan::destroyBuffer(m_allocator, stagingBuffer, stagingBufferMemory);
		YAE_VERBOSE_CAT("vulkan", "Created Vertex Buffer");
	}

	// Create Index Buffer
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Index Buffer...");
		VkDeviceSize bufferSize = sizeof(*_indices) * _indicesCount;
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferMemory = VK_NULL_HANDLE;
		vulkan::createOrResizeBuffer(
			m_allocator,
			stagingBuffer,
			stagingBufferMemory,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		{
			void* data;
			VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
			memcpy(data, _indices, bufferSize);
			vmaUnmapMemory(m_allocator, stagingBufferMemory);
		}
		vulkan::createOrResizeBuffer(
			m_allocator,
			_outMeshHandle.indexBuffer,
			_outMeshHandle.indexMemory,
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		vulkan::copyBuffer(m_device, m_commandPool, m_graphicsQueue, stagingBuffer, _outMeshHandle.indexBuffer, bufferSize);

		vulkan::destroyBuffer(m_allocator, stagingBuffer, stagingBufferMemory);

		_outMeshHandle.indicesCount = _indicesCount;

		YAE_VERBOSE_CAT("vulkan", "Created Index Buffer");
	}

	return true;
}


bool VulkanRenderer::createShader(const void* _code, size_t _codeSize, ShaderHandle& _outShaderHandle)
{
	YAE_CAPTURE_FUNCTION();

	// @NOTE(remi): We should compile the shader on the go using the shaderc lib

	YAE_ASSERT(_outShaderHandle.shaderModule == VK_NULL_HANDLE);

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = _codeSize;
	createInfo.pCode = reinterpret_cast<const u32*>(_code);

	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &_outShaderHandle.shaderModule) != VK_SUCCESS)
	{
		YAE_ERROR_CAT("vulkan", "Failed to create shader module");
		return false;
	}
	
	return true;
}


void VulkanRenderer::destroyShader(ShaderHandle& _shaderHandle)
{
	YAE_CAPTURE_FUNCTION();
	vkDestroyShaderModule(m_device, _shaderHandle.shaderModule, nullptr);
	_shaderHandle.shaderModule = VK_NULL_HANDLE;
}


void VulkanRenderer::destroyMesh(MeshHandle& _inMeshHandle)
{
	YAE_CAPTURE_FUNCTION();

	vulkan::destroyBuffer(m_allocator, _inMeshHandle.indexBuffer, _inMeshHandle.indexMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Index Buffer");

	vulkan::destroyBuffer(m_allocator, _inMeshHandle.vertexBuffer, _inMeshHandle.vertexMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Vertex Buffer");

	_inMeshHandle.indexBuffer = VK_NULL_HANDLE;
	_inMeshHandle.indexMemory = VK_NULL_HANDLE;
	_inMeshHandle.vertexBuffer = VK_NULL_HANDLE;
	_inMeshHandle.vertexMemory = VK_NULL_HANDLE;
}

Vector2 VulkanRenderer::getFrameBufferSize() const
{
	VkExtent2D extent = m_swapChain->getExtent();
	return Vector2(float(extent.width), float(extent.height));
}

void VulkanRenderer::waitIdle()
{
	YAE_CAPTURE_FUNCTION();
	
	VK_VERIFY(vkDeviceWaitIdle(m_device));
}

void VulkanRenderer::initIm3d()
{
	YAE_CAPTURE_FUNCTION();

	im3d_VulkanInitData initData{};
	initData.device = m_device;
	initData.allocator = m_allocator;
	initData.extent = m_swapChain->getExtent();
	initData.renderPass = m_swapChain->m_renderPass;
	initData.descriptorPool = m_descriptorPool;
	m_im3dInstance = im3d_Init(initData);
}

void VulkanRenderer::reloadIm3dShaders()
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_im3dInstance != nullptr);
	im3d_DestroyPipelines(m_im3dInstance);
	im3d_CreatePipelines(m_im3dInstance);
}

void VulkanRenderer::shutdownIm3d()
{
	YAE_CAPTURE_FUNCTION();
	
	im3d_Shutdown(m_im3dInstance);
	m_im3dInstance = nullptr;
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice _physicalDevice, const char* const* _extensionsList, size_t _extensionCount)
{
	u32 deviceExtensionCount;
	VK_VERIFY(vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, nullptr));
	std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
	VK_VERIFY(vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions.data()));

	for (size_t i = 0; i < _extensionCount; ++i)
	{
		bool found = false;
		const char* extensionName = _extensionsList[i];
		for (u32 j = 0; j < deviceExtensionCount; ++j)
		{
			if (strcmp(extensionName, deviceExtensions[j].extensionName) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
			return false;
	}
	return true;
}


bool VulkanRenderer::HasStencilComponent(VkFormat _format)
{
	return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
}


void VulkanRenderer::_recreateSwapChain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device);

	// Reinit Swap Chain
	YAE_ASSERT_MSG(m_swapChain != nullptr, "There must already be a valid SwapChain in order to recreate it.");
	VkExtent2D windowExtent = {
		u32(width),
		u32(height)
	};
	m_swapChain->reinit(
		m_physicalDevice,
		m_device,
		m_allocator,
		m_surface,
		windowExtent
	);
}

void VulkanRenderer::_createCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	YAE_ASSERT(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT == m_frameInfos.size());
	for (u32 i = 0; i < m_frameInfos.size(); ++i)
	{
		VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, &m_frameInfos[i].commandBuffer));
	}
}

void VulkanRenderer::_destroyCommandBuffers()
{
	YAE_ASSERT(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT == m_frameInfos.size());
	for (u32 i = 0; i < m_frameInfos.size(); ++i)
	{
		vkFreeCommandBuffers(
			m_device,
			m_commandPool,
			1,
			&m_frameInfos[i].commandBuffer
		);
	}
}

void VulkanRenderer::_createDescriptorSet()
{
	VkDescriptorSetLayout layouts[VulkanSwapChain::MAX_FRAMES_IN_FLIGHT];
	VkDescriptorSet descriptorSets[VulkanSwapChain::MAX_FRAMES_IN_FLIGHT];
	// Create DescriptorSets
	{
		for (u32 i = 0; i < countof(layouts); ++i ) { layouts[i] = m_descriptorSetLayout; }

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = u32(countof(layouts));
		allocInfo.pSetLayouts = layouts;

		VK_VERIFY(vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets));
	}

	YAE_ASSERT(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT == m_frameInfos.size());
	for (u32 i = 0; i < m_frameInfos.size(); ++i)
	{
		// Create Buffer
		vulkan::createOrResizeBuffer(
			m_allocator,
			m_frameInfos[i].uniformBuffer,
			m_frameInfos[i].uniformBufferMemory,
			sizeof(UniformBufferObject),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		// Update DescriptorSets
		m_frameInfos[i].descriptorSet = descriptorSets[i];

		VkWriteDescriptorSet descriptorWrites[2] = {};
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_frameInfos[i].uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_frameInfos[i].descriptorSet;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_texture->getTextureHandle().view;
		imageInfo.sampler = m_texture->getTextureHandle().sampler;
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_frameInfos[i].descriptorSet;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = nullptr;
		descriptorWrites[1].pImageInfo = &imageInfo;
		descriptorWrites[1].pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(m_device, u32(countof(descriptorWrites)), descriptorWrites, 0, nullptr);
	}
}

void VulkanRenderer::_destroyDescriptorSet()
{
	YAE_ASSERT(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT == m_frameInfos.size());
	for (u32 i = 0; i < m_frameInfos.size(); ++i)
	{
		vkFreeDescriptorSets(m_device, m_descriptorPool, 1, &m_frameInfos[i].descriptorSet);

		vulkan::destroyBuffer(
			m_allocator,
			m_frameInfos[i].uniformBuffer,
			m_frameInfos[i].uniformBufferMemory
		);
	}
}

void VulkanRenderer::_createPipeline()
{
	ShaderResource* vertexShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.vert", SHADERTYPE_VERTEX, "main");
	vertexShader->useLoad();
	YAE_ASSERT(vertexShader->isLoaded());

	ShaderResource* fragmentShader = findOrCreateResource<ShaderResource>("./data/shaders/shader.frag", SHADERTYPE_FRAGMENT, "main");
	fragmentShader->useLoad();
	YAE_ASSERT(fragmentShader->isLoaded());

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = vertexShader->getShaderHandle().shaderModule;
	vertexShaderStageInfo.pName = vertexShader->getEntryPoint();

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = fragmentShader->getShaderHandle().shaderModule;
	fragmentShaderStageInfo.pName = fragmentShader->getEntryPoint();

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertexShaderStageInfo,
		fragmentShaderStageInfo
	};

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributeDescriptions[3];
	attributeDescriptions[0] = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1] = {};
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	attributeDescriptions[2] = {};
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = u32(countof(attributeDescriptions));
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.f;
	rasterizer.depthBiasClamp = 0.f;
	rasterizer.depthBiasSlopeFactor = 0.f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	/*colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional*/

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	VK_VERIFY(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // optional
	depthStencil.back = {}; // optional

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_swapChain->m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	VK_VERIFY(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));

	fragmentShader->releaseUnuse();
	vertexShader->releaseUnuse();

	YAE_VERBOSE_CAT("vulkan", "Created Graphics Pipeline");
}

void VulkanRenderer::_destroyPipeline()
{
	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	m_graphicsPipeline = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	m_pipelineLayout = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Graphic Pipeline");
}

void VulkanRenderer::_transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
{
	VkCommandBuffer commandBuffer = vulkan::beginSingleTimeCommands(m_device, m_commandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = _oldLayout;
	barrier.newLayout = _newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = _image;
	if (_newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (HasStencilComponent(_format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	
	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;
	if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		YAE_ASSERT_MSG(false, "Unsupported layout transition");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	vulkan::endSingleTimeCommands(m_device, m_commandPool, m_graphicsQueue, commandBuffer);
}

void VulkanRenderer::_copyBufferToImage(VkBuffer _buffer, VkImage _image, u32 _width, u32 _height)
{
	VkCommandBuffer commandBuffer = vulkan::beginSingleTimeCommands(m_device, m_commandPool);

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = {0, 0, 0};
	region.imageExtent = { _width, _height, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		_buffer,
		_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region
	);

	vulkan::endSingleTimeCommands(m_device, m_commandPool, m_graphicsQueue, commandBuffer);
}

} // namespace yae
