#include "pch.h"
#include "Texture.h"

#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using std::runtime_error;

Texture::Texture(string file) :
	m_file{ std::move(file) }, m_image{ VK_NULL_HANDLE }, m_imageView{ VK_NULL_HANDLE },
	m_imageExtent{}, m_imageFormat{ VK_FORMAT_R32G32B32A32_SFLOAT }, m_pixels{ nullptr }
{
	CreateBuffer();
}

Texture::~Texture()
{
	DestroyBuffer();
}

void Texture::CreateBuffer()
{
	

	// Attempt to load the image using stbi TODO: Support more than one format
	int w, h, channels;
	m_pixels = stbi_load(m_file.c_str(), &w, &h, &channels, STBI_rgb_alpha);

	// Throw an error if it fails
	if (m_pixels == nullptr)
	{
		throw runtime_error("Failed to load texture!");
	}

	m_imageExtent.width = w;
	m_imageExtent.height = h;
	m_imageExtent.depth = 1;
}

void Texture::DestroyBuffer() const
{
	stbi_image_free(m_pixels);

	
}