#pragma once

#include <deque>
#include <vector>
#include <vulkan/vulkan.h>

#include "Maths/Alias.h"

using std::deque;
using std::vector;

class VulkanDescriptorWriter
{
public:
	deque<VkDescriptorImageInfo> imageInfos;
	deque<VkDescriptorBufferInfo> bufferInfos;
	vector<VkWriteDescriptorSet> writes;

public:
	void Write(uint32 binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type, uint32 count = 1, uint32 element = 0);
	void Write(uint32 binding, VkBuffer buffer, uint64 size, uint64 offset, VkDescriptorType type, uint32 count = 1, uint32 element = 0);

	void UpdateSet(VkDescriptorSet set, VkDevice device);

	void Clear();

};