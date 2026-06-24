#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include <vma/vk_mem_alloc.h>

#include "Utility/ResourceStack.h"

struct GLFWwindow;
class Config;
class Version;

using InitFunction = std::function<void()>;
using std::runtime_error;
using std::string;
using std::vector;

class Vulkan
{
	friend class Renderer;

private:
	static runtime_error VulkanError(const string& message, VkResult result);

private:
	Version* m_appVersion;
	string m_appName;
	Version* m_engineVersion;
	string m_engineName;

	ResourceStack* m_resourceStack;
	bool m_loaded;

	VmaAllocator m_vmaAllocator;
	VkInstance m_instance;

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

private:
	explicit Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

private:
	void Init(GLFWwindow* window);
	void CleanupSwapChain();

	[[nodiscard]] bool IsLoaded() const;

	void InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const;

};
