#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <00-Type/IntTypes.h>

struct ImDrawData;

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace yae {

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

extern const u32 INVALID_QUEUE;
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

class VulkanWrapper
{
public:
	bool init(GLFWwindow* _window, bool _validationLayersEnabled = false);
	void beginDraw();
	void drawMesh();
	void drawImGui(ImDrawData* _drawData);
	void endDraw();
	void waitIdle();
	void shutdown();

	void initImGui();
	void shutdownImGui();

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice _physicalDevice, const char* const* _extensionsList, size_t _extensionCount);
	static VkFormat FindSupportedFormat(VkPhysicalDevice _physicalDevice, VkFormat* _candidates, size_t _candidateCount, VkImageTiling _tiling, VkFormatFeatureFlags _features);
	static bool HasStencilComponent(VkFormat _format);

	static void FramebufferResizeCallback(GLFWwindow* _window, int _width, int _height);
	static bool LoadModel(const char* _path, std::vector<Vertex>& _outVertices, std::vector<u32>& _outIndices);

private:
	void _createSwapChain();
	void _destroySwapChain();
	void _recreateSwapChain();

	void _createBuffer(VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties, VkBuffer& _outBuffer, VkDeviceMemory& _outBufferMemory);
	void _destroyBuffer(VkBuffer& _inOutBuffer, VkDeviceMemory& _inOutBufferMemory);
	void _copyBuffer(VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);

	void _createImage(u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VkDeviceMemory& _outImageMemory);
	void _destroyImage(VkImage& _inOutImage, VkDeviceMemory& _inOutImageMemory);
	void _transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);
	void _copyBufferToImage(VkBuffer _buffer, VkImage _image, u32 _width, u32 _height);

	VkImageView _createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags);

	void _updateUniformBuffer(u32 _imageIndex);

	VkCommandBuffer _beginSingleTimeCommands();
	void _endSingleTimeCommands(VkCommandBuffer _commandBuffer);


	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

	GLFWwindow* m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_physicalDeviceProperties;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	QueueFamilyIndices m_queueIndices;

	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;

	struct VulkanFrameObjects
	{
		VkImage swapChainImage;
		VkImageView swapChainImageView;
		VkBuffer uniformBuffer;
		VkDeviceMemory uniformBufferMemory;
		VkDescriptorSet descriptorSet;
		VkFramebuffer frameBuffer;
		VkCommandBuffer commandBuffer;
	};
	std::vector<VulkanFrameObjects> m_frameobjects;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

	std::vector<Vertex> m_vertices;
	std::vector<u32> m_indices;
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

	VkDescriptorPool m_descriptorPool;

	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	VkFormat m_depthFormat;
	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;
	VkImageView m_textureImageView;
	VkSampler m_textureSampler;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;
	std::vector<VkFence> m_imagesInFlight;
	size_t m_currentFlightFrame = 0;
	u32 m_currentFrameIndex = ~0;

	bool m_validationLayersEnabled = false;
	bool m_framebufferResized = false;
};

}

#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<yae::Vertex> {
		size_t operator()(yae::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}