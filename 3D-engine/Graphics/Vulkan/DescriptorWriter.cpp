#include "pch.h"
#include "DescriptorWriter.h"

DescriptorWriter::DescriptorWriter(const VkDevice device)
	: m_device{ device }
{
	
}

void DescriptorWriter::WriteImage(const uint32 binding, const VkImageView image, const VkSampler sampler,
                                  const VkImageLayout layout, const VkDescriptorType type)
{
	VkDescriptorImageInfo& info = imageInfos.emplace_back(VkDescriptorImageInfo
		{
			.sampler = sampler,
			.imageView = image,
			.imageLayout = layout
		});

	const VkWriteDescriptorSet write
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &info,
		.pBufferInfo = VK_NULL_HANDLE,
		.pTexelBufferView = VK_NULL_HANDLE
	};

	writes.emplace_back(write);
}

void DescriptorWriter::WriteBuffer(const uint32 binding, const VkBuffer buffer, const uint64 size, const uint64 offset,
	const VkDescriptorType type)
{
	VkDescriptorBufferInfo& info = bufferInfos.emplace_back(VkDescriptorBufferInfo
		{
			.buffer = buffer,
			.offset = offset,
			.range = size
		});

	const VkWriteDescriptorSet write
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = VK_NULL_HANDLE,
		.pBufferInfo = &info,
		.pTexelBufferView = VK_NULL_HANDLE
	};

	writes.emplace_back(write);
}

void DescriptorWriter::Clear()
{
	bufferInfos.clear();
	imageInfos.clear();
	writes.clear();
}

void DescriptorWriter::UpdateSet(const VkDescriptorSet set)
{
	for (VkWriteDescriptorSet& write : writes)
	{
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}