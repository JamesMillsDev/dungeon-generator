#include "pch.h"
#include "Vulkan.h"

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "Application.h"
#include "Uniforms.h"
#include "VulkanBuffer.h"
#include "Window.h"
#include "Graphics/Rendering/Texture.h"
#include "Utility/Console.h"
#include "Utility/ResourceStack.h"

using std::exception;

constexpr uint32 MAX_TEXTURE_DESCRIPTORS = UINT16_MAX;
constexpr int32 DEFAULT_RESOURCE_STACK_SIZE = 16;
constexpr int32 UNIFORM_BUFFER_COUNT = 3;

namespace
{
	VkDebugUtilsMessageSeverityFlagBitsEXT messageLevel = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= messageLevel)
		{
			switch (messageSeverity)  // NOLINT(clang-diagnostic-switch-enum)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				{
					Console::Debug(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				{
					Console::Info(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				{
					Console::Warning(pCallbackData->pMessage);
					break;
				}
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				{
					Console::Error(pCallbackData->pMessage);
					break;
				}
				default: break;
			}
		}

		return VK_FALSE;
	}

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(  // NOLINT(clang-diagnostic-cast-function-type-strict)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
			);
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(  // NOLINT(clang-diagnostic-cast-function-type-strict)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
			);
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}
}

static void Try(const VkResult result, const string& errorMsg)  // NOLINT(misc-use-anonymous-namespace)
{
	if (result != VK_SUCCESS)
	{
		throw Vulkan::VulkanError(errorMsg, result);
	}
}

static void CheckSwapChain(const VkResult result, const string& errorMsg) // NOLINT(misc-use-anonymous-namespace, clang-diagnostic-microsoft-redeclare-static)
{
	if (result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Vulkan::Instance()->m_recreateSwapChain = true;
			return;
		}

		throw Vulkan::VulkanError(errorMsg, result);
	}
}

Vulkan* Vulkan::m_instance = nullptr;

Vulkan* Vulkan::Instance()
{
	return m_instance;
}

const VkDevice& Vulkan::Device()
{
	return m_instance->GetDevice();
}

const VkDevice& Vulkan::GetDevice() const
{
	return m_device;
}

const VmaAllocator& Vulkan::Allocator()
{
	return m_instance->GetAllocator();
}

const VmaAllocator& Vulkan::GetAllocator() const
{
	return m_vmaAllocator;
}

const VkDescriptorSetLayout& Vulkan::DescriptorSetLayout()
{
	return m_instance->GetDescriptorSetLayout();
}

const VkDescriptorSetLayout& Vulkan::GetDescriptorSetLayout() const
{
	return m_descriptorSetLayout;
}

const VkDescriptorSet& Vulkan::TextureDescriptorSets()
{
	return m_instance->GetTextureDescriptorSets();
}

const VkDescriptorSet& Vulkan::GetTextureDescriptorSets() const
{
	return m_descriptorSet;
}

bool Vulkan::IsLoaded()
{
	return m_instance != nullptr && m_instance->m_loaded;
}

runtime_error Vulkan::VulkanError(const string& message, const VkResult result)
{
	return runtime_error(std::format("{}. Error Code: {}", message, static_cast<int32>(result)));
}

void Vulkan::Create(Config* config, GLFWwindow* window)
{
	m_instance = new Vulkan{ config, window };
}

void Vulkan::Destroy()
{
	delete m_instance;
	m_instance = nullptr;
}

bool Vulkan::CheckValidationLayerSupport()
{
	// Count the number of available layers
	uint32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get the layers from the driver, storing them in a vector
	vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Iterate over the layers we want to use
	for (const char* layerName : VALIDATION_LAYERS)
	{
		bool layerFound = false;

		// Iterate over the layers that the driver supports
		for (const VkLayerProperties& layerProperties : availableLayers)
		{
			// Validate the names match for this property, if they do, mark it as found and break
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		// If the layer still hasn't been found, we are trying to enable a layer that is
		// unsupported
		if (!layerFound)
		{
			return false;
		}
	}

	// All requested layers are supported
	return true;
}

void Vulkan::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
}

Vulkan::Vulkan(Config* config, GLFWwindow* window)
	: m_resourceStack{ new ResourceStack{ DEFAULT_RESOURCE_STACK_SIZE } }, m_loaded{ false }, m_frameIndex{ 0 },
	m_imageIndex{ 0 }, m_recreateSwapChain{ false }
{
	m_appName = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };
	m_engineName = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };
	m_clearColor = config->Get<Color>("Window.ClrColor");

	Init(window);
}

Vulkan::~Vulkan()
{
	m_updateTextureDescriptors = true;

	delete m_resourceStack;
	delete m_appVersion;
	delete m_engineVersion;
}

void Vulkan::BeginOneTimeCommand(VkCommandBuffer& buffer, VkFence& fence) const
{
	VkResult result;

	// Attempt to create the one-time fence
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	if (result = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence);
		result != VK_SUCCESS)
	{
		throw VulkanError("Failed to create One-Time Fence!", result);
	}

	// Attempt to allocate one-time command buffer
	VkCommandBufferAllocateInfo cbAllocateInfo{};
	cbAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocateInfo.commandPool = m_commandPool;
	cbAllocateInfo.commandBufferCount = 1;

	if (result = vkAllocateCommandBuffers(m_device, &cbAllocateInfo, &buffer);
		result != VK_SUCCESS)
	{
		throw VulkanError("Failed to create One-Time Command Buffer!", result);
	}

	// Attempt to begin the command buffer
	VkCommandBufferBeginInfo cbBeginInfo{};
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (result = vkBeginCommandBuffer(buffer, &cbBeginInfo);
		result != VK_SUCCESS)
	{
		throw VulkanError("Failed to begin One-Time Command Buffer!", result);
	}
}

void Vulkan::EndOneTimeCommand(const VkCommandBuffer& buffer, const VkFence& fence) const
{
	VkResult result;

	// Attempt to end the command buffer
	if (result = vkEndCommandBuffer(buffer); result != VK_SUCCESS)
	{
		throw VulkanError("Failed to end One-Time Command Buffer!", result);
	}

	// Attempt to submit the queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	if (result = vkQueueSubmit(m_queue, 1, &submitInfo, fence); result != VK_SUCCESS)
	{
		throw VulkanError("Failed to submit One-Time Command!", result);
	}

	// Wait for the fences to finish
	if (result = vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX); result != VK_SUCCESS)
	{
		throw VulkanError("Fence timed out!", result);
	}

	vkDestroyFence(m_device, fence, nullptr);
}

VulkanBuffer* Vulkan::GetUboBuffer() const
{
	return m_uboBuffers[m_frameIndex];
}

VulkanBuffer* Vulkan::GetLightBuffer() const
{
	return m_lightBuffers[m_frameIndex];
}

VulkanBuffer* Vulkan::GetMaterialBuffer() const
{
	return m_materialBuffers[m_frameIndex];
}

VulkanBuffer* Vulkan::GetPushConstantBuffer() const
{
	return m_pushConstantBuffers[m_frameIndex];
}

void Vulkan::AddTexture(Texture* texture)
{
	m_textures.emplace_back(texture);

	m_updateTextureDescriptors = true;
}

void Vulkan::RemoveTexture(Texture* texture)
{
	std::erase_if(
		m_textures, [texture](const Texture* t)
		{
			return texture == t;
		}
	);

	m_updateTextureDescriptors = true;
}

void Vulkan::WriteTextureDescriptorSets()
{
	if (!m_updateTextureDescriptors)
	{
		return;
	}

	vector<VkDescriptorImageInfo> textureDescriptors(m_textures.size());
	for (uint64 i = 0; i < m_textures.size(); ++i)
	{
		textureDescriptors[i] = m_textures[i]->GetDescriptors();
	}

	const VkWriteDescriptorSet writeDescSet
	{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = m_descriptorSet,
		.dstBinding = m_maxDescriptorBinding - 1,
		.dstArrayElement = 0,
		.descriptorCount = static_cast<uint32>(textureDescriptors.size()),
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = textureDescriptors.data(),
		.pBufferInfo = nullptr,
		.pTexelBufferView = nullptr
	};
	vkUpdateDescriptorSets(m_device, 1, &writeDescSet, 0, nullptr);

	m_updateTextureDescriptors = false;
}

void Vulkan::BindTextureDescriptorSets(const VkCommandBuffer cmdBuf, const VkPipelineLayout layout) const
{
	vkCmdBindDescriptorSets(
		cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &m_descriptorSet, 0, nullptr
	);
}

VkFormat Vulkan::GetDepthFormat() const
{
	// Attempt to get the correct format for the swap chain images
	const vector depthFormatList = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	for (const VkFormat& format : depthFormatList)
	{
		// Get the device format properties
		VkFormatProperties2 formatProperties{};
		formatProperties.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;
		vkGetPhysicalDeviceFormatProperties2(m_physicalDevice, format, &formatProperties);

		// If this format properties contains the tiling features we want, store and break
		if (formatProperties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			depthFormat = format;
			break;
		}
	}

	return depthFormat;
}

void Vulkan::Init(GLFWwindow* window)
{
	try
	{
		// VK Instance
		InitAndPushResource(
			[this]
			{
				const VkApplicationInfo appInfo
				{
					.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
					.pNext = nullptr,
					.pApplicationName = m_appName.c_str(),
					.applicationVersion = VK_MAKE_VERSION(m_appVersion->major, m_appVersion->minor, m_appVersion->patch),
					.pEngineName = m_engineName.c_str(),
					.engineVersion = VK_MAKE_VERSION(m_engineVersion->major, m_engineVersion->minor, m_engineVersion->patch),
					.apiVersion = VK_API_VERSION_1_3
				};

				uint32 instanceExtensionsCount = 0;
				const char** instanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionsCount);

				vector<const char*> extensions;
				for (uint32 i = 0; i < instanceExtensionsCount; ++i)
				{
					extensions.emplace_back(instanceExtensions[i]);
				}

				if constexpr (ENABLE_VALIDATION_LAYERS)
				{
					// Add in the extra required extensions
					extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}

				VkInstanceCreateInfo instanceInfo{};
				instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				instanceInfo.pApplicationInfo = &appInfo;
				instanceInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
				instanceInfo.ppEnabledExtensionNames = extensions.data();

				if constexpr (ENABLE_VALIDATION_LAYERS)
				{
					// Add the layers into the create info if we requested it (Debug only)
					instanceInfo.enabledLayerCount = static_cast<uint32>(VALIDATION_LAYERS.size());
					instanceInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

					// Set the next create info to be the debug messenger
					VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
					PopulateDebugMessengerCreateInfo(debugCreateInfo);
					instanceInfo.pNext = &debugCreateInfo;
				}

				Try(
					vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance),
					"Failed to create Vulkan Instance!"
				);
			},
			[this]
			{
				vkDestroyInstance(m_vkInstance, nullptr);
			}
		);

		// Debug Messenger
		if constexpr (ENABLE_VALIDATION_LAYERS)
		{
			InitAndPushResource(
				[this]
				{
					VkDebugUtilsMessengerCreateInfoEXT createInfo;
					PopulateDebugMessengerCreateInfo(createInfo);

					Try(
						CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_debugMessenger),
						"Failed to create Debug Messenger!"
					);
				},
				[this]
				{
					DestroyDebugUtilsMessengerEXT(m_vkInstance, m_debugMessenger, nullptr);
				}
			);
		}

		// Logical / Physical Device
		InitAndPushResource(
			[this]
			{
				// Get a list and number of all GPU's attached to the computer
				uint32 deviceCount = 0;
				vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
				vector<VkPhysicalDevice> devices(deviceCount);
				vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

				// Get the properties of the device
				m_physicalDevice = devices[0];
				VkPhysicalDeviceProperties2 deviceProperties
				{
					.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
					.pNext = nullptr,
					.properties = {}
				};
				vkGetPhysicalDeviceProperties2(m_physicalDevice, &deviceProperties);

				// Get all the device's queue families
				uint32 queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
				vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

				// Get the graphics queue family index
				m_queueFamily = 0;
				for (uint32 i = 0; i < queueFamilyCount; ++i)
				{
					if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					{
						m_queueFamily = i;
						break;
					}
				}

				// Validate the queue family support
				if (glfwGetPhysicalDevicePresentationSupport(m_vkInstance, m_physicalDevice, m_queueFamily) == GLFW_FALSE)
				{
					throw runtime_error("GLFW does not support presentation on this queue family!");
				}

				// Generate the queue create info with a 100% priority
				constexpr float qfPriorities = 1.f;
				VkDeviceQueueCreateInfo queueCI
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.queueFamilyIndex = m_queueFamily,
					.queueCount = 1,
					.pQueuePriorities = &qfPriorities,
				};

				// Generate the feature and extension supports we need
				const vector deviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
				VkPhysicalDeviceVulkan11Features enabledVk11Features{};
				enabledVk11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
				enabledVk11Features.variablePointers = true;
				enabledVk11Features.variablePointersStorageBuffer = true;

				VkPhysicalDeviceVulkan12Features enabledVk12Features{};
				enabledVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				enabledVk12Features.pNext = &enabledVk11Features;
				enabledVk12Features.descriptorIndexing = true;
				enabledVk12Features.shaderSampledImageArrayNonUniformIndexing = true;
				enabledVk12Features.descriptorBindingUniformBufferUpdateAfterBind = true;
				enabledVk12Features.descriptorBindingVariableDescriptorCount = true;
				enabledVk12Features.runtimeDescriptorArray = true;
				enabledVk12Features.bufferDeviceAddress = true;

				VkPhysicalDeviceVulkan13Features enabledVk13Features{};
				enabledVk13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
				enabledVk13Features.pNext = &enabledVk12Features;
				enabledVk13Features.synchronization2 = true;
				enabledVk13Features.dynamicRendering = true;

				VkPhysicalDeviceFeatures enabledVk10Features{};
				enabledVk10Features.samplerAnisotropy = true;

				// Generate the device create info with the above parameters
				const VkDeviceCreateInfo deviceCI
				{
					.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
					.pNext = &enabledVk13Features,
					.flags = 0,
					.queueCreateInfoCount = 1,
					.pQueueCreateInfos = &queueCI,
					.enabledLayerCount = 0, // DEPRECATED
					.ppEnabledLayerNames = nullptr, // DEPRECATED
					.enabledExtensionCount = static_cast<uint32>(deviceExtensions.size()),
					.ppEnabledExtensionNames = deviceExtensions.data(),
					.pEnabledFeatures = &enabledVk10Features
				};

				// Attempt to create the device and get the graphics queue
				Try(
					vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device),
					"Failed to create Logical Device!"
				);
				vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);
			},
			[this]
			{
				vkDestroyDevice(m_device, nullptr);
			}
		);

		// VMA allocator
		InitAndPushResource(
			[this]
			{
				VmaVulkanFunctions vkFunctions{};
				vkFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
				vkFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
				vkFunctions.vkCreateImage = vkCreateImage;

				VmaAllocatorCreateInfo allocatorCI{};
				allocatorCI.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
				allocatorCI.physicalDevice = m_physicalDevice;
				allocatorCI.device = m_device;
				allocatorCI.pVulkanFunctions = &vkFunctions;
				allocatorCI.instance = m_vkInstance;

				Try(
					vmaCreateAllocator(&allocatorCI, &m_vmaAllocator),
					"Failed to create VMA Allocator"
				);
			},
			[this]
			{
				vmaDestroyAllocator(m_vmaAllocator);
			}
		);

		// Surface
		InitAndPushResource(
			[this, window]
			{
				Try(
					glfwCreateWindowSurface(m_vkInstance, window, nullptr, &m_surface),
					"Failed to create window surface!"
				);
			},
			[this]
			{
				vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);
			}
		);

		// Swap chain / swap chain images
		InitAndPushResource(
			[this, window]
			{
				VkSurfaceCapabilitiesKHR surfaceCaps{};
				Try(
					vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps),
					"Failed to retrieve surface capabilities"
				);

				// Verify the window size
				VkExtent2D swapChainExtent = surfaceCaps.currentExtent;
				if (surfaceCaps.currentExtent.width == 0xffffffff)
				{
					// Get the GLFW window size
					int windowW, windowH;
					glfwGetWindowSize(window, &windowW, &windowH);

					// Use the glfw window size as the swap chain size
					swapChainExtent =
					{
						.width = static_cast<uint32>(windowW),
						.height = static_cast<uint32>(windowH)
					};
				}

				// Generate the Swap Chain Create Information
				constexpr VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
				VkSwapchainCreateInfoKHR swapChainCI{};
				swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
				swapChainCI.surface = m_surface;
				swapChainCI.minImageCount = surfaceCaps.minImageCount;
				swapChainCI.imageFormat = imageFormat;
				swapChainCI.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
				swapChainCI.imageExtent = { .width = swapChainExtent.width, .height = swapChainExtent.height };
				swapChainCI.imageArrayLayers = 1;
				swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
				swapChainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
				swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
				swapChainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;

				// Attempt to create the swap chain
				Try(
					vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain),
					"Failed to create Swap Chain!"
				);

				// Attempt to acquire the swap chain images from the swap chain
				uint32 scImageCount = 0;
				Try(
					vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, nullptr),
					"Failed to count Swap Chain Images!"
				);

				m_swapChainImages.resize(scImageCount);
				Try(
					vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, m_swapChainImages.data()),
					"Failed to retrieve Swap Chain Images!"
				);

				// Resize the image view vector to match the image one
				m_swapChainImageViews.resize(scImageCount);

				// Create the new Swap Chain image views
				for (uint32 i = 0; i < scImageCount; ++i)
				{
					VkImageViewCreateInfo viewCreateInfo{};
					viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
					viewCreateInfo.image = m_swapChainImages[i];
					viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
					viewCreateInfo.format = imageFormat;
					viewCreateInfo.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)

					Try(
						vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
						std::format("Failed to create Swap Chain Image View for index: {}", i)
					);
				}
			},
			[this]
			{
				for (VkImageView& scImageView : m_swapChainImageViews)
				{
					vkDestroyImageView(m_device, scImageView, nullptr);
				}
				m_swapChainImageViews.clear();

				vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
			}
		);

		// Depth image
		InitAndPushResource(
			[this, window]
			{
				// Get the GLFW window size
				int windowW, windowH;
				glfwGetWindowSize(window, &windowW, &windowH);

				// Set up the image create info for the depth image
				CreateDepthImage(
					{ .width = static_cast<uint32_t>(windowW), .height = static_cast<uint32_t>(windowH), .depth = 1 },
					GetDepthFormat()
				);
			},
			[this]
			{
				vkDestroyImageView(m_device, m_depthImageView, nullptr);
				vmaDestroyImage(m_vmaAllocator, m_depthImage, m_depthImageAllocation);
			}
		);

		// Shader uniform buffers
		InitAndPushResource(
			[this]
			{
				// We need a set of buffers for every frame in flight
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
				{
					vector<VulkanBuffer*> buffers;

					m_uboBuffers[i] = new VulkanBuffer{ sizeof(ProjectionViewUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_uboBuffers[i]);

					m_lightBuffers[i] = new VulkanBuffer{ sizeof(LightUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_lightBuffers[i]);

					m_materialBuffers[i] = new VulkanBuffer{ sizeof(MaterialUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_materialBuffers[i]);

					m_pushConstantBuffers[i] = new VulkanBuffer{ sizeof(PushConstantData), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_pushConstantBuffers[i]);

					m_shaderDataBuffers[i] = buffers;
				}
			},
			[this]
			{
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
				{
					// Delete each buffer for this frame in flight
					for (const VulkanBuffer* buffer : m_shaderDataBuffers[i])
					{
						delete buffer;
					}

					m_shaderDataBuffers[i].clear();
				}
			}
		);

		// Fences and semaphores
		InitAndPushResource(
			[this]
			{
				VkSemaphoreCreateInfo semaphoreCreateInfo{};
				semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

				// Make sure the fence will be signaled for the first frame
				VkFenceCreateInfo fenceCreateInfo{};
				fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				// Create a fence and an image semaphore for each frame in flight
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
				{
					Try(
						vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fences[i]),
						std::format("Failed to create Fence for frame: {}!", i)
					);

					Try(
						vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAcquiredSemaphores[i]),
						std::format("Failed to create Image Acquired Semaphore for frame: {}!", i)
					);
				}

				// Match the size of the render complete semaphores to the swap chain images
				m_renderCompleteSemaphores.resize(m_swapChainImages.size());
				for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
				{
					Try(
						vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore),
						"Failed to create Render Complete Semaphore!"
					);
				}
			},
			[this]
			{
				for (const VkSemaphore& semaphore : m_renderCompleteSemaphores)
				{
					vkDestroySemaphore(m_device, semaphore, nullptr);
				}

				m_renderCompleteSemaphores.clear();

				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
				{
					vkDestroySemaphore(m_device, m_imageAcquiredSemaphores[i], nullptr);
					vkDestroyFence(m_device, m_fences[i], nullptr);
				}
			}
		);

		// Command buffers
		InitAndPushResource(
			[this]
			{
				// Attempt to create the command pool
				VkCommandPoolCreateInfo cpCreateInfo{};
				cpCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cpCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				cpCreateInfo.queueFamilyIndex = m_queueFamily;

				Try(
					vkCreateCommandPool(m_device, &cpCreateInfo, nullptr, &m_commandPool),
					"Failed to create Command Pool!"
				);

				// Attempt to create command buffers for each frame in flight
				VkCommandBufferAllocateInfo cbAllocateInfo{};
				cbAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cbAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
				cbAllocateInfo.commandPool = m_commandPool;

				Try(
					vkAllocateCommandBuffers(m_device, &cbAllocateInfo, m_commandBuffers.data()),
					"Failed to create Command Buffers!"
				);
			},
			[this]
			{
				vkDestroyCommandPool(m_device, m_commandPool, nullptr);
			}
		);

		// Descriptors
		InitAndPushResource(
			[this]
			{
				array dslBindings =
				{
					VkDescriptorSetLayoutBinding
					{
						.binding = m_maxDescriptorBinding++,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1,
						.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
						.pImmutableSamplers = nullptr,
					},
					VkDescriptorSetLayoutBinding
					{
						.binding = m_maxDescriptorBinding++,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1,
						.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
						.pImmutableSamplers = nullptr,
					},
					VkDescriptorSetLayoutBinding
					{
						.binding = m_maxDescriptorBinding++,
						.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						.descriptorCount = MAX_TEXTURE_DESCRIPTORS,
						.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
						.pImmutableSamplers = nullptr,
					},
				};

				array flags =
				{
					VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT },
					VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT },
					VkDescriptorBindingFlags{ VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT },
				};

				const VkDescriptorSetLayoutBindingFlagsCreateInfo dslFlagsCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
					.pNext = nullptr,
					.bindingCount = static_cast<uint32>(dslBindings.size()),
					.pBindingFlags = flags.data()
				};

				const VkDescriptorSetLayoutCreateInfo dslCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = &dslFlagsCreateInfo,
					.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
					.bindingCount = static_cast<uint32>(dslBindings.size()),
					.pBindings = dslBindings.data()
				};

				// Create the descriptor set layout
				Try(
					vkCreateDescriptorSetLayout(m_device, &dslCreateInfo, nullptr, &m_descriptorSetLayout),
					"Failed to create Descriptor Set Layout!"
				);

				array poolSizes
				{
					VkDescriptorPoolSize
					{
						.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						.descriptorCount = MAX_TEXTURE_DESCRIPTORS
					},
					VkDescriptorPoolSize
					{
						.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1
					},
					VkDescriptorPoolSize
					{
						.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						.descriptorCount = 1
					}
				};
				const VkDescriptorPoolCreateInfo dpCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
					.maxSets = 1,
					.poolSizeCount = static_cast<uint32>(poolSizes.size()),
					.pPoolSizes = poolSizes.data()
				};

				// Create the descriptor pool
				Try(
					vkCreateDescriptorPool(m_device, &dpCreateInfo, nullptr, &m_descriptorPool),
					"Failed to create Descriptor Pool!"
				);

				// Allocate the descriptor sets
				constexpr VkDescriptorSetVariableDescriptorCountAllocateInfo vdcAllocateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT,
					.pNext = nullptr,
					.descriptorSetCount = 1,
					.pDescriptorCounts = &MAX_TEXTURE_DESCRIPTORS
				};
				const VkDescriptorSetAllocateInfo dsAllocateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
					.pNext = &vdcAllocateInfo,
					.descriptorPool = m_descriptorPool,
					.descriptorSetCount = 1,
					.pSetLayouts = &m_descriptorSetLayout
				};

				Try(
					vkAllocateDescriptorSets(m_device, &dsAllocateInfo, &m_descriptorSet),
					"Failed to allocate Descriptor Sets!"
				);
			},
			[this]
			{
				vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
				vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
			}
		);

		// Set the resize callback
		glfwSetWindowSizeCallback(window, [](GLFWwindow* win, int w, int h)
			{
				Application::GetWindow()->SetWidth(w);
				Application::GetWindow()->SetWidth(h);

				Instance()->m_recreateSwapChain = true;
			});

		// All functions ran safely, so we loaded correctly. 
		m_loaded = true;
	}
	catch (exception& e)
	{
		Console::Exception(e);
	}
}

void Vulkan::RecreateSwapChain()
{
	vkDeviceWaitIdle(m_device);

	const Window* window = Application::GetWindow();

	// Try to get the device capabilities
	VkSurfaceCapabilitiesKHR surfaceCaps;
	Try(
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps),
		"Failed to get the device capabilities!"
	);

	// Verify the window size
	VkExtent2D swapChainExtent = surfaceCaps.currentExtent;
	if (surfaceCaps.currentExtent.width == 0xffffffff)
	{

		// Use the glfw window size as the swap chain size
		swapChainExtent =
		{
			.width = static_cast<uint32>(window->Width()),
			.height = static_cast<uint32>(window->Height())
		};
	}

	// Generate the Swap Chain Create Information
	constexpr VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
	VkSwapchainCreateInfoKHR swapChainCI{};
	swapChainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCI.surface = m_surface;
	swapChainCI.minImageCount = surfaceCaps.minImageCount;
	swapChainCI.imageFormat = imageFormat;
	swapChainCI.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapChainCI.imageExtent = { .width = swapChainExtent.width, .height = swapChainExtent.height };
	swapChainCI.imageArrayLayers = 1;
	swapChainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCI.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapChainCI.oldSwapchain = m_swapChain;

	// Create the new swap chain
	Try(
		vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain),
		"Failed to Recreate Swap Chain!"
	);

	// Destroy old swap chain images
	uint32 imageCount = static_cast<uint32>(m_swapChainImages.size());
	for (uint32 i = 0; i < imageCount; ++i)
	{
		vkDestroyImageView(m_device, m_swapChainImageViews[i], nullptr);
	}

	// Get the new Swap Chain Images
	imageCount = 0;
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr),
		"Failed to get Swap Chain Image Count!"
	);
	m_swapChainImages.resize(imageCount);
	Try(
		vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data()),
		"Failed to get Swap Chain Images!"
	);
	m_swapChainImageViews.resize(imageCount);

	// Create the new Swap Chain image views
	for (uint32 i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = m_swapChainImages[i];
		viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = imageFormat;
		viewCreateInfo.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)

		Try(
			vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_swapChainImageViews[i]),
			std::format("Failed to create Swap Chain Image View for index: {}", i)
		);
	}

	// Destroy old semaphores
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		vkDestroySemaphore(m_device, semaphore, nullptr);
	}

	// Recreate semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	m_renderCompleteSemaphores.resize(imageCount);
	for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
	{
		Try(
			vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore),
			"Failed to recreate semaphore!"
		);
	}

	// Destroy the old swap chain and depth image / image view
	vkDestroySwapchainKHR(m_device, swapChainCI.oldSwapchain, nullptr);
	vmaDestroyImage(m_vmaAllocator, m_depthImage, m_depthImageAllocation);
	vkDestroyImageView(m_device, m_depthImageView, nullptr);

	CreateDepthImage(
		{ .width = static_cast<uint32>(window->Width()), .height = static_cast<uint32>(window->Height()), .depth = 1 },
		GetDepthFormat()
	);
}

VkCommandBuffer Vulkan::BeginFrame()
{
	WriteTextureDescriptorSets();

	// Wait on and reset fences
	Try(
		vkWaitForFences(m_device, 1, &m_fences[m_frameIndex], true, UINT64_MAX),
		std::format("Failed to wait for fence on frame: {}!", m_frameIndex)
	);
	Try(
		vkResetFences(m_device, 1, &m_fences[m_frameIndex]),
		std::format("Failed to reset fence on frame: {}!", m_frameIndex)
	);

	// Try to get the swap chain image index for this frame
	CheckSwapChain(
		vkAcquireNextImageKHR(
		m_device, m_swapChain, UINT64_MAX, m_imageAcquiredSemaphores[m_frameIndex], VK_NULL_HANDLE, &m_imageIndex
	),
		std::format("Failed to acquire Swap Chain Image index for frame: {}!", m_frameIndex)
	);

	// Try to reset and retrieve the command buffer
	const VkCommandBuffer cmdBuf = m_commandBuffers[m_frameIndex];
	Try(
		vkResetCommandBuffer(cmdBuf, 0),
		std::format("Failed to reset Command Buffer for frame: {}!", m_frameIndex)
	);

	// Begin using the command buffer
	VkCommandBufferBeginInfo cbBeginInfo{};
	cbBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cbBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	Try(
		vkBeginCommandBuffer(cmdBuf, &cbBeginInfo),
		std::format("Failed to begin Command Buffer for frame: {}!", m_frameIndex)
	);

	// Transition swap chain and depth images
	TransitionFrameImages(cmdBuf);

	// Begin rendering
	VkRenderingAttachmentInfo colorAttachmentInfo{};
	colorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachmentInfo.imageView = m_swapChainImageViews[m_imageIndex];
	colorAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentInfo.clearValue = { .color = static_cast<VkClearColorValue>(m_clearColor) };  // NOLINT(clang-diagnostic-missing-braces)

	VkRenderingAttachmentInfo depthAttachmentInfo{};
	depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachmentInfo.imageView = m_depthImageView;
	depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentInfo.clearValue = { .depthStencil = { 1.f, 0 } };

	const Window* window = Application::GetWindow();
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = { .extent = { static_cast<uint32>(window->Width()), static_cast<uint32>(window->Height()) } };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = &depthAttachmentInfo;
	vkCmdBeginRendering(cmdBuf, &renderingInfo);

	// Set the viewport and scissor
	const VkViewport vp =
	{
		.x = 0.f,
		.y = 0.f,
		.width = window->Width(),
		.height = window->Height(),
		.minDepth = 0.f,
		.maxDepth = 1.f
	};
	vkCmdSetViewport(cmdBuf, 0, 1, &vp);

	const VkRect2D scissor =
	{
		.offset = { 0, 0 },
		.extent = renderingInfo.renderArea.extent
	};
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

	return cmdBuf;
}

void Vulkan::EndFrame(VkCommandBuffer cmdBuffer)
{
	// End the rendering and transition the swap chain image
	vkCmdEndRendering(cmdBuffer);

	VkImageMemoryBarrier2 barrierPresent{};
	barrierPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	barrierPresent.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrierPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrierPresent.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	barrierPresent.dstAccessMask = 0;
	barrierPresent.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
	barrierPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrierPresent.image = m_swapChainImages[m_imageIndex];
	barrierPresent.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 };  // NOLINT(clang-diagnostic-missing-designated-field-initializers)

	VkDependencyInfo barrierPresentDependencyInfo{};
	barrierPresentDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	barrierPresentDependencyInfo.imageMemoryBarrierCount = 1;
	barrierPresentDependencyInfo.pImageMemoryBarriers = &barrierPresent;
	vkCmdPipelineBarrier2(cmdBuffer, &barrierPresentDependencyInfo);

	// Try to end the command buffer
	Try(
		vkEndCommandBuffer(cmdBuffer),
		std::format("Failed to end Command Buffer for frame: {}!", m_frameIndex)
	);

	// Try to submit the queue
	VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_imageAcquiredSemaphores[m_frameIndex],
		.pWaitDstStageMask = &waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmdBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderCompleteSemaphores[m_imageIndex]
	};
	Try(
		vkQueueSubmit(m_queue, 1, &submitInfo, m_fences[m_frameIndex]),
		std::format("Failed to submit queue for frame: {}!", m_frameIndex)
	);

	// Try to present the queue
	m_frameIndex = (m_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	VkPresentInfoKHR presentInfo
	{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderCompleteSemaphores[m_imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain,
		.pImageIndices = &m_imageIndex,
		.pResults = nullptr
	};
	CheckSwapChain(
		vkQueuePresentKHR(m_queue, &presentInfo),
		std::format("Failed to present queue for frame: {}!", m_frameIndex)
	);

	// Recreate the Swap Chain if needed
	if (m_recreateSwapChain)
	{
		m_recreateSwapChain = false;
		RecreateSwapChain();
	}
}

void Vulkan::TransitionFrameImages(const VkCommandBuffer cmdBuffer) const
{
	const array outputBarriers
	{
		VkImageMemoryBarrier2
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = m_swapChainImages[m_imageIndex],
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		},
		VkImageMemoryBarrier2
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.pNext = nullptr,
			.srcStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = 0,
			.dstQueueFamilyIndex = 0,
			.image = m_depthImage,
			.subresourceRange =
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		}
	};
	VkDependencyInfo barrierDependencyInfo{};
	barrierDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	barrierDependencyInfo.imageMemoryBarrierCount = static_cast<uint32>(outputBarriers.size());
	barrierDependencyInfo.pImageMemoryBarriers = outputBarriers.data();
	vkCmdPipelineBarrier2(cmdBuffer, &barrierDependencyInfo);
}

void Vulkan::CreateDepthImage(const VkExtent3D& extent, const VkFormat& format)
{
	VkImageCreateInfo depthImageCI{};
	depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	depthImageCI.imageType = VK_IMAGE_TYPE_2D;
	depthImageCI.format = format;
	depthImageCI.extent = extent;
	depthImageCI.mipLevels = 1;
	depthImageCI.arrayLayers = 1;
	depthImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	depthImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depthImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocCI{};
	allocCI.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocCI.usage = VMA_MEMORY_USAGE_AUTO;

	// Attempt to create the depth image
	Try(
		vmaCreateImage(m_vmaAllocator, &depthImageCI, &allocCI, &m_depthImage, &m_depthImageAllocation, nullptr),
		"Failed to create Depth Image!"
	);

	VkImageViewCreateInfo depthViewCI
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = m_depthImage,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
		.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
	};

	// Attempt to create the depth image view
	Try(
		vkCreateImageView(m_device, &depthViewCI, nullptr, &m_depthImageView),
		"Failed to create Depth Image View!"
	);
}

void Vulkan::InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const
{
	init();
	m_resourceStack->Push(cleanup);
}
