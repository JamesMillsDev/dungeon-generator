#include "pch.h"
#include "GraphicsPipeline.h"

GraphicsPipelineConfig::GraphicsPipelineConfig(vector<ShaderConfig> shaders)
	: shaders{ std::move(shaders) }
{}

uint32 GraphicsPipelineConfig::Size() const
{
	return static_cast<uint32>(shaders.size());
}

ShaderConfig GraphicsPipelineConfig::ShaderAt(const uint32 index) const
{
	return shaders[index];
}
