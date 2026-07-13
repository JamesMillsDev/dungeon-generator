#include "Graphics/Vulkan/VulkanGraphicsPipeline.h"

#include "Graphics/Rendering/Mesh.h"
#include "Graphics/Rendering/Shader.h"
#include "Graphics/Vulkan/Vulkan.h"

bool ShaderConfig::StageComp::operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const
{
	return lhs < rhs;
}

GraphicsPipelineConfig::GraphicsPipelineConfig(ShaderConfig shader) :
	shader{ std::move(shader) }, m_defaultLayout{ false }
{
	TList<VkDescriptorSetLayoutBinding> dslBindings;
	TList<VkDescriptorBindingFlags> flags;

	uint32 bindingIndex = 0;
	for (DescriptorConfig& descriptor : this->shader.descriptors)
	{
		dslBindings.Add(
			{
				.binding = bindingIndex++,
				.descriptorType = descriptor.type,
				.descriptorCount = descriptor.count,
				.stageFlags = descriptor.bindingFlags,
				.pImmutableSamplers = nullptr
			}
		);

		flags.Add(descriptor.bindingFlags);
	}

	if (dslBindings.IsEmpty())
	{
		m_descriptorSetLayout = Vulkan::DescriptorSetLayout();
		m_defaultLayout = true;
		return;
	}

	const VkDescriptorSetLayoutBindingFlagsCreateInfo dslFlagsCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = nullptr,
		.bindingCount = static_cast<uint32>(flags.Count()),
		.pBindingFlags = flags.Data()
	};

	const VkDescriptorSetLayoutCreateInfo dslCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &dslFlagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = static_cast<uint32>(dslBindings.size()),
		.pBindings = dslBindings.Data()
	};

	VkResult result;
	if (result = vkCreateDescriptorSetLayout(Vulkan::Device(), &dslCreateInfo, nullptr, &m_descriptorSetLayout);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Set Layout!", result);
	}

	TArray poolSizes
	{
		VkDescriptorPoolSize
		{
			.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = bindingIndex
		}
	};

	const VkDescriptorPoolCreateInfo dpCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32>(poolSizes.size()),
		.pPoolSizes = poolSizes.Data()
	};

	if (result = vkCreateDescriptorPool(Vulkan::Device(), &dpCreateInfo, nullptr, &m_descriptorPool);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Pool!", result);
	}

	// Allocate the descriptor sets
	const VkDescriptorSetVariableDescriptorCountAllocateInfo vdcAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
		.pNext = nullptr,
		.descriptorSetCount = 1,
		.pDescriptorCounts = &bindingIndex
	};

	const VkDescriptorSetAllocateInfo dsAllocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = &vdcAllocateInfo,
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_descriptorSetLayout
	};

	if (result = vkAllocateDescriptorSets(Vulkan::Device(), &dsAllocateInfo, &m_descriptorSet);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Pool!", result);
	}
}

GraphicsPipelineConfig::GraphicsPipelineConfig(const string& shaderName)
	: GraphicsPipelineConfig{ ShaderConfig{ .name = shaderName } }
{
	
}

GraphicsPipelineConfig::~GraphicsPipelineConfig()
{
	if (m_defaultLayout)
	{
		return;
	}

	vkDestroyDescriptorPool(Vulkan::Device(), m_descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(Vulkan::Device(), m_descriptorSetLayout, nullptr);
}

uint32 GraphicsPipelineConfig::Size() const
{
	return static_cast<uint32>(shader.stages.size());
}

bool GraphicsPipelineConfig::ContainsStage(VkShaderStageFlagBits stage) const
{
	return shader.stages.contains(stage);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineConfig& config) :
	m_config{ config }
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

const VkPipelineLayout& VulkanGraphicsPipeline::GetLayout() const
{
	return m_pipelineLayout;
}

void VulkanGraphicsPipeline::Init(Vulkan* vulkan)
{
	VkResult result;

	// Attempt to create the pipeline layout
	const VkPipelineLayoutCreateInfo plCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 1,
		.pSetLayouts = &m_config.m_descriptorSetLayout,
		.pushConstantRangeCount = static_cast<uint32>(m_config.pushConstantRanges.size()),
		.pPushConstantRanges = m_config.pushConstantRanges.Data()
	};

	if (result = vkCreatePipelineLayout(vulkan->GetDevice(), &plCreateInfo, nullptr, &m_pipelineLayout);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Pipeline Layout!", result);
	}

	Shader* shader = new Shader{ m_config.shader.name };
	TList<VkPipelineShaderStageCreateInfo> ssCreateInfos;
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
		ssCreateInfo.pName = m_config.shader.entryPoint.c_str();

		ssCreateInfos.Add(ssCreateInfo);
	}

	auto bindingDescription = Vertex::GetBindingDescription();
	auto attributeDescription = Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.Data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = m_config.primitive.topology;
	inputAssembly.primitiveRestartEnable = m_config.primitive.primitiveRestartEnabled;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizerInfo{};
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = m_config.rasterizer.depthClampEnabled;
	rasterizerInfo.rasterizerDiscardEnable = m_config.rasterizer.rasterizerDiscardEnabled;
	rasterizerInfo.polygonMode = m_config.rasterizer.polygonMode;
	rasterizerInfo.lineWidth = m_config.rasterizer.lineWidth;
	rasterizerInfo.cullMode = m_config.rasterizer.cullMode;
	rasterizerInfo.frontFace = m_config.rasterizer.frontFace;
	rasterizerInfo.depthBiasEnable = m_config.rasterizer.depthBiasEnabled;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = m_config.multisampler.sampleShadingEnabled;
	multisampling.rasterizationSamples = m_config.multisampler.samples;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = m_config.colorAttachment.colorWriteMask;
	colorBlendAttachment.blendEnable = m_config.colorAttachment.blendEnabled;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = m_config.blendState.logicOpEnabled;
	colorBlending.logicOp = m_config.blendState.logicOp;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	for (int i = 0; i < ColorBlendStateConfig::BLEND_CONSTANT_COUNT; ++i)
	{
		colorBlending.blendConstants[i] = m_config.blendState.blendConstants[i];
	}

	TList dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.Data();

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

	constexpr VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingCreateInfo.colorAttachmentCount = 1;
	renderingCreateInfo.pColorAttachmentFormats = &imageFormat;
	renderingCreateInfo.depthAttachmentFormat = Vulkan::Instance()->GetDepthFormat();

	VkGraphicsPipelineCreateInfo pCreateInfo{};
	pCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pCreateInfo.pNext = &renderingCreateInfo;
	pCreateInfo.stageCount = static_cast<uint32_t>(ssCreateInfos.size());
	pCreateInfo.pStages = ssCreateInfos.Data();
	pCreateInfo.pVertexInputState = &vertexInputInfo;
	pCreateInfo.pInputAssemblyState = &inputAssembly;
	pCreateInfo.pViewportState = &viewportState;
	pCreateInfo.pRasterizationState = &rasterizerInfo;
	pCreateInfo.pMultisampleState = &multisampling;
	pCreateInfo.pColorBlendState = &colorBlending;
	pCreateInfo.pDepthStencilState = &depthStencilState;
	pCreateInfo.pDynamicState = &dynamicState;
	pCreateInfo.layout = m_pipelineLayout;

	// Attempt to create the pipeline
	if (result = vkCreateGraphicsPipelines(vulkan->GetDevice(), nullptr, 1, &pCreateInfo, nullptr, &m_pipeline);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Graphics Pipeline!", result);
	}

	delete shader;
}

void VulkanGraphicsPipeline::Destroy()
{
	vkDestroyPipeline(Vulkan::Device(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(Vulkan::Device(), m_pipelineLayout, nullptr);

	m_pipelineLayout = VK_NULL_HANDLE;
	m_pipeline = VK_NULL_HANDLE;
}
