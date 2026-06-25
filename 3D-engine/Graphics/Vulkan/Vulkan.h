#pragma once

#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Utility/ResourceStack.h"

class Texture;
class VulkanBuffer;
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

class Vulkan  // NOLINT(cppcoreguidelines-special-member-functions)
{
	friend Renderer;

private:
	static Vulkan* m_instance;

public:
	[[nodiscard]] static Vulkan* Instance();
	[[nodiscard]] static const VkDevice& Device();
	[[nodiscard]] static const VmaAllocator& Allocator();
	[[nodiscard]] static bool IsLoaded();
	[[nodiscard]] static runtime_error VulkanError(const string& message, VkResult result);

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
	uint32 m_queueFamily;

	VkSurfaceKHR m_surface;

	VkSwapchainKHR m_swapChain;
	vector<VkImage> m_swapChainImages;
	vector<VkImageView> m_swapChainImageViews;

	VkImage m_depthImage;
	VmaAllocation m_depthImageAllocation;
	VkImageView m_depthImageView;

	array<vector<VulkanBuffer*>, MAX_FRAMES_IN_FLIGHT> m_shaderDataBuffers;

	array<VkFence, MAX_FRAMES_IN_FLIGHT> m_fences;
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAcquiredSemaphores;
	vector<VkSemaphore> m_renderCompleteSemaphores;

	VkCommandPool m_commandPool;
	array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

	array<VulkanBuffer*, MAX_FRAMES_IN_FLIGHT> m_materialBuffer;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;
	vector<Texture*> m_textures;

private:
	explicit Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

public:
	[[nodiscard]] const VkDevice& GetDevice() const;
	[[nodiscard]] const VmaAllocator& GetAllocator() const;

	void BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const;
	void EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const;

	[[nodiscard]] VulkanBuffer* GetMaterialBuffer() const;

	void AddTexture(Texture* texture);
	void RemoveTexture(Texture* texture);

	void WriteTextureDescriptorSets() const;

private:
	void Init(GLFWwindow* window);
	void RecreateSwapChain();

	void InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const;

};
