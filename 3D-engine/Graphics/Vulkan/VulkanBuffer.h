#pragma once

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class VulkanBuffer
{
private:
	VkDeviceSize m_size;
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VmaAllocationInfo m_allocationInfo;
	VkBufferUsageFlags m_usage;

public:
	explicit VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage);
	~VulkanBuffer();

public:
	void Fill(const void* data, VkDeviceSize size = 0, size_t offset = 0) const;

private:
	void Create();
	void Destroy();

};

