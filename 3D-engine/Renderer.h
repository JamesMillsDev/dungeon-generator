#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

class Config;
struct GraphicsPipelineConfig;
class GraphicsPipeline;
class Mesh;
class Texture;
class Vulkan;

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

private:
	explicit Renderer(GLFWwindow* window, Config* config);
	~Renderer();

public:
	void Render(const Mesh* mesh, const GraphicsPipeline* pipeline) const;
	GraphicsPipeline* CreatePipeline(const GraphicsPipelineConfig& config) const;

private:
	void Create() const;
	void Destroy() const;

	[[nodiscard]] bool IsValid() const;

	void BeginFrame();
	void EndFrame() const;

	void WaitDeviceIdle() const;

};

