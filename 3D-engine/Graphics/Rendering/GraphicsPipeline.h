#pragma once

#include <set>
#include <string>

#include "Graphics/Vulkan/Common.h"

using std::pair;
using std::set;
using std::string;

struct ShaderConfig
{
	struct StageComp
	{
		bool operator()(const VkShaderStageFlagBits& lhs, const VkShaderStageFlagBits& rhs) const;
	};

public:
	set<VkShaderStageFlagBits, StageComp> stages = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
	string name;
	string entryPoint = "main";

public:
	string PassName(VkShaderStageFlagBits pass);
};

struct RasterizerConfig
{
	VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
	bool depthBiasEnabled = VK_FALSE;
	bool depthClampEnabled = VK_FALSE;
	bool rasterizerDiscardEnabled = VK_FALSE;
	float lineWidth = 1.f;
};

struct ColorAttachmentConfig
{
	VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	bool blendEnabled = VK_FALSE;
};

struct ColorBlendStateConfig
{
	static constexpr int BLEND_CONSTANT_COUNT = 4;

	bool logicOpEnabled = VK_FALSE;
	VkLogicOp logicOp = VK_LOGIC_OP_COPY;
	float blendConstants[BLEND_CONSTANT_COUNT] = { 0.f, 0.f, 0.f, 0.f };
};

struct PrimitiveConfig
{
	VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	bool primitiveRestartEnabled = VK_FALSE;
};

struct MultisamplerConfig
{
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	bool sampleShadingEnabled = VK_FALSE;
};

struct GraphicsPipelineConfig
{
public:
	ShaderConfig shader;
	RasterizerConfig rasterizer;
	ColorAttachmentConfig colorAttachment;
	ColorBlendStateConfig blendState;
	PrimitiveConfig primitive;
	MultisamplerConfig multisampler;

public:
	explicit GraphicsPipelineConfig(ShaderConfig shader);
	explicit GraphicsPipelineConfig(const string& shaderName);

public:
	[[nodiscard]] uint32 Size() const;
	[[nodiscard]] bool ContainsStage(VkShaderStageFlagBits stage) const;

};

class GraphicsPipeline
{
	friend class Vulkan;

private:
	VkPipeline m_pipeline;
	VkDevice m_device;
	GraphicsPipelineConfig m_config;

private:
	GraphicsPipeline(GraphicsPipelineConfig config, VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass);
	~GraphicsPipeline();

public:
	VkPipeline& Get();
	void Bind(VkCommandBuffer cmdBuffer) const;

private:
	void CreateHandle(VkPipelineLayout layout, VkRenderPass renderPass);

};