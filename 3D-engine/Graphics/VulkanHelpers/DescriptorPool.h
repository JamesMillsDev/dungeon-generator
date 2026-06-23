#pragma once

#include <span>

#include "Common.h"

using std::span;

class DescriptorPool
{
public:
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

private:
	vector<PoolSizeRatio> m_ratios;
	vector<VkDescriptorPool> m_fullPools;
	vector<VkDescriptorPool> m_readyPools;
	uint32 m_setsPerPool;

	VkDevice m_device;

public:
	explicit DescriptorPool(VkDevice device);

public:
	void Init(uint32 initialSets, span<PoolSizeRatio> poolRatios);

	void ClearPools();
	void DestroyPools();

	VkDescriptorSet Allocate(VkDescriptorSetLayout layout, void* pNext = nullptr);

private:
	VkDescriptorPool GetPool();
	VkDescriptorPool CreatePool(uint32 setCount, span<PoolSizeRatio> poolRatios) const;

};