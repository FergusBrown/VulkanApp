#include "Pipeline.h"

#include "Device.h"
#include "Mesh.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include "Swapchain.h"

Pipeline::Pipeline(Device& device) :
	mDevice(device)
{
}

Pipeline::~Pipeline()
{
	if (mHandle != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(mDevice.logicalDevice(), mHandle, nullptr);
	}
}

VkPipeline Pipeline::handle() const
{
	return mHandle;
}

// TODO : remove depth write and vertex input booleans - check pipeline requirements based on existing renderpass/subpass objects
GraphicsPipeline::GraphicsPipeline(Device& device,
	const std::vector<ShaderModule>& shaderModules,
	const Swapchain& swapchain,
	const PipelineLayout& pipelineLayout,
	const RenderPass& renderPass,
	uint32_t subpassIndex,
	VkBool32 vertexInput,
	VkBool32 depthWriteEnable) :
	Pipeline(device)
{

	// - SHADER STAGE
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = {};

	for (auto& shaderModule : shaderModules)
	{
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
		shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfo.stage = shaderModule.stageFlagBits();		// Shader stage name
		shaderStageCreateInfo.module = shaderModule.handle();			// Shader module to be used by stage
		shaderStageCreateInfo.pName = "main";							// Entry point into shader

		shaderStageCreateInfos.push_back(std::move(shaderStageCreateInfo));
	}


	// - VERTEX INPUT STATE 

	// TODO : rework the below
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	VkVertexInputBindingDescription bindingDescription = {};
	std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions;
	if (vertexInput)
	{
		// -- BINDING DESCRIPTION

		// How the data for a single vertex (including info such as position, colour, texture coords, normals, etc) is as a whole
		
		bindingDescription.binding = 0;								// Can bind multiple streams of data, this defines which one
		bindingDescription.stride = sizeof(Vertex);					// Size of a single vertex object
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;	// How to move between data after each vertex
																	// VK_VERTEX_INPUT_RATE_INDEX		: Move on the the next vertex
																	// VK_VERTEX_INPUT_RATE_INSTANCE	: Move to a vertex for the next mInstance
		// How the data for an attribute is defined within a vertex
		// Position Attribute
		attributeDescriptions[0].binding = 0;							// Which binding the data is at (Should be the same as above)
		attributeDescriptions[0].location = 0;							// Location in shader where data will be read from
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;	// Format the data will take (also helps define size of data)
		attributeDescriptions[0].offset = offsetof(Vertex, position);		// Where this attribute is defined in the data for a single vertex

		// Normal attribute
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		// Normal attribute
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, tangent);

		// Normal attribute
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

		// Texture attribute
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, uv);

		// TODO : Normal Attribute

		// -- CREATION
		//VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;					// List of vertex binding descriptions (data spacing/stride info)
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();		// List of Vertex Attribute descriptions (data format and where to bind to and from)
	}
	else
	{
		// -- CREATION
		//VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;
	}

	// - INPUT ASSEMBLY
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;			// Primitive type to assemble vertices as
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;						// Allow overriding of "strip" topology to start new primitives

	// - VIEWPORT AND SCISSORS
	auto& extent = swapchain.extent();

	// -- VIEWPORT
	VkViewport viewport = {};
	viewport.x = 0.0f;						// x start coordinate
	viewport.y = 0.0f;						// y start coordinate
	viewport.width = (float)extent.width;	// width of viewport
	viewport.height = (float)extent.height;	// height of viewport
	viewport.minDepth = 0.0f;				// min framebuffer depth
	viewport.maxDepth = 1.0f;				// max framebuffer depth

	// -- SCISSORS
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };				// Offset to use region from
	scissor.extent = extent;				// Extent to describe region to use, starting at offset

	// -- CREATION
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	// TODO : add dynamic states
	/*
	// -- DYNAMIC STATES --
	// Dynamic states to enable
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);		// Dynamic Viewport : Can resize in command buffer with vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);		// Dynamic Scissor	: Can resize in command buffer with vkCmdSetScissor(commandbuffer, 0, 1, &scissor);

	// Dynamic State creation info
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
	*/


	// - RASTERIZER
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
	rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateInfo.depthClampEnable = VK_FALSE;					// Change if fragments beyond near/far okanes are clipped (default) or clamped to plane
	rasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;			// Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without framebuffer
	rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;			// How to handle filling points between vertices ->anything other than fill requires a feature
	rasterizerCreateInfo.lineWidth = 1.0f;								// How thick lines should be when drawn
	rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Winding to determine which side is front
	rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;				// Which face of a tri to cull
	rasterizerCreateInfo.depthBiasEnable = VK_FALSE;					// Whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)

	// If no vertex input then assume we are drawing a full screen triangle in the vertex shader using vertex indices
	// In this case the vertices will be in clockwise order so we want to change the front face winding
	if (!vertexInput)
	{
		rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	}


	// - MULTISAMPLING
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;						// Enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		// Number of samples to use per fragment


	// - BLENDING
	// Blending decides how to blend a new colour being written to a fragment, with the old value
	// In this case destination is Render target image, source is fragment shader output

	// Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colourState = {};
	colourState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// Colours to apply blending too
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colourState.blendEnable = VK_TRUE;													// Enable blending

	// Blending uses equation: (srcColorBlendFactor * new colour) colorBlendOp (dstColorBlendFactor * old colour)
	colourState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colourState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colourState.colorBlendOp = VK_BLEND_OP_ADD;

	// Summarised: (Source A * Source RGB) + ( (1 - Source A) * Dst RGB)
	//		    =  (fragment alpha * fragment colour) + ((1 - fragment alpha) * render target alpha)

	colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colourState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colourState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colourState.alphaBlendOp = VK_BLEND_OP_ADD;
	// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

	// TODO : UPDATE
	// For now, all attachments have the same blending state parameters
	std::vector<VkPipelineColorBlendAttachmentState> colourStates(renderPass.colourAttachmentCount(subpassIndex));

	std::fill(colourStates.begin(), colourStates.end(), colourState);

	VkPipelineColorBlendStateCreateInfo colourBlendingCreateInfo = {};
	colourBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colourBlendingCreateInfo.logicOpEnable = VK_FALSE;				// Alternative to calculations is to use logical operations
	colourBlendingCreateInfo.attachmentCount = static_cast<uint32_t>(colourStates.size());
	colourBlendingCreateInfo.pAttachments = colourStates.data();


	// - DEPTH STENCIL TESTING
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.depthTestEnable = VK_TRUE;				// Enable depth checking to determine fragment write
	depthStencilCreateInfo.depthWriteEnable = depthWriteEnable;		// Enable writing to depth buffer (to replace old values)
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;		// Comparison operation that allows and an overwrite (is in front)
	depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;		// Depth bounds test: Does the depth value exist between two bounds
	depthStencilCreateInfo.stencilTestEnable = VK_FALSE;			// Enable Stencil Test


	// --GRAPHICS PIPELINE CREATION
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size());		// Number of shader stages
	pipelineCreateInfo.pStages = shaderStageCreateInfos.data();									// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;								// All the fixed function pipeline stages
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colourBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout.handle();			// Pipeline layout pipeline should use
	pipelineCreateInfo.renderPass = renderPass.handle();			// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = subpassIndex;						// Subpass of render pass to use with pipeline

	// Pipeline derivatives : Can create multiple pipeline which derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;		// Existind pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;					// or index of pipeline being created to derive from (in case creating multiple at once)

	VkResult result = vkCreateGraphicsPipelines(mDevice.logicalDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mHandle);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}
}
