#include "pch.h"
#include "DescriptorLayoutBuilder.h"

#include <stdexcept>

using std::runtime_error;

void DescriptorLayoutBuilder::AddBinding(const uint32 binding, const VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.descriptorCount = 1;
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = type;

	m_bindings.emplace_back(layoutBinding);
}

VkDescriptorSetLayout DescriptorLayoutBuilder::Build(const VkDevice device, const VkShaderStageFlags stages)
{
	for (VkDescriptorSetLayoutBinding& binding : m_bindings)
	{
		binding.stageFlags = stages;
	}

	VkDescriptorSetLayout layout;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32>(m_bindings.size());
	layoutInfo.pBindings = m_bindings.data();

	if (const VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Descriptor Set Layout! Error Code: {}", static_cast<int32>(result))
		);
	}

	return layout;
}
