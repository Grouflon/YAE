#pragma once

#include <yae/containers/Array.h>

#include <vulkan/vulkan.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#ifndef YAE_IMPLEMENTS_RENDERER_OPENGL
	#define YAE_IMPLEMENTS_RENDERER_OPENGL 0
#endif

#ifndef YAE_IMPLEMENTS_RENDERER_VULKAN
	#define YAE_IMPLEMENTS_RENDERER_VULKAN 0
#endif

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);

namespace yae {

enum class RendererType : u8
{
	Vulkan,
	OpenGL
};

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

/*
struct TextureHandle
{
	VkImage image = VK_NULL_HANDLE;
	VmaAllocation memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;
};
*/
typedef u32 TextureHandle;

/*
struct MeshHandle
{
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexMemory = VK_NULL_HANDLE;
	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexMemory = VK_NULL_HANDLE;
	VkDeviceSize indicesCount = 0;
};
*/
typedef u32 MeshHandle;

/*
struct ShaderHandle
{
	VkShaderModule shaderModule;
};
*/
typedef u32 ShaderHandle;
typedef u32 ShaderProgramHandle;

typedef u32 FrameHandle;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	Vertex() {}
	Vertex(const glm::vec3 _pos, const glm::vec3& _color, const glm::vec2& _texCoord) : pos(_pos), color(_color), texCoord(_texCoord) {}
	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

enum class ShaderType : u8
{
	UNDEFINED = 0,
	VERTEX,
	GEOMETRY,
	FRAGMENT,
};

typedef u32 FrameBufferHandle;

} // namespace yae
