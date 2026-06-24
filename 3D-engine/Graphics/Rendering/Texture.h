#pragma once

#include <string>
#include <vulkan/vulkan.h>

using std::string;

class Texture
{
	friend class Material;
	friend class Renderer;

private:
	string m_file;

	VkImage m_image;
	VkImageView m_imageView;
	VkExtent3D m_imageExtent;
	VkFormat m_imageFormat;

	uint8* m_pixels;

public:
	explicit Texture(string file);
	~Texture();

private:
	void CreateBuffer();
	void DestroyBuffer() const;

};