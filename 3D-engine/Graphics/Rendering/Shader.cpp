#include "pch.h"
#include "Shader.h"

#include <format>
#include <fstream>
#include <stdexcept>

#include "Graphics/Vulkan/Vulkan.h"

using std::ifstream;
using std::runtime_error;
using std::streamsize;

vector<char> Shader::ReadShaderFile(const string& fileName)
{
	ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw runtime_error("Failed to open file!");
	}

	const streamsize fileSize = file.tellg();
	vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

Shader::Shader(string path)
	: m_path{ std::move(path) }, m_shaderModule{ VK_NULL_HANDLE }
{
	Init(Vulkan::Instance());
}

Shader::Shader(string path, const Vulkan* vulkan)
	: m_path{ std::move(path) }, m_shaderModule{ VK_NULL_HANDLE }
{
	Init(vulkan);
}

Shader::~Shader()
{
	Destroy();
}

const VkShaderModule& Shader::GetShaderModule() const
{
	return m_shaderModule;
}

void Shader::Init(const Vulkan* vulkan)
{
	const string path = "./Content/" + m_path + ".slang.spv";

	const vector<char> code = ReadShaderFile(path);

	// Create the vulkan shader module
	const VkShaderModuleCreateInfo smCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = code.size(),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};
	if (const VkResult result = vkCreateShaderModule(vulkan->GetDevice(), &smCreateInfo, nullptr, &m_shaderModule);
		result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError("Failed to create Shader Module!", result);
	}
}

void Shader::Destroy()
{
	vkDestroyShaderModule(Vulkan::Device(), m_shaderModule, nullptr);
	m_shaderModule = VK_NULL_HANDLE;
}