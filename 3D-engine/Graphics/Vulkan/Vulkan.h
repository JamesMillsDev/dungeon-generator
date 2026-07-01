#pragma once

#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"
#include "Utility/ResourceStack.h"

class VulkanGraphicsPipeline;
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

#ifdef _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif

const vector VALIDATION_LAYERS =
{
	"VK_LAYER_KHRONOS_validation"
};

#define DEFINE_ACCESSOR(TYPE, NAME) \
	[[nodiscard]] static const TYPE& ##NAME(); \
	[[nodiscard]] const TYPE& Get##NAME() const; \

class Vulkan  // NOLINT(cppcoreguidelines-special-member-functions)
{
	friend class Application;
	friend Renderer;
	friend void CheckSwapChain(VkResult result, const string& errorMsg);

private:
	static Vulkan* m_instance;

public:
	[[nodiscard]] static Vulkan* Instance();
	DEFINE_ACCESSOR(VkDevice, Device)
	DEFINE_ACCESSOR(VmaAllocator, Allocator)
	DEFINE_ACCESSOR(VkDescriptorSetLayout, DescriptorSetLayout)
	DEFINE_ACCESSOR(VkDescriptorSet, TextureDescriptorSets)

	[[nodiscard]] static bool IsLoaded();
	[[nodiscard]] static runtime_error VulkanError(const string& message, VkResult result);

private:
	static void Create(Config* config, GLFWwindow* window);
	static void Destroy();

	static bool CheckValidationLayerSupport();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

private:
	Version* m_appVersion;
	string m_appName;
	Version* m_engineVersion;
	string m_engineName;
	Color m_clearColor;

	ResourceStack* m_resourceStack;
	bool m_loaded;

	VmaAllocator m_vmaAllocator;
	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

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

	array<VulkanBuffer*, MAX_FRAMES_IN_FLIGHT> m_uboBuffers;
	array<VulkanBuffer*, MAX_FRAMES_IN_FLIGHT> m_lightBuffers;
	array<VulkanBuffer*, MAX_FRAMES_IN_FLIGHT> m_materialBuffers;
	array<VulkanBuffer*, MAX_FRAMES_IN_FLIGHT> m_pushConstantBuffers;

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;
	vector<Texture*> m_textures;

	uint32 m_frameIndex;
	uint32 m_imageIndex;
	bool m_recreateSwapChain;
	bool m_isShuttingDown;
	uint32 m_maxDescriptorBinding;

private:
	explicit Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

public:
	void BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const;
	void EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const;

	[[nodiscard]] VulkanBuffer* GetUboBuffer() const;
	[[nodiscard]] VulkanBuffer* GetLightBuffer() const;
	[[nodiscard]] VulkanBuffer* GetMaterialBuffer() const;
	[[nodiscard]] VulkanBuffer* GetPushConstantBuffer() const;

	void AddTexture(Texture* texture);
	void RemoveTexture(Texture* texture);

	void WriteTextureDescriptorSets() const;
	void BindTextureDescriptorSets(VkCommandBuffer cmdBuf, VkPipelineLayout layout) const;
	VkFormat GetDepthFormat() const;

private:
	void Init(GLFWwindow* window);
	void RecreateSwapChain();

	VkCommandBuffer BeginFrame();
	void EndFrame(VkCommandBuffer cmdBuffer);

	void TransitionFrameImages(VkCommandBuffer cmdBuffer) const;
	void CreateDepthImage(const VkExtent3D& extent, const VkFormat& format);

	void InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const;

};
