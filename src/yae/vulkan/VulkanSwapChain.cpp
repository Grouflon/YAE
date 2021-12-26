#include "VulkanSwapChain.h"

#include <yae/vulkan/vulkan.h>
#include <yae/math.h>

namespace yae {

void VulkanSwapChain::init(VkPhysicalDevice _physicalDevice, VkDevice _device, VmaAllocator _allocator, VkSurfaceKHR _surface, VkExtent2D _windowExtent)
{
	YAE_CAPTURE_FUNCTION();

	m_physicalDevice = _physicalDevice;
	m_device = _device;
	m_allocator = _allocator;
	m_surface = _surface;
	m_windowExtent = _windowExtent;

	_createSwapChain();
	_createSwapChainImages();
	_createDepthImages();
	_createRenderPass();
	_createFrameBuffers();
	_createSyncObjects();
}

void VulkanSwapChain::reinit(VkPhysicalDevice _physicalDevice, VkDevice _device, VmaAllocator _allocator, VkSurfaceKHR _surface, VkExtent2D _windowExtent)
{
	YAE_CAPTURE_FUNCTION();

	YAE_ASSERT(m_swapChain != VK_NULL_HANDLE);

	m_physicalDevice = _physicalDevice;
	m_device = _device;
	m_allocator = _allocator;
	m_surface = _surface;
	m_windowExtent = _windowExtent;

	_destroySyncObjects();
	_destroyFrameBuffers();
	_destroyRenderPass();
	_destroyDepthImages();
	_destroySwapChainImages();
	m_images.clear();

	VkSwapchainKHR previousSwapChain = m_swapChain;
	_createSwapChain(previousSwapChain);
	vkDestroySwapchainKHR(m_device, previousSwapChain, nullptr);

	_createSwapChainImages();
	_createDepthImages();
	_createRenderPass();
	_createFrameBuffers();
	_createSyncObjects();
}


void VulkanSwapChain::shutdown()
{
	YAE_CAPTURE_FUNCTION();

	_destroySyncObjects();
	_destroyFrameBuffers();
	_destroyRenderPass();
	_destroyDepthImages();
	_destroySwapChainImages();
	m_images.clear();
	_destroySwapChain();

	YAE_VERBOSE_CAT("vulkan", "Destroyed SwapChain");
}

VkResult VulkanSwapChain::acquireNextImage(u32* _imageIndex)
{
	VK_VERIFY(vkWaitForFences(
		m_device,
		1,
		&m_syncObjects[m_currentFrameIndex].inFlightFence,
		VK_TRUE,
		UINT64_MAX
	));

	VkResult result = vkAcquireNextImageKHR(
		m_device,
		m_swapChain,
		UINT64_MAX,
		m_syncObjects[m_currentFrameIndex].imageAvailableSemaphore,  // must be a not signaled semaphore
		VK_NULL_HANDLE,
		_imageIndex
	);

	return result;
}

VkResult VulkanSwapChain::submitCommandBuffers(const VkCommandBuffer* _buffers, u32 _bufferCount, u32* _imageIndex)
{
	VulkanSwapChainImage& image = m_images[*_imageIndex];
	SyncObjects& syncObjects = m_syncObjects[m_currentFrameIndex];

	if (image.imageInFlight != VK_NULL_HANDLE)
	{
		vkWaitForFences(m_device, 1, &image.imageInFlight, VK_TRUE, UINT64_MAX);
	}
	image.imageInFlight = syncObjects.inFlightFence;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { syncObjects.imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = _bufferCount;
	submitInfo.pCommandBuffers = _buffers;

	VkSemaphore signalSemaphores[] = { syncObjects.renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_VERIFY(vkResetFences(m_device, 1, &image.imageInFlight));
	VK_VERIFY(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, image.imageInFlight));

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = _imageIndex;

	VkResult result = vkQueuePresentKHR(m_presentQueue, &presentInfo);

	m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

bool VulkanSwapChain::_chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* _availableFormats, size_t _availableFormatCount, VkSurfaceFormatKHR* _outSurfaceFormat)
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

VkPresentModeKHR VulkanSwapChain::_chooseSwapPresentMode(const VkPresentModeKHR* _availablePresentModes, size_t _availablePresentModeCount)
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

VkExtent2D VulkanSwapChain::_chooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities, VkExtent2D _windowExtent)
{
	if (_capabilities.currentExtent.width != UINT32_MAX)
	{
		return _capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = _windowExtent;
		actualExtent.width = max(_capabilities.minImageExtent.width, min(_capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = max(_capabilities.minImageExtent.height, min(_capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void VulkanSwapChain::_createSwapChain(VkSwapchainKHR _previousSwapChain)
{
	YAE_CAPTURE_FUNCTION();

	SwapChainSupportDetails swapChainSupport = vulkan::querySwapChainSupport(m_physicalDevice, m_surface);
	VkSurfaceFormatKHR surfaceFormat;
	YAE_VERIFY(_chooseSwapSurfaceFormat(swapChainSupport.formats.data(), swapChainSupport.formats.size(), &surfaceFormat));
	VkPresentModeKHR presentMode = _chooseSwapPresentMode(swapChainSupport.presentModes.data(), swapChainSupport.presentModes.size());
	VkExtent2D extent = _chooseSwapExtent(swapChainSupport.capabilities, m_windowExtent);

	m_imageFormat = surfaceFormat.format;
	m_extent = extent;

	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = vulkan::findQueueFamilies(m_physicalDevice, m_surface);
	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

	u32 queueFamilyIndices[] = { indices.graphicsFamily, indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;      // Optional
		createInfo.pQueueFamilyIndices = nullptr;  // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = _previousSwapChain;

	VK_VERIFY(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain));
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Created SwapChain");
}

void VulkanSwapChain::_createSwapChainImages()
{
	YAE_CAPTURE_FUNCTION();

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	u32 frameCount;
	VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &frameCount, nullptr));
	DataArray<VkImage> images(&scratchAllocator());
	images.resize(frameCount);
	VK_VERIFY(vkGetSwapchainImagesKHR(m_device, m_swapChain, &frameCount, images.data()));
	m_images.resize(frameCount);
	for (u32 i = 0; i < frameCount; ++i)
	{
		m_images[i].image = images[i];
		m_images[i].imageView = vulkan::createImageView(m_device, images[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		m_images[i].imageInFlight = VK_NULL_HANDLE;
	}
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Created Images");
}

void VulkanSwapChain::_createDepthImages()
{
	YAE_CAPTURE_FUNCTION();

	VkFormat formatCandidates[] =
	{
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};

	m_depthFormat = vulkan::findSupportedFormat(
		m_physicalDevice,
		formatCandidates, countof(formatCandidates),
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
	YAE_ASSERT(m_depthFormat != VK_FORMAT_UNDEFINED);

	for (u32 i = 0; i < m_images.size(); ++i)
	{
		vulkan::createImage(m_allocator, m_extent.width, m_extent.height, m_depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_images[i].depthImage, m_images[i].depthImageMemory);
		m_images[i].depthImageView = vulkan::createImageView(m_device, m_images[i].depthImage, m_depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		//_transitionImageLayout(m_images[i].depthImage, m_depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	}
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Depth Resources Created");
}

void VulkanSwapChain::_createRenderPass()
{
	YAE_CAPTURE_FUNCTION();

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_imageFormat;
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
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Created RenderPass");
}

void VulkanSwapChain::_createFrameBuffers()
{
	// Create Frame Buffers
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = m_renderPass;
	framebufferInfo.width = m_extent.width;
	framebufferInfo.height = m_extent.height;
	framebufferInfo.layers = 1;

	for (u32 i = 0; i < m_images.size(); ++i)
	{
		VkImageView attachments[] = {
			m_images[i].imageView,
			m_images[i].depthImageView
		};

		framebufferInfo.attachmentCount = u32(countof(attachments));
		framebufferInfo.pAttachments = attachments;

		VK_VERIFY(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_images[i].frameBuffer));
	}
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Created Frame Buffers");
}

void VulkanSwapChain::_createSyncObjects()
{
	YAE_CAPTURE_FUNCTION();

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VK_VERIFY(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_syncObjects[i].imageAvailableSemaphore));
		VK_VERIFY(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_syncObjects[i].renderFinishedSemaphore));
		VK_VERIFY(vkCreateFence(m_device, &fenceInfo, nullptr, &m_syncObjects[i].inFlightFence));
	}
	YAE_VERBOSE_CAT("vulkan", "SwapChain: Created Sync Objects");
}


void VulkanSwapChain::_destroySwapChain()
{
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_swapChain = VK_NULL_HANDLE;
}

void VulkanSwapChain::_destroySwapChainImages()
{
	for (u32 i = 0; i < m_images.size(); ++i)
	{
		vkDestroyImageView(m_device, m_images[i].imageView, nullptr);
		m_images[i].imageView = VK_NULL_HANDLE;
	}
}

void VulkanSwapChain::_destroyDepthImages()
{
	for (u32 i = 0; i < m_images.size(); ++i)
	{
		vkDestroyImageView(m_device, m_images[i].depthImageView, nullptr);
		m_images[i].depthImageView = VK_NULL_HANDLE;
		vulkan::destroyImage(m_allocator, m_images[i].depthImage, m_images[i].depthImageMemory);
		m_images[i].depthImage = VK_NULL_HANDLE;
		m_images[i].depthImageMemory = VK_NULL_HANDLE;
	}	
}

void VulkanSwapChain::_destroyRenderPass()
{
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
	m_renderPass = VK_NULL_HANDLE;
}


void VulkanSwapChain::_destroyFrameBuffers()
{
	for (u32 i = 0; i < m_images.size(); ++i)
	{
		vkDestroyFramebuffer(m_device, m_images[i].frameBuffer, nullptr);
		m_images[i].frameBuffer = VK_NULL_HANDLE;
	}
}

void VulkanSwapChain::_destroySyncObjects()
{
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyFence(m_device, m_syncObjects[i].inFlightFence, nullptr);
		vkDestroySemaphore(m_device, m_syncObjects[i].renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(m_device, m_syncObjects[i].imageAvailableSemaphore, nullptr);
		m_syncObjects[i] = {};
	}
}


} // namespace yae
