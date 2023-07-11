#pragma once

#include <yae/containers/Array.h>

#include <mirror.h>

#ifndef YAE_IMPLEMENTS_RENDERER_OPENGL
	#define YAE_IMPLEMENTS_RENDERER_OPENGL 0
#endif

#ifndef YAE_IMPLEMENTS_RENDERER_VULKAN
	#define YAE_IMPLEMENTS_RENDERER_VULKAN 0
#endif

#if YAE_IMPLEMENTS_RENDERER_VULKAN
#include <vulkan/vulkan.h>

#define VK_VERIFY(_exp) if ((_exp) != VK_SUCCESS) { YAE_ERROR_CAT("vulkan", "Failed Vulkan call: "#_exp); YAE_ASSERT(false); }

VK_DEFINE_HANDLE(VmaAllocator);
VK_DEFINE_HANDLE(VmaAllocation);
#endif

namespace yae {

enum class RendererType : u8
{
	Vulkan,
	OpenGL
};

#if YAE_IMPLEMENTS_RENDERER_VULKAN
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
#endif

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

enum class TextureFilter : i8
{
	LINEAR,
	NEAREST,
};
MIRROR_ENUM_CLASS(TextureFilter)
(
	MIRROR_ENUM_CLASS_VALUE(LINEAR)()
	MIRROR_ENUM_CLASS_VALUE(NEAREST)()
);

struct YAE_API TextureParameters
{
	TextureFilter filter = TextureFilter::LINEAR;

	MIRROR_CLASS_NOVIRTUAL(TextureParameters)
	(
		MIRROR_MEMBER(filter)();
	)
};

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
	Vector3 pos;
	Vector2 texCoord;
	Vector3 normal;
	Vector3 color;

	Vertex() {}
	Vertex(const Vector3& _pos, const Vector2& _texCoord, const Vector3& _normal, const Vector3& _color) : pos(_pos), texCoord(_texCoord), normal(_normal), color(_color) {}
	bool operator==(const Vertex& other) const {
		return pos == other.pos && texCoord == other.texCoord && normal == other.normal && color == other.color ;
	}
};

enum class ShaderType : u8
{
	UNDEFINED = 0,
	VERTEX,
	GEOMETRY,
	FRAGMENT,
};
MIRROR_ENUM_CLASS(ShaderType)
(
	MIRROR_ENUM_CLASS_VALUE(UNDEFINED)();
	MIRROR_ENUM_CLASS_VALUE(VERTEX)();
	MIRROR_ENUM_CLASS_VALUE(GEOMETRY)();
	MIRROR_ENUM_CLASS_VALUE(FRAGMENT)();
);

enum class PrimitiveMode : u8
{
	POINTS = 0,
	LINE_STRIP,
	LINE_LOOP,
	TRIANGLES,
	TRIANGLE_STRIP
};

typedef u32 FrameBufferHandle;

} // namespace yae
