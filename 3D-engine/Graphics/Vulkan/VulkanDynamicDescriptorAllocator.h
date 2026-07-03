#pragma once

#include <span>
#include <vector>

#include "Maths/Alias.h"

using std::span;
using std::vector;

class Vulkan;

class VulkanDynamicDescriptorAllocator
{
public:
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

private:
	[[nodiscard]] static VkDescriptorPool CreatePool(uint32 setCount, const span<PoolSizeRatio>& poolRatios, const Vulkan* vulkan);

private:
	vector<PoolSizeRatio> m_ratios;
	vector<VkDescriptorPool> m_fullPools;
	vector<VkDescriptorPool> m_readyPools;

	uint32 m_setsPerPool;

public:
	VulkanDynamicDescriptorAllocator(uint32 initialSets, const span<PoolSizeRatio>& poolRatios, Vulkan* vulkan);
	~VulkanDynamicDescriptorAllocator();

public:
	VkDescriptorSet Allocate(VkDescriptorSetLayout layout, void* pNext = nullptr, Vulkan* vulkan = nullptr);

	void Clear();

private:
	void Init(uint32 initialSets, const span<PoolSizeRatio>& poolRatios, const Vulkan* vulkan);
	void Destroy();

	[[nodiscard]] VkDescriptorPool GetPool(const Vulkan* vulkan);

};