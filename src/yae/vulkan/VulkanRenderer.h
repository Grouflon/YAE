#pragma once

#include <yae/types.h>
#include <yae/time.h>
#include <yae/render_types.h>
#include <yae/math_types.h>
#include <yae/containers/Array.h>

typedef struct GLFWwindow GLFWwindow;
struct ImDrawData;

namespace yae {

struct im3d_Instance;
struct im3d_FrameData;
class TextureResource;
class MeshResource;

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

class YAELIB_API VulkanRenderer
{
public:
	bool init(GLFWwindow* _window, bool _validationLayersEnabled = false);
	void beginFrame();
	void endFrame();
	void waitIdle();
	void shutdown();

	void initImGui();
	void shutdownImGui();

	void initIm3d();
	void reloadIm3dShaders();
	void shutdownIm3d();

	void drawMesh();
	void drawImGui(ImDrawData* _drawData);
	void im3dNewFrame(const im3d_FrameData& _frameData);
	void im3dEndFrame();

	void notifyFrameBufferResized(int _width, int _height);

	bool createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle);
	void destroyTexture(TextureHandle& _inTextureHandle);

	bool createMesh(Vertex* _vertices, u32 _verticesCount, u32* _indices, u32 _indicesCount, MeshHandle& _outMeshHandle);
	void destroyMesh(MeshHandle& _inMeshHandle);

	bool createShader(const void* _code, size_t _codeSize, ShaderHandle& _outShaderHandle);
	void destroyShader(ShaderHandle& _shaderHandle);

	void setViewProjectionMatrix(const Matrix4& _view, const Matrix4& _proj);
	Vector2 getFrameBufferSize() const;

	static bool CheckDeviceExtensionSupport(VkPhysicalDevice _physicalDevice, const char* const* _extensionsList, size_t _extensionCount);
	static VkFormat FindSupportedFormat(VkPhysicalDevice _physicalDevice, VkFormat* _candidates, size_t _candidateCount, VkImageTiling _tiling, VkFormatFeatureFlags _features);
	static bool HasStencilComponent(VkFormat _format);

private:
	void _createSwapChain();
	void _destroySwapChain();
	void _recreateSwapChain();

	void _transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);
	void _copyBufferToImage(VkBuffer _buffer, VkImage _image, u32 _width, u32 _height);

	VkImageView _createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags);

	void _updateUniformBuffer(u32 _imageIndex);

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

	GLFWwindow* m_window = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
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
		VmaAllocation uniformBufferMemory;
		VkDescriptorSet descriptorSet;
		VkFramebuffer frameBuffer;
		VkCommandPool commandPool;
		VkCommandBuffer commandBuffer;
	};
	DataArray<VulkanFrameObjects> m_frameobjects;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

	VkDescriptorPool m_descriptorPool;

	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	VkFormat m_depthFormat;
	VkImage m_depthImage;
	VmaAllocation m_depthImageMemory;
	VkImageView m_depthImageView;

	TextureResource* m_texture1 = nullptr;
	TextureResource* m_texture2 = nullptr;
	VkSampler m_textureSampler;

	MeshResource* m_mesh = nullptr;

	DataArray<VkSemaphore> m_imageAvailableSemaphores;
	DataArray<VkSemaphore> m_renderFinishedSemaphores;
	DataArray<VkFence> m_inFlightFences;
	DataArray<VkFence> m_imagesInFlight;
	u32 m_currentFlightFrame = 0;
	u32 m_currentFrameIndex = ~0;

	bool m_validationLayersEnabled = false;
	bool m_framebufferResized = false;

	Clock m_clock;

	Matrix4 m_viewMatrix = Matrix4::IDENTITY;
	Matrix4 m_projMatrix = Matrix4::IDENTITY;

	im3d_Instance* m_im3dInstance = nullptr;
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