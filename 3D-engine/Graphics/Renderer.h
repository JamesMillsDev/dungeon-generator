#pragma once

#include <string>

#include "Application.h"
#include "Rendering/Material.h"

class Config;
struct GLFWwindow;
struct GraphicsPipelineConfig;
class Material;
class Mesh;
class Vulkan;

using std::string;

class Renderer
{
	friend void Application::InitRenderer() const;
	friend void Application::DestroyRenderer() const;
	friend EExitCode Application::Run() const;

private:
	static Renderer* m_instance;

public:
	static Renderer* Instance();
	[[nodiscard]] static bool IsValid();

private:
	static void Create(Config* config, GLFWwindow* window);
	static void Destroy();

private:
	explicit Renderer(Config* config, GLFWwindow* window);
	~Renderer();

public:
	void Render(const Mesh* mesh, Material* material, const Matrix4& transform);
	[[nodiscard]] Material* CreateMaterial(const GraphicsPipelineConfig& config, EMaterialPass pass, uint32 textureCount = 0) const;
	[[nodiscard]] Material* CreateMaterial(const string& shaderName, EMaterialPass pass, uint32 textureCount = 0) const;


private:
	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

	void InitVulkan(Config* config, GLFWwindow* window) const;
	void DestroyVulkan() const;

};

