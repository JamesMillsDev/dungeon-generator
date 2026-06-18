#pragma once

#include "Common.h"
#include "Maths/Alias.h"

class Buffer
{
	friend class Vulkan;

private:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;
	VkBufferUsageFlags m_usage;
	VkSharingMode m_sharingMode;
	VkMemoryPropertyFlags m_memoryFlags;

	size_t m_elementSize;
	size_t m_dataCount;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_queue;

private:
	Buffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, 
		VkQueue queue, size_t elementSize, size_t dataCount, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags memoryFlags, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

public:
	void Fill(const void* data) const;
	void Copy(const Buffer* src, VkDeviceSize size) const;

	[[nodiscard]] VkBuffer Get() const;
	[[nodiscard]] size_t Size() const;

private:
	void Create();
	void Destroy();

	[[nodiscard]] uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) const;

};