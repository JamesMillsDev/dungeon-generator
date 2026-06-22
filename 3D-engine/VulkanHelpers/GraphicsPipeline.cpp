#include "pch.h"
#include "GraphicsPipeline.h"

#include <stdexcept>

#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"

using std::runtime_error;

bool ShaderConfig::StageComp::operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const
{
	return lhs < rhs;
}

string ShaderConfig::PassName(const VkShaderStageFlagBits pass)
{
	switch (pass)  // NOLINT(clang-diagnostic-switch-enum)
	{
		case VK_SHADER_STAGE_VERTEX_BIT:
		{
			return name + ".vert";
		}
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		{
			return name + ".tessctrl";
		}
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		{
			return name + ".tesseval";
		}
		case VK_SHADER_STAGE_GEOMETRY_BIT:
		{
			return name + ".geom";
		}
		case VK_SHADER_STAGE_FRAGMENT_BIT:
		{
			return name + ".frag";
		}
		case VK_SHADER_STAGE_COMPUTE_BIT:
		{
			return name + ".compute";
		}
		default:
		{
			return name;
		}
	}
}

GraphicsPipelineConfig::GraphicsPipelineConfig(ShaderConfig shader)
	: shader{ std::move(shader) }
{}

uint32 GraphicsPipelineConfig::Size() const
{
	return static_cast<uint32>(shader.stages.size());
}

bool GraphicsPipelineConfig::ContainsStage(const VkShaderStageFlagBits stage) const
{
	return shader.stages.contains(stage);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineConfig config, const VkDevice device,
	const VkPipelineLayout layout, const VkRenderPass renderPass)
	: m_pipeline{ VK_NULL_HANDLE }, m_device{ device }, m_config{ std::move(config) }
{
	CreateHandle(layout, renderPass);
}

GraphicsPipeline::~GraphicsPipeline()
{
	vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

VkPipeline& GraphicsPipeline::Get()
{
	return m_pipeline;
}

void GraphicsPipeline::Bind(const VkCommandBuffer cmdBuffer) const
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
}

void GraphicsPipeline::CreateHandle(VkPipelineLayout layout, VkRenderPass renderPass)
{
	auto& [shaderConfigs, rasterizer, colorAttachment, blendState, primitive, multisampler] = m_config;

	const uint32 shaderCount = m_config.Size();

	vector<Shader*> shaders;
	shaders.reserve(shaderCount);

	// Load each shader inside the pipeline config
	for (uint32 i = VK_SHADER_STAGE_VERTEX_BIT; i < VK_SHADER_STAGE_ALL_GRAPHICS; i <<= 1)
	{
		if (!m_config.ContainsStage(static_cast<VkShaderStageFlagBits>(i)))
		{
			continue;
		}

		Shader* shader = new Shader{ m_config.shader.PassName(static_cast<VkShaderStageFlagBits>(i)) };
		shader->Load(m_device);

		shaders.emplace_back(shader);
	}

	vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(shaderCount);

	// Create the pipeline information for each shader
	uint32 shaderIndex = 0;
	for (uint32 i = VK_SHADER_STAGE_VERTEX_BIT; i < VK_SHADER_STAGE_ALL_GRAPHICS; i <<= 1)
	{
		if (!m_config.ContainsStage(static_cast<VkShaderStageFlagBits>(i)))
		{
			continue;
		}

		VkPipelineShaderStageCreateInfo stageCreateInfo{};
		stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCreateInfo.stage = static_cast<VkShaderStageFlagBits>(i);
		stageCreateInfo.module = shaders[shaderIndex++]->GetModule();
		stageCreateInfo.pName = m_config.shader.entryPoint.c_str();

		shaderStages.emplace_back(stageCreateInfo);
	}

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescription = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = primitive.topology;
	inputAssembly.primitiveRestartEnable = primitive.primitiveRestartEnabled;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = rasterizer.depthClampEnabled;
	rasterizerInfo.rasterizerDiscardEnable = rasterizer.rasterizerDiscardEnabled;
	rasterizerInfo.polygonMode = rasterizer.polygonMode;
	rasterizerInfo.lineWidth = rasterizer.lineWidth;
	rasterizerInfo.cullMode = rasterizer.cullMode;
	rasterizerInfo.frontFace = rasterizer.frontFace;
	rasterizerInfo.depthBiasEnable = rasterizer.depthBiasEnabled;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = multisampler.sampleShadingEnabled;
	multisampling.rasterizationSamples = multisampler.samples;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = colorAttachment.colorWriteMask;
	colorBlendAttachment.blendEnable = colorAttachment.blendEnabled;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = blendState.logicOpEnabled;
	colorBlending.logicOp = blendState.logicOp;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	for (int i = 0; i < ColorBlendStateConfig::BLEND_CONSTANT_COUNT; ++i)
	{
		colorBlending.blendConstants[i] = blendState.blendConstants[i];
	}

	vector dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = layout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (const VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Graphics Pipeline! Error Code: {}", static_cast<uint32>(result))
		);
	}

	for (Shader* shader : shaders)
	{
		delete shader;
	}
}
