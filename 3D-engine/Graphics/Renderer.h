#pragma once

#include "Graphics/Vulkan/Uniforms.h"

#include <string>
#include <vulkan/vulkan.h>

class CameraComponent;
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
	static CameraComponent* m_currentCamera;

public:
	static Renderer* Instance();
	[[nodiscard]] static bool IsValid();

	static CameraComponent* GetCurrentCamera();
	static void SetCurrent(CameraComponent* newCurrent);

private:
	static void Create(Config* config, GLFWwindow* window);
	static void Destroy();

	static void InitVulkan(Config* config, GLFWwindow* window);
	static void DestroyVulkan();

	static void WaitIdle();

private:
	VkCommandBuffer m_frameCmdBuf;
	Vulkan* m_vulkan;

private:
	explicit Renderer(Config* config, GLFWwindow* window);
	~Renderer();

public:
	void Render(const Mesh* mesh, const Material* material, const mat4& transform) const;
	
private:
	void BeginFrame();
	void EndFrame();

};

