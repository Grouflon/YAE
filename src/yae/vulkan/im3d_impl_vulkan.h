#pragma once

#include <yae/types.h>
#include <yae/render_types.h>

namespace yae
{

class ShaderResource;

struct im3d_VulkanInitData
{
	VkDevice device;
	VmaAllocator allocator;
	VkExtent2D extent;
	VkRenderPass renderPass;
};

struct im3d_Instance
{
	im3d_VulkanInitData initData;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	VkPipeline pointsPipeline;
	VkPipeline linesPipeline;
	VkPipeline trianglesPipeline;
};

im3d_Instance* im3d_Init(const im3d_VulkanInitData& _initData);
void im3d_Shutdown(im3d_Instance* _context);
void im3d_NewFrame(im3d_Instance* _context);
void im3d_EndFrame(im3d_Instance* _context);

} // namespace yae
