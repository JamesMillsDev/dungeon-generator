#pragma once

class Config;
struct GLFWwindow;
class Vulkan;

class Renderer
{
	friend class Application;

private:
	Vulkan* m_vulkan;

private:
	explicit Renderer(GLFWwindow* window, Config* config);
	~Renderer();

private:
	void Create() const;
	void Destroy() const;

	[[nodiscard]] bool IsValid() const;

	void RenderFrame() const;
	void WaitDeviceIdle() const;

};

