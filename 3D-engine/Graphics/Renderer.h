#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Graphics/Rendering/Uniforms.h"

struct GLFWwindow;

class Config;
class DescriptorPool;
struct GraphicsPipelineConfig;
class Material;
class Mesh;
class Texture;
class UniformBuffer;
class Vulkan;

using std::string;

class Renderer
{
	friend class Application;

private:
	static Renderer* m_instance;

public:
	static Renderer* GetInstance();

public:
	static void Load(Mesh* mesh);
	static void Unload(Mesh*& mesh);

	static void Load(Texture* texture);
	static void Unload(Texture*& texture);

private:
	Vulkan* m_vulkan;
	VkCommandBuffer m_frameCommandBuffer;
	UniformBuffer* m_mvpBuffer;
	UniformBufferObject m_uniformBufferObj;

	Texture* m_texture;
	DescriptorPool* m_descriptorPool;

private:
	explicit Renderer(GLFWwindow* window, Config* config);
	~Renderer();

public:
	void Render(const Mesh* mesh, Material* material, const Matrix4& transform);
	[[nodiscard]] Material* CreateMaterial(const GraphicsPipelineConfig& config) const;
	[[nodiscard]] Material* CreateMaterial(const string& shaderName) const;

private:
	void Create();
	void Destroy();

	[[nodiscard]] bool IsValid() const;

	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

};

