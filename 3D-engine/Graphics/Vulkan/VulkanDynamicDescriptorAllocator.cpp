#include "pch.h"
#include "VulkanDynamicDescriptorAllocator.h"

#include "Vulkan.h"

VkDescriptorPool VulkanDynamicDescriptorAllocator::CreatePool(const uint32 setCount,
	const span<PoolSizeRatio>& poolRatios, const Vulkan* vulkan)
{
	vector<VkDescriptorPoolSize> poolSizes;
	for (const auto& [type, ratio] : poolRatios)
	{
		poolSizes.emplace_back(type, static_cast<uint32>(ratio) * setCount);
	}

	const VkDescriptorPoolCreateInfo createInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = setCount,
		.poolSizeCount = static_cast<uint32>(poolSizes.size()),
		.pPoolSizes = poolSizes.data()
	};

	VkDescriptorPool newPool;
	if (const VkResult result = vkCreateDescriptorPool(vulkan->GetDevice(), &createInfo, nullptr, &newPool);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Descriptor Pool!", result);
	}

	return newPool;
}

VulkanDynamicDescriptorAllocator::VulkanDynamicDescriptorAllocator(const uint32 initialSets,
	const span<PoolSizeRatio>& poolRatios, Vulkan* vulkan)
	: m_setsPerPool{ 0 }
{
	Init(initialSets, poolRatios, vulkan);
}

VulkanDynamicDescriptorAllocator::~VulkanDynamicDescriptorAllocator()
{
	Destroy();
}

VkDescriptorSet VulkanDynamicDescriptorAllocator::Allocate(VkDescriptorSetLayout layout, void* pNext, Vulkan* vulkan)
{
	if (vulkan == nullptr)
	{
		vulkan = Vulkan::Instance();
	}

	VkDescriptorPool poolToUse = GetPool(vulkan);

	VkDescriptorSetAllocateInfo allocateInfo
	{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = pNext,
		.descriptorPool = poolToUse,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout
	};

	const VkDevice device = vulkan->GetDevice();
	VkDescriptorSet ds;

	if (VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, &ds);
		result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
	{
		m_fullPools.emplace_back(poolToUse);

		poolToUse = GetPool(vulkan);
		allocateInfo.descriptorPool = poolToUse;

		if (result = vkAllocateDescriptorSets(device, &allocateInfo, &ds);
			result != VK_SUCCESS)
		{
			throw Vulkan::VulkanError("Failed to create Allocate Descriptor Set!", result);
		}
	}

	m_readyPools.emplace_back(poolToUse);
	return ds;
}

void VulkanDynamicDescriptorAllocator::Clear()
{
	const VkDevice device = Vulkan::Device();

	for (const VkDescriptorPool& pool : m_readyPools)
	{
		vkResetDescriptorPool(device, pool, 0);
	}

	for (const VkDescriptorPool& pool : m_fullPools)
	{
		vkResetDescriptorPool(device, pool, 0);
		m_readyPools.emplace_back(pool);
	}

	m_fullPools.clear();
}

void VulkanDynamicDescriptorAllocator::Init(uint32 initialSets, const span<PoolSizeRatio>& poolRatios, const Vulkan* vulkan)
{
	m_ratios.clear();

	m_ratios.reserve(poolRatios.size());
	for (const PoolSizeRatio& ratio : poolRatios)
	{
		m_ratios.emplace_back(ratio);
	}

	VkDescriptorPool newPool = CreatePool(initialSets, poolRatios, vulkan);

	m_setsPerPool = static_cast<uint32>(static_cast<float>(initialSets) * 1.5f);

	m_readyPools.emplace_back(newPool);
}

void VulkanDynamicDescriptorAllocator::Destroy()
{
	const VkDevice device = Vulkan::Device();

	for (const VkDescriptorPool& pool : m_readyPools)
	{
		vkDestroyDescriptorPool(device, pool, nullptr);
	}

	for (const VkDescriptorPool& pool : m_fullPools)
	{
		vkDestroyDescriptorPool(device, pool, nullptr);
	}

	m_readyPools.clear();
	m_fullPools.clear();
}

VkDescriptorPool VulkanDynamicDescriptorAllocator::GetPool(const Vulkan* vulkan)
{
	VkDescriptorPool newPool;
	if (!m_readyPools.empty())
	{
		newPool = m_readyPools.back();
		m_readyPools.pop_back();
	}
	else
	{
		newPool = CreatePool(m_setsPerPool, m_ratios, vulkan);

		m_setsPerPool = static_cast<uint32>(static_cast<float>(m_setsPerPool) * 1.5f);
		m_setsPerPool = std::max(m_setsPerPool, 4092u);
	}

	return newPool;
}
