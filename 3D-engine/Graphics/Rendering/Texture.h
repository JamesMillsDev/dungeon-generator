#pragma once

#include <string>
#include <vulkan/vulkan.h>

using std::string;

class Buffer;
struct DescriptorWriter;
class Vulkan;

class Texture
{
	friend class Material;
	friend class Renderer;
	friend class Vulkan;

private:
	string m_file;

	VkImage m_texture;
	VkImageView m_textureView;
	VkSampler m_textureSampler;
	VkDeviceMemory m_textureMemory;
	Buffer* m_stagingBuffer;

	uint32 m_width;
	uint32 m_height;

public:
	explicit Texture(string file);

private:
	void CreateBuffer();
	void DestroyBuffer();

	void Write(uint32 slot, DescriptorWriter& writer) const;

	void CopyBufferToImage() const;

};