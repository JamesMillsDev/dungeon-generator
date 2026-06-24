#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "Graphics/Rendering/Uniforms.h"
#include "Rendering/Material.h"

struct GLFWwindow;

class Config;
struct GraphicsPipelineConfig;
class Material;
class Mesh;
class Texture;

using std::string;

class Renderer
{
	friend class Application;

private:
	static Renderer* m_instance;

public:
	static Renderer* GetInstance();

private:
	VkCommandBuffer m_frameCommandBuffer;
	UniformBuffer* m_mvpBuffer;
	ProjectionViewUniform m_uniformBufferObj;

private:
	explicit Renderer(GLFWwindow* window, Config* config);
	~Renderer();

public:
	void Render(const Mesh* mesh, Material* material, const Matrix4& transform);
	[[nodiscard]] Material* CreateMaterial(const GraphicsPipelineConfig& config, EMaterialPass pass, uint32 textureCount = 0) const;
	[[nodiscard]] Material* CreateMaterial(const string& shaderName, EMaterialPass pass, uint32 textureCount = 0) const;

private:
	void Create();
	void Destroy();

	[[nodiscard]] bool IsValid() const;

	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

};

