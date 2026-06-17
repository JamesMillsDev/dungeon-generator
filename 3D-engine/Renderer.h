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
	explicit Renderer(Config* config);
	~Renderer();

private:
	void Create(GLFWwindow* window) const;
	void Destroy() const;

	[[nodiscard]] bool IsValid() const;

};

