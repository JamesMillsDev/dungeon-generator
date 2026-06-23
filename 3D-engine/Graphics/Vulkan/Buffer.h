#pragma once

#include "Graphics/Vulkan/Common.h"
#include "Maths/Alias.h"

class Buffer
{
	friend class Vulkan;

public:
	[[nodiscard]] static uint32 FindMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties);

protected:
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

protected:
	Buffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, 
		VkQueue queue, size_t elementSize, size_t dataCount, VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags memoryFlags, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);
	virtual ~Buffer() = default;

public:
	virtual void Fill(const void* data);
	void Copy(const Buffer* src, VkDeviceSize size) const;

	[[nodiscard]] VkBuffer Get() const;
	[[nodiscard]] size_t Size() const;

protected:
	virtual void Create();
	virtual void Destroy();


};