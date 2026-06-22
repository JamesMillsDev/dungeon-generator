#pragma once

#include <functional>
#include <initializer_list>
#include <string>

#include "Common.h"
#include "GraphicsPipeline.h"
#include "Structs.h"
#include "Maths/Alias.h"

using std::function;
using std::initializer_list;
using std::string;

struct GLFWwindow;

class Buffer;
class Config;
class UniformBuffer;
class Version;

struct BufferInfo
{
	uint32 vertexCount;
	uint32 instanceCount;
	uint32 firstVertex = 0;
	uint32 firstInstance = 0;
};

class Vulkan
{
	friend class Renderer;

private:
	static Vulkan* m_singleton;

public:
	static Vulkan* Instance();
	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	static void DestroyBuffer(Buffer*& buffer);

private:
	static bool CheckValidationLayerSupport();
	static vector<const char*> GetRequiredExtensions();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static uint32 RateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
	static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
	static VkSurfaceFormatKHR ChooseSwapChainFormat(const vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresetModes);
	static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

private:
	string m_engineTitle;
	Version* m_engineVersion;
	Version* m_vulkanVersion;
	string m_appTitle;
	Version* m_appVersion;
	Color m_clearColor;

	GLFWwindow* m_window;
	bool m_loaded;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSwapchainKHR m_swapChain;
	vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainFormat;
	VkExtent2D m_swapChainExtent;
	vector<VkImageView> m_swapChainImageViews;
	vector<VkFramebuffer> m_swapChainFrameBuffers;

	VkRenderPass m_renderPass;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	vector<VkDescriptorSet> m_descriptorSets;

	VkPipelineLayout m_pipelineLayout;
	vector<VkPipeline> m_pipelines;

	VkCommandPool m_commandPool;
	vector<VkCommandBuffer> m_commandBuffers;

	vector<VkSemaphore> m_imageAvailableSemaphores;
	vector<VkSemaphore> m_renderFinishedSemaphores;
	vector<VkFence> m_inFlightFences;

	bool m_frameBufferResized;
	uint32 m_currentFrame;
	uint32 m_currentImageIndex;

	vector<UniformBuffer*> m_uniformBuffers;

private:
	Vulkan(GLFWwindow* window, Config* config);
	~Vulkan();

public:
	[[nodiscard]] Buffer* MakeVertexBuffer(size_t vertexCount) const;
	[[nodiscard]] Buffer* MakeIndexBuffer(size_t indexCount) const;
	[[nodiscard]] Buffer* MakeStagingBuffer(size_t size, size_t count) const;
	[[nodiscard]] UniformBuffer* MakeUniformBuffer(size_t size, size_t count) const;

	[[nodiscard]] VkImage MakeTexture(
		uint32 w, uint32 h, VkFormat format,
		VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
		VkDeviceMemory& imageMemory
	) const;
	void DestroyTexture(VkImage texture, VkDeviceMemory textureMemory) const;

	[[nodiscard]] VkImageView MakeImageViewFor(VkImage image, VkFormat format) const;
	void DestroyTextureView(VkImageView textureView) const;

	[[nodiscard]] VkSampler MakeTextureSampler(
		VkFilter filter = VK_FILTER_LINEAR,
		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		bool anisotropyEnabled = VK_TRUE, bool normalisedCoords = VK_TRUE,
		VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, 
		float mipLodBias = 0.f, float minLod = 0.f, float maxLod = 0.f,
		bool enableCompare = VK_FALSE, VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS,
		VkBorderColor borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK
	) const;
	void DestroyTextureSampler(VkSampler sampler) const;

	[[nodiscard]] VkCommandBuffer BeginSingleTimeCommands() const;
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;

private:
	[[nodiscard]] bool Loaded() const;

	void Create(const vector<GraphicsPipelineConfig>& shaderInfos);
	void Destroy();

	void CreateInstance();
	void SetupDebugMessenger();

	void CreateSurface();

	void PickPhysicalDevice();
	void CreateLogicalDevice();

	void RecreateSwapChain();
	void CleanupSwapChain() const;
	void CreateSwapChain();
	void CreateImageViews();

	void CreateRenderPass();

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline(const vector<GraphicsPipelineConfig>& pipelineConfigs);

	void CreateFrameBuffers();

	void CreateCommandPool();

	void CreateCommandBuffer();
	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex, const function<void()>& drawCommand, uint32 pipelineIndex) const;
	void UpdateUniformBuffer(uint32 imageIndex) const;

	void CreateSyncObjects();

	VkCommandBuffer BeginRender();
	void EndRender();

};