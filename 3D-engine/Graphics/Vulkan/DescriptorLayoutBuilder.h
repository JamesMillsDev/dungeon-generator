#pragma once

#include <vector>
#include <vulkan/vulkan.h>

using std::vector;

class DescriptorLayoutBuilder
{
private:
	vector<VkDescriptorSetLayoutBinding> m_bindings;

public:
	void AddBinding(uint32 binding, VkDescriptorType type);

	VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags stages);

};