#include "pch.h"
#include "Buffer.h"

#include <stdexcept>

using std::runtime_error;

Buffer::Buffer(const VkPhysicalDevice physicalDevice, const VkDevice device, const VkCommandPool commandPool,
	const VkQueue queue, const size_t elementSize, const size_t dataCount, const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags memoryFlags, const VkSharingMode sharingMode)
	: m_buffer{ VK_NULL_HANDLE }, m_bufferMemory{ VK_NULL_HANDLE }, m_usage{ usage }, m_sharingMode{ sharingMode },
	m_memoryFlags{ memoryFlags }, m_elementSize{ elementSize }, m_dataCount{ dataCount }, m_device{ device },
	m_physicalDevice{ physicalDevice }, m_commandPool{ commandPool }, m_queue{ queue }
{

}

void Buffer::Fill(const void* data) const
{
	void* memory;
	vkMapMemory(m_device, m_bufferMemory, 0, m_elementSize * m_dataCount, 0, &memory);
	memcpy(memory, data, m_elementSize * m_dataCount);
	vkUnmapMemory(m_device, m_bufferMemory);
}

void Buffer::Copy(const Buffer* src, const VkDeviceSize size) const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate temporary memory buffer
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	// Immediately begin the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Run the command for copying
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0; // optional
	copyRegion.dstOffset = 0; // optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src->m_buffer, m_buffer, 1, &copyRegion);

	// End the command and submit
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_queue);

	// Run the final cleanup
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
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
