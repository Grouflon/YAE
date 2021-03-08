#include "VulkanWrapper.h"

#include <set>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <00-Macro/Assert.h>
#include <00-Type/IntTypes.h>
#include <00-Type/TypeTools.h>
#include <02-Log/Log.h>
#include <03-Resource/FileResource.h>
#include <03-Resource/ResourceManager.h>

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

namespace yae {

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};
static const std::vector<Vertex> s_vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.f, 0.f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.f, 0.f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.f, 1.f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.f, 1.f}}
};
static const std::vector<uint16_t> s_indices = {
	0, 1, 2, 2, 3, 0
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

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

const u32 INVALID_MEMORY_TYPE = ~0u;
static u32 FindMemoryType(VkPhysicalDevice _physicalDevice, u32 _typeFilter, VkMemoryPropertyFlags _properties)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((_typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
		{
			return i;
		}
	}

	return INVALID_MEMORY_TYPE;
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

bool VulkanWrapper::init(GLFWwindow* _window, bool _validationLayersEnabled)
{
	m_window = _window;
	m_validationLayersEnabled = _validationLayersEnabled;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, &FramebufferResizeCallback);

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
	const u32 deviceExtensionCount = u32(countof(deviceExtensions));
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

		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
		YAE_VERBOSEF_CAT("vulkan", "Picked physical device \"%s\"", m_physicalDeviceProperties.deviceName);
	}
	YAE_ASSERT(m_physicalDevice != VK_NULL_HANDLE);
	QueueFamilyIndices queueIndices = FindQueueFamilies(m_physicalDevice, m_surface);

	// Create Logical Device
	{
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
		YAE_VERBOSE_CAT("vulkan", "Created Device");
	}
	YAE_ASSERT(m_device != VK_NULL_HANDLE);

	// Get Queues
	vkGetDeviceQueue(m_device, queueIndices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, queueIndices.presentFamily, 0, &m_presentQueue);

	// Create Command Pools
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndices.graphicsFamily;
		poolInfo.flags = 0; // Optional

		VK_VERIFY(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));
		YAE_VERBOSE_CAT("vulkan", "Created Command Pool");
	}

	// Create Texture Image
	{
		int textureWidth, textureHeight, textureChannels;
		stbi_uc* pixels = stbi_load("./data/textures/texture.jpg", &textureWidth, &textureHeight, &textureChannels, STBI_rgb_alpha);
		YAE_ASSERT(pixels);

		VkDeviceSize imageSize = textureWidth * textureHeight * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		_createBuffer(
			imageSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer, stagingBufferMemory
		);

		void* data;
		vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_device, stagingBufferMemory);

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

		m_textureImageView = _createImageView(m_textureImage, VK_FORMAT_R8G8B8A8_SRGB);
		YAE_VERBOSE_CAT("vulkan", "Created Texture Image");
	}

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

		VK_VERIFY(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler));
		YAE_VERBOSE_CAT("vulkan", "Created Texture Sampler");
	}

	// Create Vertex Buffer
	{
		VkDeviceSize bufferSize = sizeof(s_vertices[0]) * s_vertices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		{
			void* data;
			VK_VERIFY(vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data));
			memcpy(data, s_vertices.data(), bufferSize);
			vkUnmapMemory(m_device, stagingBufferMemory);
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
		VkDeviceSize bufferSize = sizeof(s_indices[0]) * s_indices.size();
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory
		);

		{
			void* data;
			VK_VERIFY(vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data));
			memcpy(data, s_indices.data(), bufferSize);
			vkUnmapMemory(m_device, stagingBufferMemory);
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
		layoutInfo.bindingCount = uint32_t(countof(bindings));
		layoutInfo.pBindings = bindings;
		VK_VERIFY(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout));
		YAE_VERBOSE_CAT("vulkan", "Created Descriptors Set Layout");
	}

	// Create Swap Chain
	_createSwapChain();

	return true;
}

void VulkanWrapper::draw()
{
	VK_VERIFY(vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX));
	VK_VERIFY(vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]));

	uint32_t imageIndex;
	{
		VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			_recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS || result == VK_SUBOPTIMAL_KHR)
		{
			YAE_ASSERT(false);
		}
	}

	if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
	{
		VK_VERIFY(vkWaitForFences(m_device, 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX))
	}
	m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

	_updateUniformBuffer(imageIndex);

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = uint32_t(countof(waitSemaphores));
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = uint32_t(countof(signalSemaphores));
	submitInfo.pSignalSemaphores = signalSemaphores;
	VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

	VkSwapchainKHR swapChains[] = { m_swapChain };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = uint32_t(countof(signalSemaphores));
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = uint32_t(countof(swapChains));
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	{
		VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
		{
			m_framebufferResized = false;
			_recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			YAE_ASSERT(false);
		}
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanWrapper::shutdown()
{
	VK_VERIFY(vkDeviceWaitIdle(m_device));

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
	_destroyImage(m_textureImage, m_textureImageMemory);
	YAE_VERBOSE_CAT("vulkan", "Destroyed Texture Image");

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	m_commandPool = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Command Pool");

	m_presentQueue = VK_NULL_HANDLE;
	m_graphicsQueue = VK_NULL_HANDLE;

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

void VulkanWrapper::FramebufferResizeCallback(GLFWwindow* _window, int _width, int _height)
{
	VulkanWrapper* vulkanWrapper = reinterpret_cast<VulkanWrapper*>(glfwGetWindowUserPointer(_window));
	vulkanWrapper->m_framebufferResized = true;
}

void VulkanWrapper::_createSwapChain()
{
	QueueFamilyIndices queueIndices = FindQueueFamilies(m_physicalDevice, m_surface);

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

		m_swapChainImageViews.resize(imageCount);
		for (size_t i = 0; i < imageCount; ++i)
		{
			m_swapChainImageViews[i] = _createImageView(m_swapChainImages[i], m_swapChainImageFormat);
		}
		YAE_VERBOSE_CAT("vulkan", "Created Swap Chain Images");
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

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VK_VERIFY(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
		YAE_VERBOSE_CAT("vulkan", "Created Render Pass");
	}

	// Create Pipeline
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

		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescriptions[3];
		attributeDescriptions[0] = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
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
		vertexInputInfo.vertexAttributeDescriptionCount = uint32_t(countof(attributeDescriptions));
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


	// Create Uniform Buffers
	{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(m_swapChainImages.size());
	m_uniformBuffersMemory.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		_createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_uniformBuffers[i],
			m_uniformBuffersMemory[i]
		);
	}
	YAE_VERBOSE_CAT("vulkan", "Created Uniform Buffers");
	}

	// Create Descriptor Pools
	{
		uint32_t descriptorSetCount = uint32_t(m_swapChainImages.size());

		VkDescriptorPoolSize poolSizes[2] = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = descriptorSetCount;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = descriptorSetCount;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = uint32_t(countof(poolSizes));
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = descriptorSetCount;

		VK_VERIFY(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool));

		std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, m_descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorPool;
		allocInfo.descriptorSetCount = descriptorSetCount;
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(descriptorSetCount);
		VK_VERIFY(vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()));

		for (uint32_t i = 0; i < descriptorSetCount; ++i)
		{
			VkWriteDescriptorSet descriptorWrites[2] = {};

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = m_descriptorSets[i];
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
			descriptorWrites[1].dstSet = m_descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = nullptr;
			descriptorWrites[1].pImageInfo = &imageInfo;
			descriptorWrites[1].pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(m_device, uint32_t(countof(descriptorWrites)), descriptorWrites, 0, nullptr);
		}

		YAE_VERBOSE_CAT("vulkan", "Created Descriptor Sets");
	}

	// Create Frame Buffers
	{
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {
				m_swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = m_swapChainExtent.width;
			framebufferInfo.height = m_swapChainExtent.height;
			framebufferInfo.layers = 1;

			VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]));
		}
		YAE_VERBOSE_CAT("vulkan", "Created Frame Buffers");
	}

	// Allocate command buffers
	{
		m_commandBuffers.resize(m_swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = uint32_t(m_commandBuffers.size());

		VK_VERIFY(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()));
		YAE_VERBOSE_CAT("vulkan", "Allocated Command Buffers");
	}

	// Start Command Buffers
	{
		for (size_t i = 0; i < m_commandBuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional

			VK_VERIFY(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo));

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_renderPass;
			renderPassInfo.framebuffer = m_swapChainFramebuffers[i];

			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChainExtent;

			VkClearValue clearColor = { 0.f, 0.f, 0.f, 1.f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

			VkBuffer vertexBuffers[] = { m_vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

			vkCmdDrawIndexed(m_commandBuffers[i], uint32_t(s_indices.size()), 1, 0, 0, 0);
			vkCmdEndRenderPass(m_commandBuffers[i]);

			VK_VERIFY(vkEndCommandBuffer(m_commandBuffers[i]));
		}

		YAE_VERBOSE_CAT("vulkan", "Filled command buffers");
	}

	m_imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);
}

void VulkanWrapper::_destroySwapChain()
{
	m_imagesInFlight.clear();

	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		_destroyBuffer(m_uniformBuffers[i], m_uniformBuffersMemory[i]);
	}
	m_uniformBuffers.clear();
	m_uniformBuffersMemory.clear();
	YAE_VERBOSE_CAT("vulkan", "Destroyed Uniform Buffers");

	vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
	m_commandBuffers.clear();
	YAE_VERBOSE_CAT("vulkan", "Freed Command Buffers");

	for (VkFramebuffer framebuffer : m_swapChainFramebuffers) {
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}
	m_swapChainFramebuffers.clear();
	YAE_VERBOSE_CAT("vulkan", "Destroyed Frame Buffers");

	vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
	m_descriptorSets.clear();
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

	for (VkImageView imageView : m_swapChainImageViews) {
		vkDestroyImageView(m_device, imageView, nullptr);
	}
	m_swapChainImages.clear();
	YAE_VERBOSE_CAT("vulkan", "Destroyed Swap Chain Images");

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_swapChain = VK_NULL_HANDLE;
	YAE_VERBOSE_CAT("vulkan", "Destroyed Swap Chain");
}

void VulkanWrapper::_recreateSwapChain()
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

void VulkanWrapper::_createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _outBuffer, VkDeviceMemory& _outBufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VK_VERIFY(vkCreateBuffer(m_device, &bufferInfo, nullptr, &_outBuffer));

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(m_device, _outBuffer, &memoryRequirements);
	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	uint32_t memoryType = FindMemoryType(m_physicalDevice, memoryRequirements.memoryTypeBits, _properties);
	YAE_ASSERT(memoryType != INVALID_MEMORY_TYPE);
	allocateInfo.memoryTypeIndex = memoryType;
	VK_VERIFY(vkAllocateMemory(m_device, &allocateInfo, nullptr, &_outBufferMemory));

	VK_VERIFY(vkBindBufferMemory(m_device, _outBuffer, _outBufferMemory, 0));
}

void VulkanWrapper::_destroyBuffer(VkBuffer& _inOutBuffer, VkDeviceMemory& _inOutBufferMemory)
{
	vkDestroyBuffer(m_device, _inOutBuffer, nullptr);
	_inOutBuffer = VK_NULL_HANDLE;
	vkFreeMemory(m_device, _inOutBufferMemory, nullptr);
	_inOutBufferMemory = VK_NULL_HANDLE;
}

void VulkanWrapper::_copyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();
	
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	_endSingleTimeCommands(commandBuffer);
}

void VulkanWrapper::_createImage(uint32_t _width, uint32_t _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VkDeviceMemory& _outImageMemory)
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
	VK_VERIFY(vkCreateImage(m_device, &imageInfo, nullptr, &_outImage));

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_device, _outImage, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memoryRequirements.memoryTypeBits, _properties);
	VK_VERIFY(vkAllocateMemory(m_device, &allocateInfo, nullptr, &_outImageMemory));

	vkBindImageMemory(m_device, _outImage, _outImageMemory, 0);
}

void VulkanWrapper::_destroyImage(VkImage& _inOutImage, VkDeviceMemory& _inOutImageMemory)
{
	vkDestroyImage(m_device, _inOutImage, nullptr);
	_inOutImage = VK_NULL_HANDLE;
	vkFreeMemory(m_device, _inOutImageMemory, nullptr);
	_inOutImageMemory = VK_NULL_HANDLE;
}

void VulkanWrapper::_transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout)
{
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = _oldLayout;
	barrier.newLayout = _newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = _image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

void VulkanWrapper::_copyBufferToImage(VkBuffer _buffer, VkImage _image, uint32_t _width, uint32_t _height)
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

VkImageView VulkanWrapper::_createImageView(VkImage _image, VkFormat _format)
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
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	VK_VERIFY(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView));
	return imageView;
}

void VulkanWrapper::_updateUniformBuffer(uint32_t _imageIndex)
{
	static auto s_startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - s_startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.f), time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
	ubo.view = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	ubo.proj = glm::perspective(glm::radians(45.f), m_swapChainExtent.width / float(m_swapChainExtent.height), .1f, 10.f);
	ubo.proj[1][1] *= -1;

	void* data;
	VK_VERIFY(vkMapMemory(m_device, m_uniformBuffersMemory[_imageIndex], 0, sizeof(ubo), 0, &data));
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_device, m_uniformBuffersMemory[_imageIndex]);
}

VkCommandBuffer VulkanWrapper::_beginSingleTimeCommands()
{
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

void VulkanWrapper::_endSingleTimeCommands(VkCommandBuffer _commandBuffer)
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
