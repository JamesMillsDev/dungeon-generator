#pragma once

#include <deque>

#include "Graphics/Vulkan/Common.h"

class Buffer;

using std::deque;

struct DescriptorWriter
{
public:
	deque<VkDescriptorImageInfo> imageInfos;
	deque<VkDescriptorBufferInfo> bufferInfos;
	vector<VkWriteDescriptorSet> writes;

private:
	VkDevice m_device;

public:
	explicit DescriptorWriter(VkDevice device);

public:
	void WriteImage(uint32 binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void WriteBuffer(uint32 binding, VkBuffer buffer, uint64 size, uint64 offset, VkDescriptorType type);

	void Clear();
	void UpdateSet(VkDescriptorSet set);

};