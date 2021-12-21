#pragma once

#include <yae/types.h>
#include <yae/render_types.h>

namespace yae {

namespace vulkan {

void createOrResizeBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation, VkDeviceSize _size, VkBufferUsageFlags _usage, VkMemoryPropertyFlags _properties);
void destroyBuffer(VmaAllocator _allocator, VkBuffer& _inOutBuffer, VmaAllocation& _inOutAllocation);

VkCommandBuffer beginSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool);
void endSingleTimeCommands(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkCommandBuffer _commandBuffer);

void copyBuffer(VkDevice _device, VkCommandPool _commandPool, VkQueue _queue, VkBuffer _srcBuffer, VkBuffer _dstBuffer, VkDeviceSize _size);

void createImage(VmaAllocator _allocator, u32 _width, u32 _height, VkFormat _format, VkImageTiling _tiling, VkImageUsageFlags _usage, VkMemoryPropertyFlags _properties, VkImage& _outImage, VmaAllocation& _outImageMemory);
void destroyImage(VmaAllocator _allocator, VkImage& _inOutImage, VmaAllocation& _inOutImageMemory);

} // namespace vulkan

} // namespace yae