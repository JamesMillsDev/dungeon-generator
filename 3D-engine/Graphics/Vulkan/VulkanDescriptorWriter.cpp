#include "pch.h"
#include "VulkanDescriptorWriter.h"

void VulkanDescriptorWriter::Write(const uint32 binding, const VkImageView image, const VkSampler sampler,
	const VkImageLayout layout, const VkDescriptorType type, const uint32 count, const uint32 element)
{
	VkDescriptorImageInfo& info = imageInfos.emplace_back(
		VkDescriptorImageInfo
		{
			.sampler = sampler,
			.imageView = image,
			.imageLayout = layout
		}
	);

	VkWriteDescriptorSet write =
	{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = VK_NULL_HANDLE,
			.dstBinding = binding,
			.dstArrayElement = element,
			.descriptorCount = count,
			.descriptorType = type,
			.pImageInfo = &info,
			.pBufferInfo = nullptr,
			.pTexelBufferView = nullptr
	};

	writes.emplace_back(write);
}

void VulkanDescriptorWriter::Write(const uint32 binding, const VkBuffer buffer, const uint64 size,
	const uint64 offset, const VkDescriptorType type, const uint32 count, const uint32 element)
{
	VkDescriptorBufferInfo& info = bufferInfos.emplace_back(
		VkDescriptorBufferInfo
		{
			.buffer = buffer,
			.offset = offset,
			.range = size
		}
	);

	VkWriteDescriptorSet write =
	{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = VK_NULL_HANDLE,
			.dstBinding = binding,
			.dstArrayElement = element,
			.descriptorCount = count,
			.descriptorType = type,
			.pImageInfo = nullptr,
			.pBufferInfo = &info,
			.pTexelBufferView = nullptr
	};

	writes.emplace_back(write);
}

void VulkanDescriptorWriter::UpdateSet(const VkDescriptorSet set, const VkDevice device)
{
	for (VkWriteDescriptorSet& write : writes)
	{
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(device, static_cast<uint32>(writes.size()), writes.data(), 0, nullptr);
}

void VulkanDescriptorWriter::Clear()
{
	imageInfos.clear();
	bufferInfos.clear();
	writes.clear();
}