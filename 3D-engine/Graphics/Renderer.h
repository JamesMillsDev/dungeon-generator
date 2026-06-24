#pragma once

#include <string>

#include "Rendering/Material.h"

struct GLFWwindow;

class Config;
struct GraphicsPipelineConfig;
class Material;
class Mesh;
class Vulkan;

using std::string;

class Renderer
{
	friend class Application;

private:
	static Renderer* m_instance;

public:
	static Renderer* GetInstance();

private:
	Vulkan* m_vulkan;

private:
	explicit Renderer(Config* config, GLFWwindow* window);
	~Renderer();

public:
	void Render(const Mesh* mesh, Material* material, const Matrix4& transform);
	[[nodiscard]] Material* CreateMaterial(const GraphicsPipelineConfig& config, EMaterialPass pass, uint32 textureCount = 0) const;
	[[nodiscard]] Material* CreateMaterial(const string& shaderName, EMaterialPass pass, uint32 textureCount = 0) const;

private:
	[[nodiscard]] bool IsValid() const;
	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

};

