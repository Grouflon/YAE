#include "im3d_impl_vulkan.h"

#include <yae/vulkan/VulkanRenderer.h>
#include <yae/vulkan/vulkan.h>
#include <yae/resources/ShaderResource.h>

#include <im3d/im3d.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

const char* SHADER_PATH = "./data/shaders/im3d.glsl";

namespace yae {

struct UniformBufferObject
{
	Matrix4 viewProj;
	Vector2 viewport;
};

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

enum PrimitiveType
{
	PRIMITIVETYPE_POINT,	
	PRIMITIVETYPE_LINE,
	PRIMITIVETYPE_TRIANGLE,
};

VkPipeline _CreatePipeline(ShaderResource** _shaders, size_t _shaderCount, const im3d_VulkanInitData& _initData, VkPipelineLayout _pipelineLayout, PrimitiveType _primitiveType)
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
	Vec4   m_positionSize; // xyz = position, w = size
	Color  m_color;        // rgba8 (MSB = r)

	against

	layout(location=0) in vec4 aPositionSize;
	layout(location=1) in vec4 aColor;
	*/

	VkPrimitiveTopology topology;
	VkCullModeFlagBits cullMode;
	switch(_primitiveType)
	{
		case PRIMITIVETYPE_POINT:
			topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			cullMode = VK_CULL_MODE_NONE;
			break;
		case PRIMITIVETYPE_LINE:
			topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			cullMode = VK_CULL_MODE_NONE;
			break;
		case PRIMITIVETYPE_TRIANGLE:
			topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			cullMode = VK_CULL_MODE_BACK_BIT;
			break;
	}

	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = (sizeof(float) * 4) + sizeof(u32);
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
	attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
	attributeDescriptions[1].offset = sizeof(float) * 4;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = u32(countof(attributeDescriptions));
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_initData.extent.width;
	viewport.height = (float)_initData.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _initData.extent;

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
	rasterizer.cullMode = cullMode;
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
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = _primitiveType == PRIMITIVETYPE_TRIANGLE ? VK_TRUE : VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

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
	pipelineInfo.renderPass = _initData.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	VK_VERIFY(vkCreateGraphicsPipelines(_initData.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
	return pipeline;
}

im3d_Instance* im3d_Init(const im3d_VulkanInitData& _initData)
{
	im3d_Instance* instance = toolAllocator().create<im3d_Instance>();
	*instance = {};

	instance->initData = _initData;

	// Descriptor sets & Uniform buffers
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
		VK_VERIFY(vkCreateDescriptorSetLayout(_initData.device, &layoutInfo, nullptr, &instance->descriptorSetLayout));

		VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = instance->initData.descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &instance->descriptorSetLayout;
        VK_VERIFY(vkAllocateDescriptorSets(instance->initData.device, &allocInfo, &instance->descriptorSet));

        vulkan::createOrResizeBuffer(
        	instance->initData.allocator,
        	instance->uniformBuffer,
        	instance->uniformBufferMemory,
        	sizeof(UniformBufferObject),
        	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        VkWriteDescriptorSet descriptorWrites[2] = {};
        {
        	VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = instance->uniformBuffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Matrix4);
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = instance->descriptorSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;
			descriptorWrites[0].pImageInfo = nullptr;
			descriptorWrites[0].pTexelBufferView = nullptr;	
        }

        {
        	VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = instance->uniformBuffer;
			bufferInfo.offset = sizeof(Matrix4);
			bufferInfo.range = sizeof(Vector2);
			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = instance->descriptorSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &bufferInfo;
			descriptorWrites[1].pImageInfo = nullptr;
			descriptorWrites[1].pTexelBufferView = nullptr;	
        }

		vkUpdateDescriptorSets(_initData.device, u32(countof(descriptorWrites)), descriptorWrites, 0, nullptr);
	}

	// Pipeline layout
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &instance->descriptorSetLayout;
		VK_VERIFY(vkCreatePipelineLayout(_initData.device, &pipelineLayoutInfo, nullptr, &instance->pipelineLayout));
	}

	im3d_CreatePipelines(instance);
	return instance;
}


void im3d_CreatePipelines(im3d_Instance* _instance)
{
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

	// CREATE PIPELINES
	{
		ShaderResource* shaders[] =
		{
			pointsVertexShader,
			pointsFragmentShader
		};
		_instance->pointsPipeline = _CreatePipeline(shaders, countof(shaders), _instance->initData, _instance->pipelineLayout, PRIMITIVETYPE_POINT);
	}

	{
		ShaderResource* shaders[] =
		{
			linesVertexShader,
			linesGeometryShader,
			linesFragmentShader
		};
		_instance->linesPipeline = _CreatePipeline(shaders, countof(shaders), _instance->initData, _instance->pipelineLayout, PRIMITIVETYPE_LINE);
	}

	{
		ShaderResource* shaders[] =
		{
			trianglesVertexShader,
			trianglesFragmentShader
		};
		_instance->trianglesPipeline = _CreatePipeline(shaders, countof(shaders), _instance->initData, _instance->pipelineLayout, PRIMITIVETYPE_TRIANGLE);
	}

	trianglesFragmentShader->releaseUnuse();
	trianglesVertexShader->releaseUnuse();

	linesFragmentShader->releaseUnuse();
	linesGeometryShader->releaseUnuse();
	linesVertexShader->releaseUnuse();

	pointsFragmentShader->releaseUnuse();
	pointsVertexShader->releaseUnuse();
}


void im3d_DestroyPipelines(im3d_Instance* _instance)
{
	vkDestroyPipeline(_instance->initData.device, _instance->pointsPipeline, nullptr);
	vkDestroyPipeline(_instance->initData.device, _instance->linesPipeline, nullptr);
	vkDestroyPipeline(_instance->initData.device, _instance->trianglesPipeline, nullptr);

	_instance->pointsPipeline = VK_NULL_HANDLE;
	_instance->linesPipeline = VK_NULL_HANDLE;
	_instance->trianglesPipeline = VK_NULL_HANDLE;
}


void im3d_Shutdown(im3d_Instance* _instance)
{
	for (VertexBufferData& data : _instance->vertexBuffers)
	{
		vulkan::destroyBuffer(_instance->initData.allocator, data.vertexBuffer, data.vertexBufferMemory);	
	}
	_instance->vertexBuffers.clear();

	vulkan::destroyBuffer(_instance->initData.allocator, _instance->uniformBuffer, _instance->uniformBufferMemory);

	im3d_DestroyPipelines(_instance);

	vkDestroyPipelineLayout(_instance->initData.device, _instance->pipelineLayout, nullptr);
	vkFreeDescriptorSets(_instance->initData.device, _instance->initData.descriptorPool, 1, &_instance->descriptorSet);
	vkDestroyDescriptorSetLayout(_instance->initData.device, _instance->descriptorSetLayout, nullptr);

	toolAllocator().destroy(_instance);
}


void im3d_NewFrame(im3d_Instance* _instance, const im3d_FrameData& _frameData)
{
	YAE_CAPTURE_FUNCTION();

	Im3d::AppData& ad = Im3d::GetAppData();

	Vector2 viewportSize = Vector2(float(_instance->initData.extent.width), float(_instance->initData.extent.height));

	ad.m_deltaTime     = _frameData.deltaTime;
	ad.m_viewportSize  = _frameData.viewportSize;
	ad.m_viewOrigin    = _frameData.camera.position; // for VR use the head position
	ad.m_viewDirection = _frameData.camera.direction;
	ad.m_worldUp       = Im3d::Vec3(0.0f, 1.0f, 0.0f); // used internally for generating orthonormal bases
	ad.m_projOrtho     = _frameData.camera.orthographic; 
	
 // m_projScaleY controls how gizmos are scaled in world space to maintain a constant screen height
	ad.m_projScaleY = _frameData.camera.orthographic
		? 2.0f / _frameData.camera.projection[1][1] // use far plane height for an ortho projection
		: tanf(_frameData.camera.fov * 0.5f) * 2.0f // or vertical fov for a perspective projection
		;  

 // World space cursor ray from mouse position; for VR this might be the position/orientation of the HMD or a tracked controller.
	Vector2 cursorPos = _frameData.cursorPosition;
	cursorPos = (cursorPos / viewportSize) * 2.0f - 1.0f;
	cursorPos.y = -cursorPos.y; // window origin is top-left, ndc is bottom-left
	Vector4 rayOrigin, rayDirection;
	Matrix4 worldMatrix = inverse(_frameData.camera.view);
	if (_frameData.camera.orthographic)
	{
		rayOrigin.x  = cursorPos.x / _frameData.camera.projection[0][0];
		rayOrigin.y  = cursorPos.y / _frameData.camera.projection[1][1];
		rayOrigin.z  = 0.0f;
		rayOrigin.w = 1.0f;
		rayOrigin    = worldMatrix * rayOrigin;
		rayDirection = worldMatrix * Vector4(0.0f, 0.0f, -1.0f, 0.0f);
		 
	}
	else
	{
		rayOrigin = Vector4(ad.m_viewOrigin, 1.0f);
		rayDirection.x  = cursorPos.x / _frameData.camera.projection[0][0];
		rayDirection.y  = cursorPos.y / _frameData.camera.projection[1][1];
		rayDirection.z  = -1.0f;
		rayDirection.w  = 0.0f;
		rayDirection    = worldMatrix * normalize(rayDirection);
	}
	ad.m_cursorRayOrigin = rayOrigin.xyz();
	ad.m_cursorRayDirection = rayDirection.xyz();

 // Set cull frustum planes. This is only required if IM3D_CULL_GIZMOS or IM3D_CULL_PRIMTIIVES is enable via
 // im3d_config.h, or if any of the IsVisible() functions are called.
	Matrix4 viewProj = Matrix4(_frameData.camera.projection * _frameData.camera.view);
	//ad.setCullFrustum(viewProj, true);

	for (int i = 0; i < Im3d::Action_Count; ++i)
	{
		ad.m_keyDown[i] = _frameData.actionKeyStates[i];
	}

 // Fill the key state array; using GetAsyncKeyState here but this could equally well be done via the window proc.
 // All key states have an equivalent (and more descriptive) 'Action_' enum.
	//ad.m_keyDown[Im3d::Mouse_Left/*Im3d::Action_Select*/] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

 // The following key states control which gizmo to use for the generic Gizmo() function. Here using the left ctrl
 // key as an additional predicate.
	//bool ctrlDown = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_L/*Action_GizmoLocal*/]       = ctrlDown && (GetAsyncKeyState(0x4c) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_T/*Action_GizmoTranslation*/] = ctrlDown && (GetAsyncKeyState(0x54) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_R/*Action_GizmoRotation*/]    = ctrlDown && (GetAsyncKeyState(0x52) & 0x8000) != 0;
	//ad.m_keyDown[Im3d::Key_S/*Action_GizmoScale*/]       = ctrlDown && (GetAsyncKeyState(0x53) & 0x8000) != 0;

 // Enable gizmo snapping by setting the translation/rotation/scale increments to be > 0
	//ad.m_snapTranslation = ctrlDown ? 0.5f : 0.0f;
	//ad.m_snapRotation    = ctrlDown ? Im3d::Radians(30.0f) : 0.0f;
	//ad.m_snapScale       = ctrlDown ? 0.5f : 0.0f;

	ad.m_snapTranslation = 0.0f;
	ad.m_snapRotation    = 0.0f;
	ad.m_snapScale       = 0.0f;

	// Update uniform buffer objects
	UniformBufferObject ubo;
	ubo.viewProj = viewProj;
	ubo.viewport = viewportSize;
	void* data;
	VK_VERIFY(vmaMapMemory(_instance->initData.allocator, _instance->uniformBufferMemory, &data));
	memcpy(data, &ubo, sizeof(ubo));
	vmaUnmapMemory(_instance->initData.allocator, _instance->uniformBufferMemory);

	Im3d::NewFrame();
}


void im3d_EndFrame(im3d_Instance* _instance, VkCommandBuffer _commandBuffer)
{
	YAE_CAPTURE_FUNCTION();

	Im3d::EndFrame();

	_instance->vertexBuffers.resize(Im3d::GetDrawListCount(), VertexBufferData());
	for (u32 i = 0, n = Im3d::GetDrawListCount(); i < n; ++i)
	{
		const Im3d::DrawList& drawList = Im3d::GetDrawLists()[i];
		VertexBufferData& vertexBufferData = _instance->vertexBuffers[i];

		size_t vertexBufferSize = drawList.m_vertexCount * sizeof(Im3d::VertexData);
    	if (vertexBufferData.vertexBuffer == VK_NULL_HANDLE || vertexBufferData.vertexBufferSize < vertexBufferSize)
    	{
        	vulkan::createOrResizeBuffer(_instance->initData.allocator, vertexBufferData.vertexBuffer, vertexBufferData.vertexBufferMemory, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        	YAE_ASSERT(vertexBufferData.vertexBuffer != VK_NULL_HANDLE);
        	YAE_ASSERT(vertexBufferData.vertexBufferMemory != VK_NULL_HANDLE);

        	vertexBufferData.vertexBufferSize = vertexBufferSize;
    	}
    	
        void* mappedMemory = nullptr;
        VK_VERIFY(vmaMapMemory(_instance->initData.allocator, vertexBufferData.vertexBufferMemory, &mappedMemory));
        memcpy(mappedMemory, drawList.m_vertexData, vertexBufferSize);
        vmaUnmapMemory(_instance->initData.allocator, vertexBufferData.vertexBufferMemory);

        VkPipeline pipeline = VK_NULL_HANDLE;
        switch (drawList.m_primType)
		{
			case Im3d::DrawPrimitive_Points:
				pipeline = _instance->pointsPipeline;
				/*prim = GL_POINTS;
				sh = g_Im3dShaderPoints;
				glAssert(glDisable(GL_CULL_FACE)); // points are view-aligned*/
				break;
			case Im3d::DrawPrimitive_Lines:
				pipeline = _instance->linesPipeline;
				/*prim = GL_LINES;
				sh = g_Im3dShaderLines;
				glAssert(glDisable(GL_CULL_FACE)); // lines are view-aligned*/
				break;
			case Im3d::DrawPrimitive_Triangles:
				pipeline = _instance->trianglesPipeline;
				/*prim = GL_TRIANGLES;
				sh = g_Im3dShaderTriangles;
				//glAssert(glEnable(GL_CULL_FACE)); // culling valid for triangles, but optional*/
				break;
			default:
				IM3D_ASSERT(false);
				return;
		};

		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &vertexBufferData.vertexBuffer, offsets);
		vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _instance->pipelineLayout, 0, 1, &_instance->descriptorSet, 0, NULL);
		vkCmdDraw(_commandBuffer, drawList.m_vertexCount, 1, 0, 0);
	}
}

} // namespace yae
