#pragma once

#include <vulkan/vulkan.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>
#include <glm/glm/vec3.hpp>
#include <glm/glm/vec2.hpp>

namespace yae {

struct TextureHandle
{
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
};

struct MeshHandle
{
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexMemory = VK_NULL_HANDLE;
};

struct ShaderHandle
{
	VkShaderModule shaderModule;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

} // namespace yae
