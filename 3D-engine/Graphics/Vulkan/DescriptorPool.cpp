#include "pch.h"
#include "DescriptorPool.h"

#include <stdexcept>

using std::runtime_error;

constexpr uint32 MAX_SETS_PER_POOL = 4092;

DescriptorPool::DescriptorPool(const VkDevice device)
	: m_setsPerPool{ 0 }, m_device{ device }
{

}

void DescriptorPool::Init(const uint32 initialSets, const span<PoolSizeRatio> poolRatios)
{
	m_ratios.clear();

	for (PoolSizeRatio r : poolRatios)
	{
		m_ratios.emplace_back(r);
	}

	VkDescriptorPool newPool = CreatePool(initialSets, poolRatios);
	m_setsPerPool = static_cast<uint32>(initialSets * 1.5);

	m_readyPools.emplace_back(newPool);
}

void DescriptorPool::ClearPools()
{
	for (const VkDescriptorPool& p : m_readyPools)
	{
		vkResetDescriptorPool(m_device, p, 0);
	}

	for (const VkDescriptorPool& p : m_fullPools)
	{
		vkResetDescriptorPool(m_device, p, 0);
		m_readyPools.emplace_back(p);
	}

	m_fullPools.clear();
}

void DescriptorPool::DestroyPools()
{
	for (const VkDescriptorPool& p : m_readyPools)
	{
		vkDestroyDescriptorPool(m_device, p, nullptr);
	}
	m_readyPools.clear();

	for (const VkDescriptorPool& p : m_fullPools)
	{
		vkDestroyDescriptorPool(m_device, p, nullptr);
	}

	m_fullPools.clear();
}

VkDescriptorSet DescriptorPool::Allocate(VkDescriptorSetLayout layout, void* pNext)
{
	VkDescriptorPool poolToUse = GetPool();

	VkDescriptorSetAllocateInfo allocInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = pNext,
		.descriptorPool = poolToUse,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};

	VkDescriptorSet ds;
	VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, &ds);

	//allocation failed. Try again
	if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
	{
		m_fullPools.emplace_back(poolToUse);

		poolToUse = GetPool();
		allocInfo.descriptorPool = poolToUse;

		if (result = vkAllocateDescriptorSets(m_device, &allocInfo, &ds);
			result != VK_SUCCESS)
		{
			throw runtime_error(std::format(
				"Failed to allocate Descriptor Set! Error Code: {}", static_cast<uint32>(result))
			);
		}
	}

	m_readyPools.emplace_back(poolToUse);
	return ds;
}

VkDescriptorPool DescriptorPool::GetPool()
{
	VkDescriptorPool newPool;
	if (!m_readyPools.empty())
	{
		newPool = m_readyPools.back();
		m_readyPools.pop_back();

		return newPool;
	}

	newPool = CreatePool(m_setsPerPool, m_ratios);
	m_setsPerPool = std::min(static_cast<uint32>(m_setsPerPool * 1.5), MAX_SETS_PER_POOL);

	return newPool;
}

VkDescriptorPool DescriptorPool::CreatePool(const uint32 setCount, const span<PoolSizeRatio> poolRatios) const
{
	vector<VkDescriptorPoolSize> poolSizes;
	for (auto [type, ratio] : poolRatios)
	{
		poolSizes.push_back({
			.type = type,
			.descriptorCount = static_cast<uint32>(ratio) * setCount
			});
	}

	const VkDescriptorPoolCreateInfo poolInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.maxSets = setCount,
		.poolSizeCount = static_cast<uint32>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	VkDescriptorPool newPool;
	if (const VkResult result = vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &newPool))
	{
		throw runtime_error(std::format(
			"Failed to create Descriptor Pool! Error Code: {}", static_cast<uint32>(result))
		);
	}

	return newPool;
}
