#pragma once

#include <yae/types.h>
#include <yae/render_types.h>

namespace yae {

class VulkanSwapChain
{
public:
	static const u8 MAX_FRAMES_IN_FLIGHT = 2;

	void init(VkPhysicalDevice _physicalDevice, VkDevice _device, VmaAllocator _allocator, VkSurfaceKHR _surface, VkExtent2D _windowExtent);
	void reinit(VkPhysicalDevice _physicalDevice, VkDevice _device, VmaAllocator _allocator, VkSurfaceKHR _surface, VkExtent2D _windowExtent);
	void shutdown();

	VkExtent2D getExtent() const { return m_extent; }
	VkRenderPass getRenderPass() const { return m_renderPass; }
	VkFramebuffer getFrameBuffer(u32 _imageIndex) const { YAE_ASSERT(_imageIndex < m_images.size()); return m_images[_imageIndex].frameBuffer; }

	VkResult acquireNextImage(u32* _imageIndex);
	VkResult submitCommandBuffers(const VkCommandBuffer* _buffers, u32 _bufferCount, u32* _imageIndex);

//private:
	// Helper functions
	bool _chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* _availableFormats, size_t _availableFormatCount, VkSurfaceFormatKHR* _outSurfaceFormat);
	VkPresentModeKHR _chooseSwapPresentMode(const VkPresentModeKHR* _availablePresentModes, size_t _availablePresentModeCount);
	VkExtent2D _chooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities, VkExtent2D _windowExtent);

	// Init functions
	void _createSwapChain(VkSwapchainKHR _previousSwapChain = VK_NULL_HANDLE);
	void _createSwapChainImages();
	void _createDepthImages();
	void _createRenderPass();
	void _createFrameBuffers();
	void _createSyncObjects();

	void _destroySwapChain();
	void _destroySwapChainImages();
	void _destroyDepthImages();
	void _destroyRenderPass();
	void _destroyFrameBuffers();
	void _destroySyncObjects();
	
	// Helper functions
	/*VkSurfaceFormatKHR chooseSwapSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(
	const std::vector<VkPresentModeKHR> &availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);*/

	VkFormat m_imageFormat;
	VkFormat m_depthFormat;
	VkExtent2D m_extent;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VmaAllocator m_allocator;
	VkSurfaceKHR m_surface;
	VkExtent2D m_windowExtent;

	VkSwapchainKHR m_swapChain;
	VkRenderPass m_renderPass;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	struct VulkanSwapChainImage
	{
		VkImage image;
		VkImageView imageView;
		VkImage depthImage;
		VmaAllocation depthImageMemory;
		VkImageView depthImageView;
		VkFramebuffer frameBuffer;
		VkFence imageInFlight;
	};
	DataArray<VulkanSwapChainImage> m_images;

	struct SyncObjects
	{
		VkSemaphore imageAvailableSemaphore; 
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;	
	};
	SyncObjects m_syncObjects[MAX_FRAMES_IN_FLIGHT];

	u32 m_currentFrameIndex = 0;
};

} // namespace yae
