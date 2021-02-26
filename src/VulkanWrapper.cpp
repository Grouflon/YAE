#include "VulkanWrapper.h"

#include <set>
#include <vector>

#include <00-Macro/Assert.h>
#include <00-Type/IntTypes.h>
#include <02-Log/Log.h>
#include <03-Resource/FileResource.h>
#include <03-Resource/ResourceManager.h>

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

namespace yae {

const u32 MAX_EXTENSION_COUNT = 64u;

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

const u32 INVALID_QUEUE = ~0u;
struct QueueFamilyIndices
{
	u32 graphicsFamily = INVALID_QUEUE;
	u32 presentFamily = INVALID_QUEUE;

	bool isComplete() const
	{
		if (graphicsFamily == INVALID_QUEUE)
			return false;

		if (presentFamily == INVALID_QUEUE)
			return false;

		return true;
	}
};
static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	QueueFamilyIndices queueFamilyIndices;

	const u32 MAX_QUEUE_FAMILY_COUNT = 64u;
	u32 queueFamilyCount = MAX_QUEUE_FAMILY_COUNT;
	VkQueueFamilyProperties queueFamilyProperties[MAX_QUEUE_FAMILY_COUNT];
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilyProperties);
	YAE_ASSERT(queueFamilyCount < MAX_QUEUE_FAMILY_COUNT);

	for (u32 i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		VK_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, _surface, &presentSupport));
		if (presentSupport)
		{
			queueFamilyIndices.presentFamily = i;
		}

		if (queueFamilyIndices.isComplete())
		{
			break;
		}
	}

	return queueFamilyIndices;
}

const u32 MAX_FORMAT_COUNT = 16u;
const u32 MAX_PRESENTMODE_COUNT = 16u;
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR formats[MAX_FORMAT_COUNT];
	u32 formatCount = 0;
	VkPresentModeKHR presentModes[MAX_PRESENTMODE_COUNT];
	u32 presentModeCount = 0;

	bool isValid() const
	{
		return formatCount > 0 && presentModeCount > 0;
	}
};
static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	SwapChainSupportDetails details;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, _surface, &details.capabilities));

	details.formatCount = MAX_FORMAT_COUNT;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &details.formatCount, details.formats));

	details.presentModeCount = MAX_PRESENTMODE_COUNT;
	VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &details.presentModeCount, details.presentModes));

	return details;
}

bool ChooseSwapSurfaceFormat(const VkSurfaceFormatKHR* _availableFormats, u32 _availableFormatCount, VkSurfaceFormatKHR* _outSurfaceFormat)
{
	for (u32 i = 0; i < _availableFormatCount; ++i)
	{
		if (_availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && _availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			if (_outSurfaceFormat) *_outSurfaceFormat = _availableFormats[i];
			return true;
		}
	}

	if (_availableFormatCount > 0)
	{
		if (_outSurfaceFormat) *_outSurfaceFormat = _availableFormats[0];
		return true;
	}
	return false;
}

VkPresentModeKHR ChooseSwapPresentMode(const VkPresentModeKHR* _availablePresentModes, u32 _availablePresentModeCount)
{
	for (u32 i = 0; i < _availablePresentModeCount; ++i)
	{
		if (_availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return _availablePresentModes[i];
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities, GLFWwindow* _window)
{
	if (_capabilities.currentExtent.width != UINT32_MAX)
	{
		return _capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		actualExtent.width = std::max(_capabilities.minImageExtent.width, std::min(_capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(_capabilities.minImageExtent.height, std::min(_capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

bool VulkanWrapper::Init(GLFWwindow* _window, bool _validationLayersEnabled)
{
	m_window = _window;
	m_validationLayersEnabled = _validationLayersEnabled;

	// Extensions
	u32 extensionCount = 0;
	const char* extensions[MAX_EXTENSION_COUNT];

	// GLFW extensions
	{
		u32 glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		YAE_ASSERT(glfwExtensionCount < MAX_EXTENSION_COUNT - extensionCount);
		for (u32 i = 0; i < glfwExtensionCount; ++i)
		{
			extensions[extensionCount] = glfwExtensions[i];
			++extensionCount;
		}
	}

	// Message callback
	if (m_validationLayersEnabled)
	{
		YAE_ASSERT(extensionCount < MAX_EXTENSION_COUNT);
		extensions[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		++extensionCount;
	}

	// Check extensions support
	{
		u32 extensionPropertiesCount = MAX_EXTENSION_COUNT;
		VkExtensionProperties extensionProperties[MAX_EXTENSION_COUNT];
		VK_VERIFY(vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionProperties));

		int16_t missingExtensionSupport = 0;
		for (u32 i = 0; i < extensionCount; ++i)
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
			return EXIT_FAILURE;
		}
	}

	// Validation Layers
	const char* validationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
	};
	const u32 VALIDATION_LAYERS_COUNT = sizeof(validationLayers) / sizeof(*validationLayers);

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
			return EXIT_FAILURE;
		}
	}

	{
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
		createInfo.enabledExtensionCount = extensionCount;
		createInfo.ppEnabledExtensionNames = extensions;
		createInfo.enabledLayerCount = 0;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_validationLayersEnabled)
		{
			createInfo.enabledLayerCount = VALIDATION_LAYERS_COUNT;
			createInfo.ppEnabledLayerNames = validationLayers;

			PopulateDebugUtilsMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
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
			YAE_ERRORF_CAT("vulkan", "Can't get \"vkCreateDebugUtilsMessengerEXT\". An extension is probably missing");
			return EXIT_FAILURE;
		}

		vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		if (vkDestroyDebugUtilsMessengerEXT == nullptr) {
			YAE_ERRORF_CAT("vulkan", "Can't get \"vkDestroyDebugUtilsMessengerEXT\". An extension is probably missing");
			return EXIT_FAILURE;
		}
	}

	// Debug Messenger
	if (m_validationLayersEnabled)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugUtilsMessengerCreateInfo(createInfo);

		VK_VERIFY(vkCreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
		YAE_VERBOSE_CAT("vulkan", "Created Debug Messenger");
	}

	// Create Surface
	{
		VK_VERIFY(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));
		YAE_VERBOSE_CAT("vulkan", "Created Surface");
	}
	YAE_ASSERT(m_surface != VK_NULL_HANDLE);

	// Select Physical Device
	const char* deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const u32 deviceExtensionCount = sizeof(deviceExtensions) / sizeof(*deviceExtensions);
	{
		const u32 MAX_PHYSICAL_DEVICES = 32u;
		u32 availablePhysicalDeviceCount = MAX_PHYSICAL_DEVICES;
		VkPhysicalDevice availablePhysicalDevices[MAX_PHYSICAL_DEVICES];
		VK_VERIFY(vkEnumeratePhysicalDevices(m_instance, &availablePhysicalDeviceCount, availablePhysicalDevices));

		if (availablePhysicalDeviceCount == 0)
		{
			YAE_ERROR_CAT("vulkan", "Failed to find any GPU with Vulkan support");
			return EXIT_FAILURE;
		}

		auto GetDeviceScore = [deviceExtensions, deviceExtensionCount](VkPhysicalDevice _device, VkSurfaceKHR _surface) -> u32
		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(_device, &deviceProperties);

			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(_device, &deviceFeatures);

			// Application can't function without geometry shaders
			if (deviceFeatures.geometryShader == 0)
				return 0;

			QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_device, _surface);
			if (!queueFamilyIndices.isComplete())
				return 0;

			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_device, _surface);
			if (!swapChainSupport.isValid())
				return 0;

			if (!CheckDeviceExtensionSupport(_device, deviceExtensions, deviceExtensionCount))
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
		for (u32 i = 0; i < availablePhysicalDeviceCount; ++i)
		{
			u32 deviceScore = GetDeviceScore(availablePhysicalDevices[i], m_surface);
			if (deviceScore > 0 && deviceScore > highestDeviceScore)
			{
				m_physicalDevice = availablePhysicalDevices[i];
				highestDeviceScore = deviceScore;
				break;
			}
		}

		if (m_physicalDevice == VK_NULL_HANDLE)
		{
			YAE_ERROR_CAT("vulkan", "Failed to find any suitable GPU");
			return EXIT_FAILURE;
		}

		{
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
			YAE_VERBOSEF_CAT("vulkan", "Picked physical device \"%s\"", deviceProperties.deviceName);
		}
	}
	YAE_ASSERT(m_physicalDevice != VK_NULL_HANDLE);
	QueueFamilyIndices queueIndices = FindQueueFamilies(m_physicalDevice, m_surface);

	// Create Logical Device
	{
		VkPhysicalDeviceFeatures deviceFeatures{};

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
		std::set<u32> uniqueQueueFamilies = { queueIndices.graphicsFamily, queueIndices.presentFamily };
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

	// Get Queues
	vkGetDeviceQueue(m_device, queueIndices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, queueIndices.presentFamily, 0, &m_presentQueue);

	// Create Swap Chain
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);
		VkSurfaceFormatKHR surfaceFormat;
		{
			bool result = ChooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatCount, &surfaceFormat);
			YAE_ASSERT(result);
		}
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModeCount);
		VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, m_window);
		u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		/*
* 		The imageUsage bit field specifies what kind of operations we'll use the images in the swap chain for.
		In this tutorial we're going to render directly to them, which means that they're used as color attachment.
		It is also possible that you'll render images to a separate image first to perform operations like post-processing.
		In that case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a swap chain image.
		*/
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		u32 queueFamilyIndices[] = { queueIndices.graphicsFamily, queueIndices.presentFamily };
		if (queueIndices.graphicsFamily != queueIndices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		VK_VERIFY(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));
		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
		YAE_VERBOSE_CAT("vulkan", "Created Swap Chain");
	}

	// Get Swap chain images;
	{
		u32 imageCount;
		VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr));
		m_swapChainImages.resize(imageCount);
		VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data()));
	}

	// Create render pass
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VK_VERIFY(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
		YAE_VERBOSE_CAT("vulkan", "Created Render Pass");
	}

	// Create pipeline
	{
		auto createShaderModule = [](VkDevice _device, const void* _code, size_t _codeSize) -> VkShaderModule
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = _codeSize;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(_code);

			VkShaderModule shaderModule;
			VK_VERIFY(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
			return shaderModule;
		};

		VkShaderModule vertexShaderModule;
		{
			FileResource* file = FindOrCreateResource<FileResource>("./data/shaders/vert.spv");
			file->useLoad();
			YAE_ASSERT(file->getError() == Resource::ERROR_NONE);
			vertexShaderModule = createShaderModule(m_device, file->getContent(), file->getContentSize());
			file->releaseUnuse();
		}

		VkShaderModule fragmentShaderModule;
		{
			FileResource* file = FindOrCreateResource<FileResource>("./data/shaders/frag.spv");
			file->useLoad();
			YAE_ASSERT(file->getError() == Resource::ERROR_NONE);
			fragmentShaderModule = createShaderModule(m_device, file->getContent(), file->getContentSize());
			file->releaseUnuse();
		}
	
		VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderStageInfo.module = vertexShaderModule;
		vertexShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{};
		fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderStageInfo.module = fragmentShaderModule;
		fragmentShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {
			vertexShaderStageInfo,
			fragmentShaderStageInfo
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapChainExtent.width;
		viewport.height = (float)m_swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = m_swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		VK_VERIFY(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_VERIFY(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));

		vkDestroyShaderModule(m_device, fragmentShaderModule, nullptr);
		vkDestroyShaderModule(m_device, vertexShaderModule, nullptr);

		YAE_VERBOSE_CAT("vulkan", "Created Graphics Pipeline");
	}
	
	return true;
}

void VulkanWrapper::Shutdown()
{
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	m_pipelineLayout = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Graphic Pipeline");

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	m_renderPass = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Render Pass");

	m_swapChainImages.clear();

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Swap Chain");

	m_presentQueue = VK_NULL_HANDLE;
	m_graphicsQueue = VK_NULL_HANDLE;

	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;
	m_physicalDevice = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Logical Device");

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

bool VulkanWrapper::CheckDeviceExtensionSupport(VkPhysicalDevice _physicalDevice, const char* const* _extensionsList, u32 _extensionCount)
{
	u32 deviceExtensionCount;
	VK_VERIFY(vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, nullptr));
	std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
	VK_VERIFY(vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, deviceExtensions.data()));

	for (u32 i = 0; i < _extensionCount; ++i)
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

} // namespace yae
