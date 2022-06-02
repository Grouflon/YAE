#pragma once

#include <yae/types.h>
#include <yae/rendering/render_types.h>

namespace yae {

const u32 INVALID_MEMORY_TYPE = ~0u;

namespace vulkan {

void createOrResizeBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties);
void destroyBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation);

VkCommandBuffer beginSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool);
void endSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkCommandBuffer _commandBuffer);

void copyBuffer(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);

void createImage(VmaAllocator _allocator, u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VmaAllocation& _outImageMemory);
void destroyImage(VmaAllocator _allocator, VkImage& _inOutImage, VmaAllocation& _inOutImageMemory);

VkImageView createImageView(VkDevice _device, VkImage _image, VkFormat _format, VkImageAspectFlags _aspectFlags);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice _device, VkSurfaceKHR _surface);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice _device, VkSurfaceKHR _surface);
VkFormat findSupportedFormat(VkPhysicalDevice _physicalDevice, VkFormat* _candidates, size_t _candidateCount, VkImageTiling _tiling, VkFormatFeatureFlags _features);
u32 findMemoryType(VkPhysicalDevice _physicalDevice, u32 _typeFilter, VkMemoryPropertyFlags _properties);

} // namespace vulkan

} // namespace yae