#include "pch.h"
#include "Texture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "Graphics/VulkanHelpers/Buffer.h"
#include "Graphics/VulkanHelpers/Vulkan.h"

using std::runtime_error;

Texture::Texture(string file) :
	m_file{ std::move(file) }, m_texture{ VK_NULL_HANDLE }, m_textureView{ VK_NULL_HANDLE },
	m_textureSampler{ VK_NULL_HANDLE }, m_textureMemory{ VK_NULL_HANDLE },
	m_stagingBuffer{ nullptr }, m_width{ 0 }, m_height{ 0 }
{}

void Texture::CreateBuffer()
{
	Vulkan* vulkan = Vulkan::Instance();

	// Attempt to load the image using stbi TODO: Support more than one format
	int w, h, channels;
	stbi_uc* pixels = stbi_load(m_file.c_str(), &w, &h, &channels, STBI_rgb_alpha);

	m_width = w;
	m_height = h;

	// Throw an error if it fails
	if (pixels == nullptr)
	{
		throw runtime_error("Failed to load texture!");
	}

	// Fill the staging buffer and clean up the pixel memory
	m_stagingBuffer = vulkan->MakeStagingBuffer((size_t)(m_width * m_height * 4), 1);
	m_stagingBuffer->Fill(pixels);
	stbi_image_free(pixels);

	// Create the texture handle in vulkan
	m_texture = vulkan->MakeTexture(
		static_cast<uint32>(w), static_cast<uint32>(h), VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureMemory
	);

	// Copy the buffer into the image by transitioning the format
	vulkan->TransitionImageLayout(
		m_texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);
	CopyBufferToImage();
	vulkan->TransitionImageLayout(
		m_texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// Create the image view and the sampler
	m_textureView = vulkan->MakeImageViewFor(m_texture, VK_FORMAT_R8G8B8A8_SRGB);
	m_textureSampler = vulkan->MakeTextureSampler();
}

void Texture::DestroyBuffer()
{
	Vulkan* vulkan = Vulkan::Instance();

	vulkan->TransitionImageLayout(
		m_texture, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	Vulkan::DestroyBuffer(m_stagingBuffer);

	vulkan->DestroyTexture(m_texture, m_textureMemory);
	vulkan->DestroyTextureView(m_textureView);
	vulkan->DestroyTextureSampler(m_textureSampler);
}

void Texture::Render(VkCommandBuffer buffer)
{

}

void Texture::CopyBufferToImage() const
{
	const Vulkan* vulkan = Vulkan::Instance();

	const VkCommandBuffer commandBuffer = vulkan->BeginSingleTimeCommands();

	const VkBufferImageCopy region
	{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,

		.imageSubresource =
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},

		.imageOffset = {.x = 0, .y = 0, .z = 0 },
		.imageExtent = {.width = m_width, .height = m_height, .depth = 1 }
	};

	vkCmdCopyBufferToImage(
		commandBuffer, m_stagingBuffer->Get(), m_texture,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region
	);

	vulkan->EndSingleTimeCommands(commandBuffer);
}
