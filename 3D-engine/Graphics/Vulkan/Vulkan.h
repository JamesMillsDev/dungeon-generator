#pragma once

#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Utility/ResourceStack.h"

struct GLFWwindow;
class Config;
class Renderer;
class Version;

using InitFunction = std::function<void()>;

using std::array;
using std::runtime_error;
using std::string;
using std::vector;

constexpr int32 MAX_FRAMES_IN_FLIGHT = 2;

class Vulkan
{
	friend Renderer;

private:
	static Vulkan* m_instance;

public:
	static Vulkan* Instance();
	static const VkDevice& Device();
	static const VmaAllocator& Allocator();
	static bool IsLoaded();
	static runtime_error VulkanError(const string& message, VkResult result);

private:
	static void Create(Config* config, GLFWwindow* window);
	static void Destroy();

private:
	Version* m_appVersion;
	string m_appName;
	Version* m_engineVersion;
	string m_engineName;

	ResourceStack* m_resourceStack;
	bool m_loaded;

	VmaAllocator m_vmaAllocator;
	VkInstance m_vkInstance;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkQueue m_queue;

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	vector<VkImage> m_swapChainImages;
	vector<VkImageView> m_swapChainImageViews;

	VkImage m_depthImage;
	VmaAllocation m_depthImageAllocation;
	VkImageView m_depthImageView;

	array<VkBuffer**, MAX_FRAMES_IN_FLIGHT> m_commonShaderDataBuffer;
	array<VkCommandBuffer*, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

private:
	explicit Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

private:
	void Init(GLFWwindow* window);
	void CleanupSwapChain();

	void InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const;

};
