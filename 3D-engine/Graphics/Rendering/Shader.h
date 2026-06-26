#pragma once

#include <vulkan/vulkan.h>

class Vulkan;

class Shader
{
private:
	static vector<char> ReadShaderFile(const string& fileName);

private:
	string m_path;

	VkShaderModule m_shaderModule;

public:
	explicit Shader(string path);
	~Shader();

public:
	[[nodiscard]] const VkShaderModule& GetShaderModule() const;

private:
	void Init(const Vulkan* vulkan);
	void Destroy();

};