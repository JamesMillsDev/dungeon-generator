#include "pch.h"
#include "VulkanBuffer.h"

#include "Vulkan.h"

VulkanBuffer::VulkanBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage)
	: m_size{ size }, m_buffer{ VK_NULL_HANDLE }, m_allocation{ VK_NULL_HANDLE },
	m_usage{ usage }
{
	Create();
}

VulkanBuffer::~VulkanBuffer()
{
	Destroy();
}

void VulkanBuffer::Fill(const void* data, VkDeviceSize size, const size_t offset) const
{
	size = size == 0 ? m_size : size;
	memcpy(static_cast<char*>(m_allocationInfo.pMappedData) + offset, data, size);
}

void VulkanBuffer::Create()
{
	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = m_size;
	bufferCreateInfo.usage = m_usage;

	VmaAllocationCreateInfo bufferAllocCreateInfo{};
	bufferAllocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
		VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | 
		VMA_ALLOCATION_CREATE_MAPPED_BIT;
	bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	// Attempt to allocate the memory
	if (const VkResult result = vmaCreateBuffer(Vulkan::Allocator(), &bufferCreateInfo, &bufferAllocCreateInfo, &m_buffer, &m_allocation, &m_allocationInfo);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to allocate Mesh Buffer!", result);
	}
}

void VulkanBuffer::Destroy()
{
	vmaDestroyBuffer(Vulkan::Allocator(), m_buffer, m_allocation);
	
	m_buffer = VK_NULL_HANDLE;
	m_allocation = VK_NULL_HANDLE;

	m_allocationInfo.pMappedData = nullptr;
}