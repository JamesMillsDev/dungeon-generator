#pragma once

#include <slang/slang-com-ptr.h>
#include <slang/slang.h>
#include <vulkan/vulkan.h>

using slang::IModule;
using slang::ISession;
using Slang::ComPtr;

class Shader
{
private:
	string m_path;
	ComPtr<ISession> m_session;
	ComPtr<IModule> m_module;
	ComPtr<ISlangBlob> m_source;

	VkShaderModule m_shaderModule;

public:
	explicit Shader(string path);
	~Shader();

private:
	void Init();
	void Destroy();

};