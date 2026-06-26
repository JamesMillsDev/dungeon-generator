#include "pch.h"
#include "Vulkan.h"

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "Uniforms.h"
#include "VulkanBuffer.h"
#include "VulkanGraphicsPipeline.h"
#include "Graphics/Rendering/Texture.h"
#include "Utility/Console.h"
#include "Utility/ResourceStack.h"

using std::exception;

constexpr uint32 MAX_TEXTURE_DESCRIPTORS = UINT16_MAX;
constexpr int32 DEFAULT_RESOURCE_STACK_SIZE = 16;

Vulkan* Vulkan::m_instance = nullptr;

Vulkan* Vulkan::Instance()
{
	return m_instance;
}

const VkDevice& Vulkan::Device()
{
	return m_instance->GetDevice();
}

const VmaAllocator& Vulkan::Allocator()
{
	return m_instance->GetAllocator();
}

const VkDescriptorSetLayout& Vulkan::DescriptorSetLayout()
{
	return m_instance->GetDescriptorSetLayout();
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

Vulkan::Vulkan(Config* config, GLFWwindow* window)
	: m_resourceStack{ new ResourceStack{ DEFAULT_RESOURCE_STACK_SIZE } }, m_loaded{ false }
{
	m_appName = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };
	m_engineName = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };

	Init(window);
}

Vulkan::~Vulkan()
{
	delete m_resourceStack;
	delete m_appVersion;
	delete m_engineVersion;
}

const VkDevice& Vulkan::GetDevice() const
{
	return m_device;
}

const VmaAllocator& Vulkan::GetAllocator() const
{
	return m_vmaAllocator;
}

const VkDescriptorSetLayout& Vulkan::GetDescriptorSetLayout() const
{
	return m_descriptorSetLayout;
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
}

VulkanBuffer* Vulkan::GetProjectionViewBuffer() const
{
	// TODO: This needs to be the current frame
	return m_projViewBuffers[0];
}

VulkanBuffer* Vulkan::GetLightBuffer() const
{
	// TODO: This needs to be the current frame
	return m_lightBuffers[0];
}

VulkanBuffer* Vulkan::GetMaterialBuffer() const
{
	// TODO: This needs to be the current frame
	return m_materialBuffers[0];
}

void Vulkan::AddTexture(Texture* texture)
{
	m_textures.emplace_back(texture);
	WriteTextureDescriptorSets();
}

void Vulkan::RemoveTexture(Texture* texture)
{
	std::erase_if(
		m_textures, [texture](const Texture* t)
		{
			return texture == t;
		}
	);
	WriteTextureDescriptorSets();
}

void Vulkan::WriteTextureDescriptorSets() const
{
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
		.dstBinding = 0,
		.dstArrayElement = 0,
		.descriptorCount = static_cast<uint32>(textureDescriptors.size()),
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = textureDescriptors.data(),
		.pBufferInfo = nullptr,
		.pTexelBufferView = nullptr
	};
	vkUpdateDescriptorSets(m_device, 1, &writeDescSet, 0, nullptr);
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

				const VkInstanceCreateInfo instanceInfo
				{
					.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.pApplicationInfo = &appInfo,
					.enabledLayerCount = 0,
					.ppEnabledLayerNames = nullptr,
					.enabledExtensionCount = instanceExtensionsCount,
					.ppEnabledExtensionNames = instanceExtensions
				};

				if (const VkResult result = vkCreateInstance(&instanceInfo, nullptr, &m_vkInstance);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Vulkan Instance!", result);
				}
			},
			[this]
			{
				vkDestroyInstance(m_vkInstance, nullptr);
			}
		);

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
				VkPhysicalDeviceVulkan12Features enabledVk12Features{};
				enabledVk12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
				enabledVk12Features.descriptorIndexing = true;
				enabledVk12Features.shaderSampledImageArrayNonUniformIndexing = true;
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

				// Attempt to create the device
				if (const VkResult result = vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Logical Device!", result);
				}

				// Get the graphics queue
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

				if (const VkResult result = vmaCreateAllocator(&allocatorCI, &m_vmaAllocator);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create VMA Allocator", result);
				}
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
				if (const VkResult result = glfwCreateWindowSurface(m_vkInstance, window, nullptr, &m_surface);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create window surface!", result);
				}
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
				VkResult result;

				VkSurfaceCapabilitiesKHR surfaceCaps{};
				if (result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCaps);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to retrieve surface capabilities", result);
				}

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
				if (result = vkCreateSwapchainKHR(m_device, &swapChainCI, nullptr, &m_swapChain);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Swap Chain!", result);
				}

				// Attempt to acquire the swap chain images from the swap chain
				uint32 scImageCount = 0;
				if (result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, nullptr);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to count Swap Chain Images!", result);
				}

				m_swapChainImages.resize(scImageCount);
				if (result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &scImageCount, m_swapChainImages.data());
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to retrieve Swap Chain Images!", result);
				}

				// Resize the image view vector to match the image one
				m_swapChainImageViews.resize(scImageCount);
			},
			[this]
			{
				vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
			}
		);

		// Depth image
		InitAndPushResource(
			[this, window]
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

				// Get the GLFW window size
				int windowW, windowH;
				glfwGetWindowSize(window, &windowW, &windowH);

				// Set up the image create info for the depth image
				VkImageCreateInfo depthImageCI{};
				depthImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				depthImageCI.imageType = VK_IMAGE_TYPE_2D;
				depthImageCI.format = depthFormat;
				depthImageCI.extent = { .width = static_cast<uint32_t>(windowW), .height = static_cast<uint32_t>(windowH), .depth = 1 };
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
				if (const VkResult result = vmaCreateImage(m_vmaAllocator, &depthImageCI, &allocCI, &m_depthImage, &m_depthImageAllocation, nullptr);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Depth Image!", result);
				}

				VkImageViewCreateInfo depthViewCI
				{
					.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.image = m_depthImage,
					.viewType = VK_IMAGE_VIEW_TYPE_2D,
					.format = depthFormat,
					.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
					.subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
				};

				// Attempt to create the depth image view
				if (const VkResult result = vkCreateImageView(m_device, &depthViewCI, nullptr, &m_depthImageView);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Depth Image View!", result);
				}
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

					m_projViewBuffers[i] = new VulkanBuffer{ sizeof(ProjectionViewUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_projViewBuffers[i]);

					m_lightBuffers[i] = new VulkanBuffer{ sizeof(LightUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_lightBuffers[i]);

					m_materialBuffers[i] = new VulkanBuffer{ sizeof(MaterialUniform), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, this };
					buffers.emplace_back(m_materialBuffers[i]);

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

				VkResult result;

				// Create a fence and an image semaphore for each frame in flight
				for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
				{
					if (result = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fences[i]);
						result != VK_SUCCESS)
					{
						throw VulkanError(std::format("Failed to create Fence for frame: {}!", i), result);
					}

					if (result = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_imageAcquiredSemaphores[i]);
						result != VK_SUCCESS)
					{
						throw VulkanError(std::format("Failed to create Image Acquired Semaphore for frame: {}!", i), result);
					}
				}

				// Match the size of the render complete semaphores to the swap chain images
				m_renderCompleteSemaphores.resize(m_swapChainImages.size());
				for (VkSemaphore& semaphore : m_renderCompleteSemaphores)
				{
					if (result = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &semaphore);
						result != VK_SUCCESS)
					{
						throw VulkanError("Failed to create Render Complete Semaphore!", result);
					}
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
				VkResult result;

				// Attempt to create the command pool
				VkCommandPoolCreateInfo cpCreateInfo{};
				cpCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cpCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				cpCreateInfo.queueFamilyIndex = m_queueFamily;

				if (result = vkCreateCommandPool(m_device, &cpCreateInfo, nullptr, &m_commandPool);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Command Pool!", result);
				}

				// Attempt to create command buffers for each frame in flight
				VkCommandBufferAllocateInfo cbAllocateInfo{};
				cbAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				cbAllocateInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
				cbAllocateInfo.commandPool = m_commandPool;

				if (result = vkAllocateCommandBuffers(m_device, &cbAllocateInfo, m_commandBuffers.data());
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Command Buffers!", result);
				}
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
				VkResult result;

				constexpr VkDescriptorBindingFlags descVariableFlag = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
				const VkDescriptorSetLayoutBindingFlagsCreateInfo dslFlagsCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
					.pNext = nullptr,
					.bindingCount = 1,
					.pBindingFlags = &descVariableFlag
				};

				constexpr VkDescriptorSetLayoutBinding dslBinding
				{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = MAX_TEXTURE_DESCRIPTORS,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = nullptr
				};
				const VkDescriptorSetLayoutCreateInfo dslCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
					.pNext = &dslFlagsCreateInfo,
					.flags = 0,
					.bindingCount = 1,
					.pBindings = &dslBinding
				};

				// Create the descriptor set layout
				if (result = vkCreateDescriptorSetLayout(m_device, &dslCreateInfo, nullptr, &m_descriptorSetLayout);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Descriptor Set Layout!", result);
				}

				constexpr VkDescriptorPoolSize poolSize
				{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = MAX_TEXTURE_DESCRIPTORS
				};
				const VkDescriptorPoolCreateInfo dpCreateInfo
				{
					.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.maxSets = 1,
					.poolSizeCount = 1,
					.pPoolSizes = &poolSize
				};

				// Create the descriptor pool
				if (result = vkCreateDescriptorPool(m_device, &dpCreateInfo, nullptr, &m_descriptorPool);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to create Descriptor Pool!", result);
				}

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

				if (result = vkAllocateDescriptorSets(m_device, &dsAllocateInfo, &m_descriptorSet);
					result != VK_SUCCESS)
				{
					throw VulkanError("Failed to allocate Descriptor Sets!", result);
				}

				// Write to the descriptor sets
				WriteTextureDescriptorSets();
			},
			[this]
			{
				vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
				vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
			}
		);

		// Graphics Pipeline
		InitAndPushResource(
			[this]
			{
				GraphicsPipelineConfig config = GraphicsPipelineConfig{ "Shaders/pbr", this };

				m_pipeline = new VulkanGraphicsPipeline{ config, this };
			},
			[this]
			{
				delete m_pipeline;
			}
		);

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

}

void Vulkan::InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const
{
	init();
	m_resourceStack->Push(cleanup);
}