#include "im3d_impl_vulkan.h"

#include <yae/resources/ShaderResource.h>

const char* SHADER_PATH = "./data/shaders/im3d.glsl";

namespace yae {

VkShaderStageFlagBits _ShaderTypeToVkStageFlag(ShaderType _type)
{
	switch (_type)
	{
		case SHADERTYPE_VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
		case SHADERTYPE_GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
		case SHADERTYPE_FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

VkPipeline _CreatePipeline(ShaderResource** _shaders, size_t _shaderCount, VkDevice _device, VkExtent2D _extent, VkRenderPass _renderPass, VkPipelineLayout _pipelineLayout)
{
	YAE_ASSERT(_shaderCount > 0 && _shaders != nullptr);

	DataArray<VkPipelineShaderStageCreateInfo> shaderStages(&scratchAllocator());
	for (u8 i = 0; i < _shaderCount; ++i)
	{
		YAE_ASSERT(_shaders[i] != nullptr);

		VkPipelineShaderStageCreateInfo vertexShaderStageInfo{};
		vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexShaderStageInfo.stage = _ShaderTypeToVkStageFlag(_shaders[i]->getShaderType());
		vertexShaderStageInfo.module = _shaders[i]->getShaderHandle().shaderModule;
		vertexShaderStageInfo.pName = _shaders[i]->getEntryPoint();

		shaderStages.push_back(vertexShaderStageInfo);
	}

	/*
	layout(location=0) in vec4 aPositionSize;
	layout(location=1) in vec4 aColor;
	*/

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(float) * 8;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributeDescriptions[2];
	attributeDescriptions[0] = {};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[0].offset = 0;

	attributeDescriptions[1] = {};
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attributeDescriptions[1].offset = sizeof(float) * 4;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = u32(countof(attributeDescriptions));
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_extent.width;
	viewport.height = (float)_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _extent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // optional
	depthStencil.back = {}; // optional

	VkPipeline pipeline = VK_NULL_HANDLE;
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	VK_VERIFY(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
	return pipeline;
}

im3d_Instance* im3d_Init(VkDevice _device, VkRenderPass _renderPass, VkExtent2D _extent)
{
	im3d_Instance* instance = toolAllocator().create<im3d_Instance>();
	*instance = {};

	instance->device = _device;

	// POINTS
	ShaderResource* pointsVertexShader = nullptr;
	ShaderResource* pointsFragmentShader = nullptr;
	{
		const char* defines[] = { "VERTEX_SHADER", "POINTS" };
		pointsVertexShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_VERTEX, "main", defines, countof(defines));
		YAE_ASSERT(pointsVertexShader);
		pointsVertexShader->useLoad();
	}
	{
		const char* defines[] = { "FRAGMENT_SHADER", "POINTS" };
		pointsFragmentShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_FRAGMENT, "main", defines, countof(defines));
		YAE_ASSERT(pointsFragmentShader);
		pointsFragmentShader->useLoad();
	}

	// LINES
	ShaderResource* linesVertexShader = nullptr;
	ShaderResource* linesGeometryShader = nullptr;
	ShaderResource* linesFragmentShader = nullptr;
	{
		const char* defines[] = { "VERTEX_SHADER", "LINES" };
		linesVertexShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_VERTEX, "main", defines, countof(defines));
		YAE_ASSERT(linesVertexShader);
		linesVertexShader->useLoad();
	}
	{
		const char* defines[] = { "GEOMETRY_SHADER", "LINES" };
		linesGeometryShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_GEOMETRY, "main", defines, countof(defines));
		YAE_ASSERT(linesGeometryShader);
		linesGeometryShader->useLoad();
	}
	{
		const char* defines[] = { "FRAGMENT_SHADER", "LINES" };
		linesFragmentShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_FRAGMENT, "main", defines, countof(defines));
		YAE_ASSERT(linesFragmentShader);
		linesFragmentShader->useLoad();
	}

	// TRIANGLES
	ShaderResource* trianglesVertexShader = nullptr;
	ShaderResource* trianglesFragmentShader = nullptr;
	{
		const char* defines[] = { "VERTEX_SHADER", "TRIANGLES" };
		trianglesVertexShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_VERTEX, "main", defines, countof(defines));
		YAE_ASSERT(trianglesVertexShader);
		trianglesVertexShader->useLoad();
	}
	{
		const char* defines[] = { "FRAGMENT_SHADER", "TRIANGLES" };
		trianglesFragmentShader = findOrCreateResource<ShaderResource>(SHADER_PATH, SHADERTYPE_FRAGMENT, "main", defines, countof(defines));
		YAE_ASSERT(trianglesFragmentShader);
		trianglesFragmentShader->useLoad();
	}

	// Descriptor sets
	{
		VkDescriptorSetLayoutBinding viewProjMatrixLayoutBinding;
		viewProjMatrixLayoutBinding.binding = 0;
		viewProjMatrixLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjMatrixLayoutBinding.descriptorCount = 1;
		viewProjMatrixLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding viewportLayoutBinding;
		viewportLayoutBinding.binding = 1;
		viewportLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewportLayoutBinding.descriptorCount = 1;
		viewportLayoutBinding.stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT;

		VkDescriptorSetLayoutBinding bindings[] =
		{
			viewProjMatrixLayoutBinding,
			viewportLayoutBinding
		};

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = u32(countof(bindings));
		layoutInfo.pBindings = bindings;
		VK_VERIFY(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &instance->descriptorSetLayout));
	}

	// Pipeline layout
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &instance->descriptorSetLayout;
		VK_VERIFY(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &instance->pipelineLayout));
	}

	{
		ShaderResource* shaders[] =
		{
			pointsVertexShader,
			pointsFragmentShader
		};
		instance->pointsPipeline = _CreatePipeline(shaders, countof(shaders), _device, _extent, _renderPass, instance->pipelineLayout);
	}

	{
		ShaderResource* shaders[] =
		{
			linesVertexShader,
			linesGeometryShader,
			linesFragmentShader
		};
		instance->linesPipeline = _CreatePipeline(shaders, countof(shaders), _device, _extent, _renderPass, instance->pipelineLayout);
	}

	{
		ShaderResource* shaders[] =
		{
			trianglesVertexShader,
			trianglesFragmentShader
		};
		instance->trianglesPipeline = _CreatePipeline(shaders, countof(shaders), _device, _extent, _renderPass, instance->pipelineLayout);
	}

	trianglesFragmentShader->releaseUnuse();
	trianglesVertexShader->releaseUnuse();

	linesFragmentShader->releaseUnuse();
	linesGeometryShader->releaseUnuse();
	linesVertexShader->releaseUnuse();

	pointsFragmentShader->releaseUnuse();
	pointsVertexShader->releaseUnuse();

	return instance;
}


void im3d_Shutdown(im3d_Instance* _instance)
{
	vkDestroyPipeline(_instance->device, _instance->pointsPipeline, nullptr);
	vkDestroyPipeline(_instance->device, _instance->linesPipeline, nullptr);
	vkDestroyPipeline(_instance->device, _instance->trianglesPipeline, nullptr);

	vkDestroyPipelineLayout(_instance->device, _instance->pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(_instance->device, _instance->descriptorSetLayout, nullptr);

	toolAllocator().destroy(_instance);
}


void im3d_NewFrame(im3d_Instance* _instance)
{

}


void im3d_EndFrame(im3d_Instance* _instance)
{

}

} // namespace yae
