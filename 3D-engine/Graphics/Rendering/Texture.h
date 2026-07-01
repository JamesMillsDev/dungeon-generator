#pragma once

#include <ktx.h>
#include <queue>
#include <string>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Maths/Alias.h"

class VulkanBuffer;

using std::queue;
using std::string;

class Texture
{
	friend class Material;
	friend class Renderer;

private:
	static uint32 m_nextId;
	static queue<uint32> m_freeIds;

private:
	string m_file;

	VkImage m_image;
	VmaAllocation m_imageAllocation;
	VkImageView m_imageView;
	VkSampler m_sampler;

	VkExtent3D m_imageExtent;
	VkFormat m_imageFormat;

	VulkanBuffer* m_buffer;
	ktxTexture2* m_texture;

	VkDescriptorImageInfo m_textureDescriptors;

	uint32 m_id;

public:
	explicit Texture(string file);
	~Texture();

public:
	[[nodiscard]] const VkDescriptorImageInfo& GetDescriptors() const;
	[[nodiscard]] uint32 GetId() const;

private:
	void CreateBuffer();
	void DestroyBuffer() const;

	void TransitionImage() const;

};