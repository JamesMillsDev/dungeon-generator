#pragma once

#include "Buffer.h"

class UniformBuffer : public Buffer
{
	friend class Vulkan;

private:
	void* m_memory;

protected:
	UniformBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool,
		VkQueue queue, size_t elementSize, size_t dataCount, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags memoryFlags, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);

public:
	void Fill(const void* data) override;

protected:
	void Create() override;
	void Destroy() override;

};

