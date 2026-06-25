#include "pch.h"
#include "Shader.h"

#include "Graphics/Vulkan/Vulkan.h"

#define SPIR_V_PROFILE "spirv_1_4"

using slang::CompilerOptionEntry;
using slang::CompilerOptionName;
using slang::CompilerOptionValueKind;
using slang::SessionDesc;
using slang::TargetDesc;

Shader::Shader(string path)
	: m_path{ std::move(path) }, m_shaderModule{ VK_NULL_HANDLE }
{
	Init();
}

Shader::~Shader()
{
	Destroy();
}

void Shader::Init()
{
	string path = "./Content/" + m_path + ".slang";

	const ComPtr<IGlobalSession>& globalSession = Vulkan::SlangSession();

	auto slangTargets = std::to_array<TargetDesc>(
		{
			{
				.format = SLANG_SPIRV,
				.profile = globalSession->findProfile(SPIR_V_PROFILE)
			}
		}
	);

	auto slangOptions = std::to_array<CompilerOptionEntry>(
		{
			{
				.name = CompilerOptionName::EmitSpirvDirectly,
				.value = 
				{
					.kind = CompilerOptionValueKind::Int,
					.intValue0 = 1,
				}
			}
		}
	);

	// Create the shader session
	const SessionDesc slangSessionDesc
	{
		.targets = slangTargets.data(),
		.targetCount = static_cast<SlangInt>(slangTargets.size()),
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
		.compilerOptionEntries = slangOptions.data(),
		.compilerOptionEntryCount = static_cast<uint32>(slangOptions.size())
	};
	globalSession->createSession(slangSessionDesc, m_session.writeRef());

	// Create the slang module and get the code from it
	m_module = m_session->loadModuleFromSource("triangle", path.c_str(), nullptr, nullptr);
	m_module->getTargetCode(0, m_source.writeRef());

	// Create the vulkan shader module
	const VkShaderModuleCreateInfo smCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = m_source->getBufferSize(),
		.pCode = static_cast<const uint32_t*>(m_source->getBufferPointer())
	};
	if (const VkResult result = vkCreateShaderModule(Vulkan::Device(), &smCreateInfo, nullptr, &m_shaderModule);
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