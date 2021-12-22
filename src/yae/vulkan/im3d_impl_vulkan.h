#pragma once

#include <yae/types.h>
#include <yae/math_types.h>
#include <yae/render_types.h>
#include <yae/containers/Array.h>
#include <im3d.h>

namespace yae
{

class ShaderResource;

struct im3d_VulkanInitData
{
	VkDevice device;
	VmaAllocator allocator;
	VkDescriptorPool descriptorPool;
	VkExtent2D extent;
	VkRenderPass renderPass;
};

struct VertexBufferData
{
	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexBufferMemory = VK_NULL_HANDLE;
	size_t vertexBufferSize = 0;
};

struct im3d_Instance
{
	im3d_VulkanInitData initData;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;

	VkPipeline pointsPipeline;
	VkPipeline linesPipeline;
	VkPipeline trianglesPipeline;

	DataArray<VertexBufferData> vertexBuffers;

	VkBuffer uniformBuffer;
	VmaAllocation uniformBufferMemory;
};

struct im3d_Camera
{
	Vector3 position;
	Vector3 direction;
	Matrix4 view;
	Matrix4 projection;
	float fov; // in radians
	bool orthographic;
};

struct im3d_FrameData
{
	float deltaTime;
	Vector2 cursorPosition; // window relative, non-normalized
	Vector2 viewportSize;
	im3d_Camera camera;
	bool actionKeyStates[Im3d::Action_Count];
};

im3d_Instance* im3d_Init(const im3d_VulkanInitData& _initData);
void im3d_Shutdown(im3d_Instance* _context);
void im3d_NewFrame(im3d_Instance* _context, const im3d_FrameData& _frameData);
void im3d_EndFrame(im3d_Instance* _context, VkCommandBuffer _commandBuffer);

} // namespace yae
