#include "pch.h"
#include "VulkanGraphicsPipeline.h"

#include "Vulkan.h"
#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Shader.h"

bool ShaderConfig::StageComp::operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const
{
	return lhs < rhs;
}

GraphicsPipelineConfig::GraphicsPipelineConfig(ShaderConfig shader) :
	shader{ std::move(shader) }, descriptorSetLayout{ Vulkan::DescriptorSetLayout() }
{}

GraphicsPipelineConfig::GraphicsPipelineConfig(const string & shaderName) :
	shader{ .name = shaderName }, descriptorSetLayout{ Vulkan::DescriptorSetLayout() }
{}

uint32 GraphicsPipelineConfig::Size() const
{
	return static_cast<uint32>(shader.stages.size());
}

bool GraphicsPipelineConfig::ContainsStage(VkShaderStageFlagBits stage) const
{
	return shader.stages.contains(stage);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(GraphicsPipelineConfig config) :
	m_config{ std::move(config) }
{
	Init(Vulkan::Instance());
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	Destroy();
}

const VkPipeline& VulkanGraphicsPipeline::Get() const
{
	return m_pipeline;
}

void VulkanGraphicsPipeline::Init(Vulkan* vulkan)
{
	VkResult result;
	auto& [shaderConfigs, rasterizer, colorAttachment, blendState,
		primitive, multisampler, descriptorSetLayout, pushConstants] = m_config;

	// Attempt to create the pipeline layout
	const VkPipelineLayoutCreateInfo plCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &descriptorSetLayout,
		.pushConstantRangeCount = static_cast<uint32>(pushConstants.size()),
		.pPushConstantRanges = pushConstants.data()
	};

	if (result = vkCreatePipelineLayout(vulkan->GetDevice(), &plCreateInfo, nullptr, &m_pipelineLayout);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Pipeline Layout!", result);
	}

	Shader* shader = new Shader{ shaderConfigs.name };
	vector<VkPipelineShaderStageCreateInfo> ssCreateInfos;
	for (uint32 i = VK_SHADER_STAGE_VERTEX_BIT; i < VK_SHADER_STAGE_ALL_GRAPHICS; i <<= 1)
	{
		if (!m_config.ContainsStage(static_cast<VkShaderStageFlagBits>(i)))
		{
			continue;
		}

		// Generate the stage's create info
		VkPipelineShaderStageCreateInfo ssCreateInfo{};
		ssCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ssCreateInfo.stage = static_cast<VkShaderStageFlagBits>(i);
		ssCreateInfo.module = shader->GetShaderModule();
		ssCreateInfo.pName = shaderConfigs.entryPoint.c_str();

		ssCreateInfos.emplace_back(ssCreateInfo);
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

	VkGraphicsPipelineCreateInfo pCreateInfo{};
	pCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pCreateInfo.stageCount = static_cast<uint32_t>(ssCreateInfos.size());
	pCreateInfo.pStages = ssCreateInfos.data();
	pCreateInfo.pVertexInputState = &vertexInputInfo;
	pCreateInfo.pInputAssemblyState = &inputAssembly;
	pCreateInfo.pViewportState = &viewportState;
	pCreateInfo.pRasterizationState = &rasterizerInfo;
	pCreateInfo.pMultisampleState = &multisampling;
	pCreateInfo.pColorBlendState = &colorBlending;
	pCreateInfo.pDynamicState = &dynamicState;
	pCreateInfo.layout = m_pipelineLayout;

	// Attempt to create the pipeline
	if (result = vkCreateGraphicsPipelines(vulkan->GetDevice(), nullptr, 1, &pCreateInfo, nullptr, &m_pipeline);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Graphics Pipeline!", result);
	}
}

void VulkanGraphicsPipeline::Destroy()
{
	vkDestroyPipeline(Vulkan::Device(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(Vulkan::Device(), m_pipelineLayout, nullptr);

	m_pipelineLayout = VK_NULL_HANDLE;
	m_pipeline = VK_NULL_HANDLE;
}
