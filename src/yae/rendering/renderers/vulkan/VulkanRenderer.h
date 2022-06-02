#pragma once

#include <yae/types.h>

#include <yae/rendering/render_types.h>
#include <yae/math_types.h>
#include <yae/containers/Array.h>
#include <yae/rendering/Renderer.h>

typedef struct GLFWwindow GLFWwindow;
struct ImDrawData;

namespace yae {

struct im3d_Instance;
struct im3d_FrameData;
class VulkanSwapChain;
class TextureResource;

class YAELIB_API VulkanRenderer : public Renderer
{
public:
	virtual bool init(GLFWwindow* _window) override;
	virtual void waitIdle() override;
	virtual void shutdown() override;

	virtual RendererType getType() const override { return RendererType::Vulkan; }

	virtual void hintWindow() override;

	virtual FrameHandle beginFrame() override;
  	virtual void endFrame() override;

	virtual Vector2 getFrameBufferSize() const override;
	virtual void notifyFrameBufferResized(int _width, int _height) override;

	virtual bool createTexture(void* _data, int _width, int _height, int _channels, TextureHandle& _outTextureHandle) override;
	virtual void destroyTexture(TextureHandle& _inTextureHandle) override;

	virtual bool createMesh(Vertex* _vertices, u32 _verticesCount, u32* _indices, u32 _indicesCount, MeshHandle& _outMeshHandle) override;
	virtual void destroyMesh(MeshHandle& _inMeshHandle) override;

	virtual bool createShader(const void* _code, size_t _codeSize, ShaderHandle& _outShaderHandle) override;
	virtual void destroyShader(ShaderHandle& _shaderHandle) override;

	virtual void drawCommands(FrameHandle _frameHandle) override;

	virtual void drawMesh(const Matrix4& _transform, const MeshHandle& _meshHandle) override;

  	void beginSwapChainRenderPass(VkCommandBuffer _commandBuffer);
  	void endSwapChainRenderPass(VkCommandBuffer _commandBuffer);

	void initIm3d();
	void reloadIm3dShaders();
	void shutdownIm3d();

	void drawIm3d(VkCommandBuffer _commandBuffer);


	static bool CheckDeviceExtensionSupport(VkPhysicalDevice _physicalDevice, const char* const* _extensionsList, size_t _extensionCount);
	static bool HasStencilComponent(VkFormat _format);

//private:
	void _recreateSwapChain();
	void _createCommandBuffers();
	void _destroyCommandBuffers();
	void _createDescriptorSet();
	void _destroyDescriptorSet();
	void _createPipeline();
	void _destroyPipeline();

	void _transitionImageLayout(VkImage _image, VkFormat _format, VkImageLayout _oldLayout, VkImageLayout _newLayout);
	void _copyBufferToImage(VkBuffer _buffer, VkImage _image, u32 _width, u32 _height);

	VkImageView _createImageView(VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags);

	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;

	GLFWwindow* m_window = nullptr;
	VulkanSwapChain* m_swapChain = nullptr;
	VkInstance m_instance = VK_NULL_HANDLE;
	VmaAllocator m_allocator = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties m_physicalDeviceProperties;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	QueueFamilyIndices m_queueIndices;

	struct FrameInfo
	{
		VkCommandBuffer commandBuffer;
		VkDescriptorSet descriptorSet;
		VkBuffer uniformBuffer;
		VmaAllocation uniformBufferMemory;
	};
	DataArray<FrameInfo> m_frameInfos;

	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

	u32 m_currentFlightImageIndex = ~0;
	u32 m_currentFrameIndex = 0;

	bool m_validationLayersEnabled = false;
	bool m_framebufferResized = false;

	struct DrawCommand
	{
		Matrix4 transform;
		MeshHandle mesh;
	};
	DataArray<DrawCommand> m_drawCommands;

	im3d_Instance* m_im3dInstance = nullptr;

	TextureResource* m_texture = nullptr;
};

}
