#include "pch.h"
#include "Buffer.h"

#include <stdexcept>

#include "Vulkan.h"

using std::runtime_error;

uint32 Buffer::FindMemoryType(VkPhysicalDevice physicalDevice, const uint32 typeFilter, const VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw runtime_error("failed to find suitable memory type!");
}


Buffer::Buffer(const VkPhysicalDevice physicalDevice, const VkDevice device, const VkCommandPool commandPool,
	const VkQueue queue, const size_t elementSize, const size_t dataCount, const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags memoryFlags, const VkSharingMode sharingMode)
	: m_buffer{ VK_NULL_HANDLE }, m_bufferMemory{ VK_NULL_HANDLE }, m_usage{ usage }, m_sharingMode{ sharingMode },
	m_memoryFlags{ memoryFlags }, m_elementSize{ elementSize }, m_dataCount{ dataCount }, m_device{ device },
	m_physicalDevice{ physicalDevice }, m_commandPool{ commandPool }, m_queue{ queue }
{

}

void Buffer::Fill(const void* data)
{
	void* memory;
	vkMapMemory(m_device, m_bufferMemory, 0, Size(), 0, &memory);
	memcpy(memory, data, Size());
	vkUnmapMemory(m_device, m_bufferMemory);
}

void Buffer::Copy(const Buffer* src, const VkDeviceSize size) const
{
	// Allocate temporary memory buffer
	const VkCommandBuffer commandBuffer = Vulkan::Instance()->BeginSingleTimeCommands();

	// Run the command for copying
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0; // optional
	copyRegion.dstOffset = 0; // optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src->m_buffer, m_buffer, 1, &copyRegion);

	// End the command and submit
	Vulkan::Instance()->EndSingleTimeCommands(commandBuffer);
}

VkBuffer Buffer::Get() const
{
	return m_buffer;
}

size_t Buffer::Size() const
{
	return m_elementSize * m_dataCount;
}

void Buffer::Create()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = Size();
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
		m_physicalDevice,
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