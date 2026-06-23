#include "pch.h"
#include "UniformBuffer.h"

UniformBuffer::UniformBuffer(const VkPhysicalDevice physicalDevice, const VkDevice device,
	const VkCommandPool commandPool, const VkQueue queue, const size_t elementSize,
	const size_t dataCount, const VkBufferUsageFlags usage,
	const VkMemoryPropertyFlags memoryFlags, const VkSharingMode sharingMode)
	: Buffer{ physicalDevice, device, commandPool, queue, elementSize, dataCount, usage, memoryFlags, sharingMode },
	m_memory{ nullptr }
{

}

void UniformBuffer::Fill(const void* data)
{
	memcpy(m_memory, data, Size());
}

void UniformBuffer::Create()
{
	Buffer::Create();
	vkMapMemory(m_device, m_bufferMemory, 0, Size(), 0, &m_memory);
}

void UniformBuffer::Destroy()
{
	vkUnmapMemory(m_device, m_bufferMemory);
	Buffer::Destroy();
}
