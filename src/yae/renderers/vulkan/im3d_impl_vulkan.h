#pragma once

#include <yae/types.h>
#include <yae/math_types.h>
#include <yae/render_types.h>
#include <yae/containers/Array.h>
#include <im3d.h>

namespace yae
{

class ShaderResource;

struct Im3dUniformBufferObject
{
	Matrix4 viewProj;
	Vector2 viewport;
};

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

struct im3d_Frame
{
	DataArray<VertexBufferData> vertexBuffers;
};

struct im3d_Instance
{
	im3d_VulkanInitData initData;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkPipelineLayout pipelineLayout;
	VkBuffer uniformBuffer;
	VmaAllocation uniformBufferMemory;

	VkPipeline pointsPipeline;
	VkPipeline linesPipeline;
	VkPipeline trianglesPipeline;

	Array<im3d_Frame> frames;
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
void im3d_CreatePipelines(im3d_Instance* _instance);
void im3d_DestroyPipelines(im3d_Instance* _instance);
void im3d_Shutdown(im3d_Instance* _instance);
void im3d_NewFrame(const im3d_FrameData& _frameData);
void im3d_Render(im3d_Instance* _instance, VkCommandBuffer _commandBuffer, u32 _frameIndex, const Matrix4& _viewProj, const Vector2& _viewport);

} // namespace yae
