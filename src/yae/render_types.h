#pragma once

#include <yae/containers/Array.h>

#include <vulkan/vulkan.h>
#include <glm/glm/vec3.hpp>
#include <glm/glm/vec2.hpp>

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);

namespace yae {

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

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	DataArray<VkSurfaceFormatKHR> formats;
	DataArray<VkPresentModeKHR> presentModes;

	bool isValid() const
	{
		return !formats.empty() && !presentModes.empty();
	}
};

struct TextureHandle
{
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
};

struct MeshHandle
{
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexMemory = VK_NULL_HANDLE;
	VkDeviceSize indicesCount = 0;
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
