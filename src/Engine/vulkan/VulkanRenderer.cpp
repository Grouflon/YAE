#include "VulkanRenderer.h"

#include <log.h>
#include <resources/FileResource.h>
#include <profiling.h>

#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <vulkan/imgui_impl_vulkan.h>

#include <set>
#include <vector>

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

namespace yae {

const char* MODEL_PATH = "./data/models/viking_room.obj";
const char* TEXTURE_PATH = "./data/textures/viking_room.png";

const u32 INVALID_QUEUE = ~0u;

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
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

static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	QueueFamilyIndices queueFamilyIndices;

	u32 queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilyProperties.data());

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

const u32 INVALID_MEMORY_TYPE = ~0u;
static u32 FindMemoryType(VkPhysicalDevice _physicalDevice, u32 _typeFilter, VkMemoryPropertyFlags _properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

	for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((_typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
		{
			return i;
		}
	}

	return INVALID_MEMORY_TYPE;
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;

	bool isValid() const
	{
		return !formats.empty() && !presentModes.empty();
	}
};
static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice _device, VkSurfaceKHR _surface)
{
	SwapChainSupportDetails details;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, _surface, &details.capabilities));

	u32 formatCount;
	VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &formatCount, nullptr));
	details.formats.resize(formatCount);
	VK_VERIFY(vkGetPhysicalDeviceSurfaceFormatsKHR(_device, _surface, &formatCount, details.formats.data()));

	u32 presentModeCount;
	VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &presentModeCount, nullptr));
	details.presentModes.resize(presentModeCount);
	VK_VERIFY(vkGetPhysicalDeviceSurfacePresentModesKHR(_device, _surface, &presentModeCount, details.presentModes.data()));

	return details;
}

bool ChooseSwapSurfaceFormat(const VkSurfaceFormatKHR* _availableFormats, size_t _availableFormatCount, VkSurfaceFormatKHR* _outSurfaceFormat)
{
	for (size_t i = 0; i < _availableFormatCount; ++i)
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

VkPresentModeKHR ChooseSwapPresentMode(const VkPresentModeKHR* _availablePresentModes, size_t _availablePresentModeCount)
{
	for (size_t i = 0; i < _availablePresentModeCount; ++i)
	{
		//if (_availablePresentModes[i] == VK_PRESENT_MODE_FIFO_KHR) // vsync
		if (_availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) // no vsync
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

bool VulkanRenderer::init(GLFWwindow* _window, bool _validationLayersEnabled)
{
	m_window = _window;
	m_validationLayersEnabled = _validationLayersEnabled;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, &FramebufferResizeCallback);

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
			return EXIT_FAILURE;
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
			return EXIT_FAILURE;
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

		auto GetDeviceScore = [deviceExtensions, deviceExtensionCount](VkPhysicalDevice _device, VkSurfaceKHR _surface) -> u32
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
			return EXIT_FAILURE;
		}

		m_queueIndices = FindQueueFamilies(m_physicalDevice, m_surface);
		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
		YAE_VERBOSEF_CAT("vulkan", "Picked physical device \"%s\"", m_physicalDeviceProperties.deviceName);
	}
	YAE_ASSERT(m_physicalDevice != VK_NULL_HANDLE);

	// Create Logical Device
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Logical Device...");

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

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
		poolInfo.flags = 0;
		VK_VERIFY(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
		YAE_VERBOSE_CAT("vulkan", "Created Command Pool");
	}

	// Create Texture Image
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Texture Image...");
		int textureWidth, textureHeight, textureChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH, &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		YAE_ASSERT(pixels);

		VkDeviceSize imageSize = textureWidth * textureHeight * 4;

		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		_createBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);


		void* data;
		VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vmaUnmapMemory(m_allocator, stagingBufferMemory);

		stbi_image_free(pixels);

		_createImage(
			textureWidth,
			textureHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_textureImage,
			m_textureImageMemory
		);

		_transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		_copyBufferToImage(stagingBuffer, m_textureImage, textureWidth, textureHeight);
		_transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_destroyBuffer(stagingBuffer, stagingBufferMemory);

		m_textureImageView = _createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
		YAE_VERBOSE_CAT("vulkan", "Created Texture Image");
	}

	// Create Texture Sampler
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Texture Sampler...");
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

		VK_VERIFY(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler));
		YAE_VERBOSE_CAT("vulkan", "Created Texture Sampler");
	}

	// Load Model
	{
		YAE_VERBOSE_CAT("vulkan", "Loading Model...");
		m_vertices.clear();
		m_indices.clear();
		bool ret = LoadModel(MODEL_PATH, m_vertices, m_indices);
		YAE_ASSERT(ret);
		YAE_VERBOSE_CAT("vulkan", "Loaded Model");
	}

	// Create Vertex Buffer
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Vertex Buffer...");

		VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		{
			void* data;
			VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
			memcpy(data, m_vertices.data(), bufferSize);
			vmaUnmapMemory(m_allocator, stagingBufferMemory);
		}
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory
		);
		_copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

		_destroyBuffer(stagingBuffer, stagingBufferMemory);
		YAE_VERBOSE_CAT("vulkan", "Created Vertex Buffer");
	}

	// Create Index Buffer
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Index Buffer...");
		VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();
		VkBuffer stagingBuffer;
		VmaAllocation stagingBufferMemory;
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		{
			void* data;
			VK_VERIFY(vmaMapMemory(m_allocator, stagingBufferMemory, &data));
			memcpy(data, m_indices.data(), bufferSize);
			vmaUnmapMemory(m_allocator, stagingBufferMemory);
		}
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_indexBuffer,
			m_indexBufferMemory
		);
		_copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

		_destroyBuffer(stagingBuffer, stagingBufferMemory);
		YAE_VERBOSE_CAT("vulkan", "Created Index Buffer");
	}

	// Create Sync Objects
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Sync Objects...");
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			VK_VERIFY(vkCreateSemaphore(m_device, &createInfo, nullptr, &m_imageAvailableSemaphores[i]));
			VK_VERIFY(vkCreateSemaphore(m_device, &createInfo, nullptr, &m_renderFinishedSemaphores[i]));
			VK_VERIFY(vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]));
		}

		YAE_VERBOSE_CAT("vulkan", "Created Sync Objects");
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

	// Create Swap Chain
	_createSwapChain();

	m_clock.reset();

	return true;
}

void VulkanRenderer::beginFrame()
{
	ImGui_ImplVulkan_NewFrame();

	VK_VERIFY(vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFlightFrame], VK_TRUE, UINT64_MAX));
	VK_VERIFY(vkResetFences(m_device, 1, &m_inFlightFences[m_currentFlightFrame]));

	{
		VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFlightFrame], VK_NULL_HANDLE, &m_currentFrameIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			_recreateSwapChain();
			shutdownImGui();
			initImGui();
			return;
		}
		else if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
		{
			YAE_ASSERT(false);
		}
	}

	if (m_imagesInFlight[m_currentFrameIndex] != VK_NULL_HANDLE)
	{
		VK_VERIFY(vkWaitForFences(m_device, 1, &m_imagesInFlight[m_currentFrameIndex], VK_TRUE, UINT64_MAX))
	}
	m_imagesInFlight[m_currentFrameIndex] = m_inFlightFences[m_currentFlightFrame];
	VulkanFrameObjects& frame = m_frameobjects[m_currentFrameIndex];

	_updateUniformBuffer(m_currentFrameIndex);
	vkResetCommandPool(m_device, frame.commandPool, 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional
	VK_VERIFY(vkBeginCommandBuffer(frame.commandBuffer, &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = frame.frameBuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.f, 0.f, 0.f, 1.f };
	clearValues[1].depthStencil = { 1.f, 0 };

	renderPassInfo.clearValueCount = u32(countof(clearValues));
	renderPassInfo.pClearValues = clearValues;
	vkCmdBeginRenderPass(frame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::drawMesh()
{
	YAE_CAPTURE_FUNCTION();

	VulkanFrameObjects& frame = m_frameobjects[m_currentFrameIndex];

	vkCmdBindPipeline(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(frame.commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(frame.commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(frame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &frame.descriptorSet, 0, nullptr);

	vkCmdDrawIndexed(frame.commandBuffer, u32(m_indices.size()), 1, 0, 0, 0);
}

void VulkanRenderer::drawImGui(ImDrawData* _drawData)
{
	YAE_CAPTURE_FUNCTION();

	VulkanFrameObjects& frame = m_frameobjects[m_currentFrameIndex];

	ImGui_ImplVulkan_RenderDrawData(_drawData, frame.commandBuffer);
}

void VulkanRenderer::endFrame()
{
	YAE_CAPTURE_FUNCTION();

	VulkanFrameObjects& frame = m_frameobjects[m_currentFrameIndex];
	
	vkCmdEndRenderPass(frame.commandBuffer);
	VK_VERIFY(vkEndCommandBuffer(frame.commandBuffer));

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFlightFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFlightFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = u32(countof(waitSemaphores));
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frame.commandBuffer;
	submitInfo.signalSemaphoreCount = u32(countof(signalSemaphores));
	submitInfo.pSignalSemaphores = signalSemaphores;
	VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFlightFrame]));

	VkSwapchainKHR swapChains[] = { m_swapChain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = u32(countof(signalSemaphores));
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = u32(countof(swapChains));
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_currentFrameIndex;

	{
		VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapChain();
			shutdownImGui();
			initImGui();
		}
		else if (result != VK_SUCCESS)
		{
			YAE_ASSERT(false);
		}
	}

	m_currentFlightFrame = (m_currentFlightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	m_currentFrameIndex = ~0;
}

void VulkanRenderer::waitIdle()
{
	YAE_CAPTURE_FUNCTION();
	
	VK_VERIFY(vkDeviceWaitIdle(m_device));
}

void VulkanRenderer::shutdown()
{
	_destroySwapChain();

	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Descriptor Set Layout");

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
	}
	m_inFlightFences.clear();
	m_renderFinishedSemaphores.clear();
	m_imageAvailableSemaphores.clear();
	YAE_VERBOSE_CAT("vulkan", "Destroyed Sync Objects");

	_destroyBuffer(m_indexBuffer, m_indexBufferMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Index Buffer");

	_destroyBuffer(m_vertexBuffer, m_vertexBufferMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Vertex Buffer");

	vkDestroySampler(m_device, m_textureSampler, nullptr);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Texture Sampler");

	vkDestroyImageView(m_device, m_textureImageView, nullptr);
	m_textureImageView = VK_NULL_HANDLE;
	_destroyImage(m_textureImage, m_textureImageMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Texture Image");

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

	glfwSetFramebufferSizeCallback(m_window, nullptr);
	glfwSetWindowUserPointer(m_window, nullptr);
	m_window = nullptr;
}

void VulkanRenderer::initImGui()
{
	YAE_ASSERT(m_window);

	YAE_VERBOSE_CAT("imgui", "Initializing ImGui...");

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance = m_instance;
	initInfo.PhysicalDevice = m_physicalDevice;
	initInfo.Device = m_device;
	initInfo.QueueFamily = m_queueIndices.graphicsFamily;
	initInfo.Queue = m_graphicsQueue;
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = m_descriptorPool;
	initInfo.Subpass = 0;
	initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;          // >= 2
	initInfo.ImageCount = u32(m_frameobjects.size());		// >= MinImageCount
	initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;           // >= VK_SAMPLE_COUNT_1_BIT
	initInfo.Allocator = nullptr;
	initInfo.VmaAllocator = m_allocator;
	initInfo.CheckVkResultFn = [](VkResult _err) { VK_VERIFY(_err); };

	bool ret = ImGui_ImplVulkan_Init(&initInfo, m_renderPass);
	YAE_ASSERT(ret);

	// Upload Fonts
	{
		// Use any command queue
		VkCommandBuffer commandBuffer = _beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		_endSingleTimeCommands(commandBuffer);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	YAE_VERBOSE_CAT("imgui", "Initialized ImGui");
}

void VulkanRenderer::shutdownImGui()
{
	ImGui_ImplVulkan_Shutdown();
	YAE_VERBOSE_CAT("imgui", "Shutdown ImGui");
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

VkFormat VulkanRenderer::FindSupportedFormat(VkPhysicalDevice _physicalDevice, VkFormat* _candidates, size_t _candidateCount, VkImageTiling _tiling, VkFormatFeatureFlags _features)
{
	for (size_t i = 0; i < _candidateCount; ++i)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(_physicalDevice, _candidates[i], &properties);

		if (_tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & _features) == _features)
		{
			return _candidates[i];
		}
		else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & _features) == _features)
		{
			return _candidates[i];
		}
	}

	return VK_FORMAT_UNDEFINED;
}

bool VulkanRenderer::HasStencilComponent(VkFormat _format)
{
	return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanRenderer::FramebufferResizeCallback(GLFWwindow* _window, int _width, int _height)
{
	VulkanRenderer* vulkanWrapper = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(_window));
	vulkanWrapper->m_framebufferResized = true;
}

bool VulkanRenderer::LoadModel(const char* _path, std::vector<Vertex>& _outVertices, std::vector<u32>& _outIndices)
{
	YAE_ASSERT(_outVertices.empty());
	YAE_ASSERT(_outIndices.empty());

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH))
	{
		YAE_ERROR((warn + err).c_str());
		return false;
	}

	std::unordered_map<Vertex, u32> uniqueVertices;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex v{};
			v.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			v.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			v.color = {1.f, 1.f, 1.f};

			auto it = uniqueVertices.find(v);
			if (it == uniqueVertices.end())
			{
				it = uniqueVertices.insert(std::make_pair(v, u32(_outVertices.size()))).first;
				_outVertices.push_back(v);
			}
			_outIndices.push_back(it->second);
		}
	}

	return true;
}

void VulkanRenderer::_createSwapChain()
{
	// Create Swap Chain
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Swap Chain...");

		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);
		VkSurfaceFormatKHR surfaceFormat;
		{
			bool result = ChooseSwapSurfaceFormat(swapChainSupport.formats.data(), swapChainSupport.formats.size(), &surfaceFormat);
			YAE_ASSERT(result);
		}
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes.data(), swapChainSupport.presentModes.size());
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
		u32 queueFamilyIndices[] = { m_queueIndices.graphicsFamily, m_queueIndices.presentFamily };
		if (m_queueIndices.graphicsFamily != m_queueIndices.presentFamily)
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
	u32 frameCount = 0;
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Swap Chain Images...");
		VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &frameCount, nullptr));
		std::vector<VkImage> swapChainImages(frameCount);
		VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &frameCount, swapChainImages.data()));

		m_frameobjects.resize(frameCount);
		for (size_t i = 0; i < frameCount; ++i)
		{
			m_frameobjects[i].swapChainImage = swapChainImages[i];
			m_frameobjects[i].swapChainImageView = _createImageView(swapChainImages[i], m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
		YAE_VERBOSE_CAT("vulkan", "Created Swap Chain Images");
	}

	// Create Depth Resources
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Depth Resources...");

		VkFormat formatCandidates[] =
		{
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT
		};

		m_depthFormat = FindSupportedFormat(
			m_physicalDevice,
			formatCandidates, countof(formatCandidates),
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		);
		YAE_ASSERT(m_depthFormat != VK_FORMAT_UNDEFINED);

		_createImage(m_swapChainExtent.width, m_swapChainExtent.height, m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory);;
		m_depthImageView = _createImageView(m_depthImage, m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		_transitionImageLayout(m_depthImage, m_depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		YAE_VERBOSE_CAT("vulkan", "Depth Resources Created");
	}

	// Create render pass
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Render Pass...");

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

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = m_depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[] =
		{
			colorAttachment,
			depthAttachment
		};

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = u32(countof(attachments));
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_VERIFY(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
		YAE_VERBOSE_CAT("vulkan", "Created Render Pass");
	}

	// Create Pipeline
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Graphics Pipeline...");

		auto createShaderModule = [](VkDevice _device, const void* _code, size_t _codeSize) -> VkShaderModule
		{
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = _codeSize;
			createInfo.pCode = reinterpret_cast<const u32*>(_code);

			VkShaderModule shaderModule;
			VK_VERIFY(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));
			return shaderModule;
		};

		VkShaderModule vertexShaderModule;
		{
			FileResource* file = findOrCreateResource<FileResource>("./data/shaders/vert.spv");
			file->useLoad();
			YAE_ASSERT(file->getError() == Resource::ERROR_NONE);
			vertexShaderModule = createShaderModule(m_device, file->getContent(), file->getContentSize());
			file->releaseUnuse();
		}

		VkShaderModule fragmentShaderModule;
		{
			FileResource* file = findOrCreateResource<FileResource>("./data/shaders/frag.spv");
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

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapChainExtent.width;
		viewport.height = (float)m_swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
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
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
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


	// Create Uniform Buffers
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Uniform Buffers...");

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		for (size_t i = 0; i < frameCount; ++i)
		{
			_createBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_frameobjects[i].uniformBuffer,
				m_frameobjects[i].uniformBufferMemory
			);
		}
		YAE_VERBOSE_CAT("vulkan", "Created Uniform Buffers");
	}

	// Create Descriptor Pools
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Descriptor Sets...");

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

		std::vector<VkDescriptorSetLayout> layouts(frameCount, m_descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = u32(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptorSets(frameCount);
		VK_VERIFY(vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()));

		for (u32 i = 0; i < frameCount; ++i)
		{
			m_frameobjects[i].descriptorSet = descriptorSets[i];

			VkWriteDescriptorSet descriptorWrites[2] = {};

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_frameobjects[i].uniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr;
			descriptorWrites[0].pTexelBufferView = nullptr;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = m_textureImageView;
			imageInfo.sampler = m_textureSampler;
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_device, u32(countof(descriptorWrites)), descriptorWrites, 0, nullptr);
		}

		YAE_VERBOSE_CAT("vulkan", "Created Descriptor Sets");
	}

	// Create Frame Buffers
	{
		YAE_VERBOSE_CAT("vulkan", "Creating Frame Buffers...");
		for (size_t i = 0; i < m_frameobjects.size(); ++i)
		{
			VkImageView attachments[] = {
				m_frameobjects[i].swapChainImageView,
				m_depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = u32(countof(attachments));
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_frameobjects[i].frameBuffer));
		}
		YAE_VERBOSE_CAT("vulkan", "Created Frame Buffers");
	}

	// Create pool and allocate command buffers
	{
		YAE_VERBOSE_CAT("vulkan", "Allocating Command Buffers...");
		for (size_t i = 0; i < frameCount; ++i)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = m_queueIndices.graphicsFamily;
			poolInfo.flags = 0;
			VK_VERIFY(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_frameobjects[i].commandPool));

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = m_frameobjects[i].commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
			VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, &m_frameobjects[i].commandBuffer));
		}
		YAE_VERBOSE_CAT("vulkan", "Allocated Command Buffers");
	}

	m_imagesInFlight.resize(frameCount, VK_NULL_HANDLE);
}

void VulkanRenderer::_destroySwapChain()
{
	m_imagesInFlight.clear();
	u32 frameCount = u32(m_frameobjects.size());

	for (size_t i = 0; i < frameCount; ++i)
	{
		_destroyBuffer(m_frameobjects[i].uniformBuffer, m_frameobjects[i].uniformBufferMemory);
	}
	YAE_VERBOSE_CAT("vulkan", "Destroyed Uniform Buffers");

	for (size_t i = 0; i < frameCount; ++i)
	{
		vkFreeCommandBuffers(m_device, m_frameobjects[i].commandPool, 1, &m_frameobjects[i].commandBuffer);
		vkDestroyCommandPool(m_device, m_frameobjects[i].commandPool, nullptr);
	}
	YAE_VERBOSE_CAT("vulkan", "Freed Command Buffers");

	for (size_t i = 0; i < frameCount; ++i)
	{
		vkDestroyFramebuffer(m_device, m_frameobjects[i].frameBuffer, nullptr);
	}
	YAE_VERBOSE_CAT("vulkan", "Destroyed Frame Buffers");

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	m_descriptorPool = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Descriptor Sets");

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	m_graphicsPipeline = VK_NULL_HANDLE;
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	m_pipelineLayout = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Graphic Pipeline");

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	m_renderPass = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Render Pass");

	vkDestroyImageView(m_device, m_depthImageView, nullptr);
	m_depthImageView = VK_NULL_HANDLE;
	_destroyImage(m_depthImage, m_depthImageMemory);

	for (size_t i = 0; i < frameCount; ++i)
	{
		vkDestroyImageView(m_device, m_frameobjects[i].swapChainImageView, nullptr);
	}
	YAE_VERBOSE_CAT("vulkan", "Destroyed Swap Chain Images");

	m_frameobjects.clear();

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_swapChain = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Swap Chain");
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

	_destroySwapChain();
	_createSwapChain();
}

void VulkanRenderer::_createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _outBuffer, VmaAllocation& _allocation)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.preferredFlags = _properties;
	VK_VERIFY(vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &_outBuffer, &_allocation, nullptr));
}

void VulkanRenderer::_destroyBuffer(VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation)
{
	vmaDestroyBuffer(m_allocator, _inOutBuffer, _inOutAllocation);
	_inOutBuffer = VK_NULL_HANDLE;
	_inOutAllocation = VK_NULL_HANDLE;
}

void VulkanRenderer::_copyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();
	
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	_endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::_createImage(u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VmaAllocation& _outImageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = _width;
	imageInfo.extent.height = _height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = _format;
	imageInfo.tiling = _tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = _usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	VmaAllocationCreateInfo allocCreateInfo{};
	allocCreateInfo.preferredFlags = _properties;
	VmaAllocationInfo allocInfo;
	VK_VERIFY(vmaCreateImage(m_allocator, &imageInfo, &allocCreateInfo, &_outImage, &_outImageMemory, &allocInfo));
}

void VulkanRenderer::_destroyImage(VkImage& _inOutImage, VmaAllocation& _inOutImageMemory)
{
	vmaDestroyImage(m_allocator, _inOutImage, _inOutImageMemory);
	_inOutImage = VK_NULL_HANDLE;
	_inOutImageMemory = VK_NULL_HANDLE;
}

void VulkanRenderer::_transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
{
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

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
	
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
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

	_endSingleTimeCommands(commandBuffer);
}

void VulkanRenderer::_copyBufferToImage(VkBuffer _buffer, VkImage _image, u32 _width, u32 _height)
{
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

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

	_endSingleTimeCommands(commandBuffer);
}

VkImageView VulkanRenderer::_createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = _image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = _format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = _aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView));
	return imageView;
}

void VulkanRenderer::_updateUniformBuffer(u32 _imageIndex)
{
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.f), m_clock.elapsed().asSeconds() * glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f));
	ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	ubo.proj = glm::perspective(glm::radians(45.f), m_swapChainExtent.width / float(m_swapChainExtent.height), .1f, 10.f);
	ubo.proj[1][1] *= -1;

	void* data;
	VK_VERIFY(vmaMapMemory(m_allocator, m_frameobjects[_imageIndex].uniformBufferMemory, &data));
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(m_allocator, m_frameobjects[_imageIndex].uniformBufferMemory);
}

VkCommandBuffer VulkanRenderer::_beginSingleTimeCommands()
{
	VK_VERIFY(vkResetCommandPool(m_device, m_commandPool, 0));

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	return commandBuffer;
}

void VulkanRenderer::_endSingleTimeCommands(VkCommandBuffer _commandBuffer)
{
	VK_VERIFY(vkEndCommandBuffer(_commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;
	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &_commandBuffer);
}

} // namespace yae
