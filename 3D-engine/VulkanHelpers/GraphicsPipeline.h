#pragma once

#include <initializer_list>
#include <string>

#include "Common.h"

using std::initializer_list;
using std::string;

struct ShaderConfig
{
	VkShaderStageFlagBits stage;
	string shader;
	const char* entryPoint = "main";
};

struct RasterizerConfig
{
	VkFrontFace frontFace;
	VkCullModeFlags cullMode;
	VkPolygonMode polygonMode;
	bool depthBiasEnabled;
	bool depthClampEnabled;
	bool rasterizerDiscardEnabled;
	float lineWidth;
};

struct ColorAttachmentConfig
{
	VkColorComponentFlags colorWriteMask;
	bool blendEnabled;
};

struct ColorBlendStateConfig
{
	static constexpr int BLEND_CONSTANT_COUNT = 4;

	bool logicOpEnabled;
	VkLogicOp logicOp;
	float blendConstants[BLEND_CONSTANT_COUNT];
};

struct PrimitiveConfig
{
	VkPrimitiveTopology topology;
	bool primitiveRestartEnabled;
};

struct MultisamplerConfig
{
	VkSampleCountFlagBits samples;
	bool sampleShadingEnabled;
};

struct GraphicsPipelineConfig
{
public:
	vector<ShaderConfig> shaders;
	RasterizerConfig rasterizer
	{
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.depthBiasEnabled = VK_FALSE,
		.depthClampEnabled = VK_FALSE,
		.rasterizerDiscardEnabled = VK_FALSE,
		.lineWidth = 1.f
	};
	ColorAttachmentConfig colorAttachment
	{
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.blendEnabled = VK_FALSE
	};
	ColorBlendStateConfig blendState
	{
		.logicOpEnabled = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.blendConstants = { 0.f, 0.f, 0.f, 0.f }
	};
	PrimitiveConfig primitive
	{
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnabled = VK_FALSE
	};
	MultisamplerConfig multisampler
	{
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnabled = VK_FALSE
	};

public:
	explicit GraphicsPipelineConfig(vector<ShaderConfig> shaders);

public:
	[[nodiscard]] uint32 Size() const;
	[[nodiscard]] ShaderConfig ShaderAt(uint32 index) const;

};

class GraphicsPipeline
{
	
};