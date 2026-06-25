#pragma once

#include <string>

class Application;
class Config;
struct GLFWwindow;
struct GraphicsPipelineConfig;
class Material;
class Mesh;
class Vulkan;

using std::string;

class Renderer
{
	friend Application;

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
	
private:
	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

	void InitVulkan(Config* config, GLFWwindow* window) const;
	void DestroyVulkan() const;

};

