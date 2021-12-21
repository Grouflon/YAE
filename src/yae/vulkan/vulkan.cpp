#include "vulkan.h"

#include <VulkanMemoryAllocator/vk_mem_alloc.h>

namespace yae {

namespace vulkan {

void createOrResizeBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties)
{
    if (_inOutBuffer != VK_NULL_HANDLE)
    {
    	YAE_ASSERT(_inOutAllocation != VK_NULL_HANDLE);
    	destroyBuffer(_allocator, _inOutBuffer, _inOutAllocation);
    }

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = _size;
	bufferInfo.usage = _usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.preferredFlags = _properties;
	VK_VERIFY(vmaCreateBuffer(_allocator, &bufferInfo, &allocInfo, &_inOutBuffer, &_inOutAllocation, nullptr));
}

void destroyBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation)
{
	vmaDestroyBuffer(_allocator, _inOutBuffer, _inOutAllocation);
	_inOutBuffer = VK_NULL_HANDLE;
	_inOutAllocation = VK_NULL_HANDLE;
}

VkCommandBuffer beginSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool)
{
	VK_VERIFY(vkResetCommandPool(_device, _commandPool, 0));

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = _commandPool;
	allocateInfo.commandBufferCount = 1;
	VkCommandBuffer commandBuffer;
	VK_VERIFY(vkAllocateCommandBuffers(_device, &allocateInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_VERIFY(vkBeginCommandBuffer(commandBuffer, &beginInfo));
	return commandBuffer;
}

void endSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkCommandBuffer _commandBuffer)
{
	VK_VERIFY(vkEndCommandBuffer(_commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;
	vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE);

	vkQueueWaitIdle(_queue);

	vkFreeCommandBuffers(_device, _commandPool, 1, &_commandBuffer);
}

void copyBuffer(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(_device, _commandPool);
	
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;
	vkCmdCopyBuffer(commandBuffer, _srcBuffer, _dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(_device, _commandPool, _queue, commandBuffer);
}

void createImage(VmaAllocator _allocator, u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VmaAllocation& _outImageMemory)
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
	VK_VERIFY(vmaCreateImage(_allocator, &imageInfo, &allocCreateInfo, &_outImage, &_outImageMemory, &allocInfo));
}

void destroyImage(VmaAllocator _allocator, VkImage& _inOutImage, VmaAllocation& _inOutImageMemory)
{
	vmaDestroyImage(_allocator, _inOutImage, _inOutImageMemory);
	_inOutImage = VK_NULL_HANDLE;
	_inOutImageMemory = VK_NULL_HANDLE;
}

} // namespace vulkan

} // namespace yae