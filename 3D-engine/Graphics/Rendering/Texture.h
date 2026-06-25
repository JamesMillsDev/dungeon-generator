#pragma once

#include <ktx.h>
#include <string>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

class VulkanBuffer;

using std::string;

class Texture
{
	friend class Material;
	friend class Renderer;

private:
	string m_file;

	VkImage m_image;
	VmaAllocation m_imageAllocation;
	VkImageView m_imageView;
	VkSampler m_sampler;

	VkExtent3D m_imageExtent;
	VkFormat m_imageFormat;

	VulkanBuffer* m_buffer;
	ktxTexture* m_texture;

	VkDescriptorImageInfo m_textureDescriptors;

public:
	explicit Texture(string file);
	~Texture();

public:
	const VkDescriptorImageInfo& GetDescriptors() const;

private:
	void CreateBuffer();
	void DestroyBuffer() const;

	void TransitionImage() const;

};