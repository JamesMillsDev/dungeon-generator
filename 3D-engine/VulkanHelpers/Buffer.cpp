#include "pch.h"
#include "Buffer.h"

#include <stdexcept>

using std::runtime_error;

Buffer::Buffer(const VkPhysicalDevice physicalDevice, const VkDevice device, const size_t elementSize, 
	const size_t dataCount, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryFlags,
	const VkSharingMode sharingMode)
	: m_buffer{ VK_NULL_HANDLE }, m_bufferMemory{ VK_NULL_HANDLE }, m_usage{ usage }, m_sharingMode{ sharingMode },
	m_memoryFlags{ memoryFlags }, m_elementSize{ elementSize }, m_dataCount{ dataCount }, m_device{ device },
	m_physicalDevice{ physicalDevice }
{

}

void Buffer::Fill(const void* data) const
{
	void* memory;
	vkMapMemory(m_device, m_bufferMemory, 0, m_elementSize * m_dataCount, 0, &memory);
	memcpy(memory, data, m_elementSize * m_dataCount);
	vkUnmapMemory(m_device, m_bufferMemory);
}

VkBuffer Buffer::Get() const
{
	return m_buffer;
}

void Buffer::Create()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_elementSize * m_dataCount;
	bufferInfo.usage = m_usage;
	bufferInfo.sharingMode = m_sharingMode;

	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
	{
		throw runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(
		memRequirements.memoryTypeBits,
		m_memoryFlags
	);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS)
	{
		throw runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(m_device, m_buffer, m_bufferMemory, 0);
}

void Buffer::Destroy()
{
	vkDestroyBuffer(m_device, m_buffer, nullptr);
	vkFreeMemory(m_device, m_bufferMemory, nullptr);

	m_buffer = VK_NULL_HANDLE;
	m_bufferMemory = VK_NULL_HANDLE;
}

uint32 Buffer::FindMemoryType(const uint32 typeFilter, const VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw runtime_error("failed to find suitable memory type!");
}
