#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

class Config;
class Mesh;
class Vulkan;

class Renderer
{
	friend class Application;

private:
	Vulkan* m_vulkan;
	VkCommandBuffer m_frameCommandBuffer;

private:
	explicit Renderer(GLFWwindow* window, Config* config);
	~Renderer();

public:
	void RenderMesh(const Mesh* mesh) const;

private:
	void Create() const;
	void Destroy() const;

	[[nodiscard]] bool IsValid() const;

	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

};

