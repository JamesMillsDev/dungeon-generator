#pragma once

#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "Maths/Color.h"
#include "Utility/ResourceStack.h"

class Config;
struct GLFWwindow;
class Renderer;
class Texture;
class Version;
class VulkanBuffer;
class VulkanDynamicDescriptorAllocator;
class VulkanDescriptorWriter;
class VulkanGraphicsPipeline;

using InitFunction = std::function<void()>;

using std::array;
using std::runtime_error;
using std::string;
using std::unordered_map;
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
	[[nodiscard]] static const TYPE& NAME(); \
	[[nodiscard]] const TYPE& Get##NAME() const; \

struct UniformBufferData
{
	uint32 count;
	VkDeviceSize size;
	uint16 id;
};

enum class EUniformBufferIds : uint16
{
	ProjectionView = 0,
	SceneLighting = 1,
	Lights = 2,
	Material = 3,
	PushConstant = UINT16_MAX
};

using UniformBufferSet = unordered_map<uint16, vector<VulkanBuffer*>>;

class Vulkan  // NOLINT(cppcoreguidelines-special-member-functions)
{
	friend Renderer;
	friend void CheckSwapChain(VkResult result, const string& errorMsg);

private:
	static Vulkan* m_instance;

public:
	[[nodiscard]] static Vulkan* Instance();
	DEFINE_ACCESSOR(VkDevice, Device)
	DEFINE_ACCESSOR(VmaAllocator, Allocator)
	DEFINE_ACCESSOR(VkDescriptorSetLayout, DescriptorSetLayout)
	DEFINE_ACCESSOR(VkDescriptorSet, DescriptorSet)

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

	array<UniformBufferSet, MAX_FRAMES_IN_FLIGHT> m_shaderDataBuffers;

	array<VkFence, MAX_FRAMES_IN_FLIGHT> m_fences;
	array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAcquiredSemaphores;
	vector<VkSemaphore> m_renderCompleteSemaphores;

	VkCommandPool m_commandPool;
	array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

	VulkanDynamicDescriptorAllocator* m_descriptorAllocator;
	VulkanDescriptorWriter* m_descriptorWriter;
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;
	vector<Texture*> m_textures;

	uint32 m_frameIndex;
	uint32 m_imageIndex;
	bool m_recreateSwapChain;
	bool m_updateTextureDescriptors;
	uint32 m_descriptorBindingIndex = 0;

private:
	explicit Vulkan(Config* config, GLFWwindow* window);
	~Vulkan();

public:
	void BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const;
	void EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const;

	[[nodiscard]] VulkanBuffer* GetUniformBuffer(uint16 id, uint32 index = 0) const;
	[[nodiscard]] VulkanBuffer* GetUniformBuffer(EUniformBufferIds id, uint32 index = 0) const;

	void AddTexture(Texture* texture);
	void RemoveTexture(Texture* texture);

	void WriteTextureDescriptorSets();
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
