#include "pch.h"
#include "Vulkan.h"

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include "Utility/Console.h"
#include "Utility/ResourceStack.h"

using std::exception;

constexpr int32 DEFAULT_RESOURCE_STACK_SIZE = 16;

Vulkan* Vulkan::m_instance = nullptr;

Vulkan* Vulkan::Instance()
{
	return m_instance;
}

const VkDevice& Vulkan::Device()
{
	return m_instance->m_device;
}

const VmaAllocator& Vulkan::Allocator()
{
	return m_instance->m_vmaAllocator;
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

void Vulkan::Init(GLFWwindow* window)
{
	try
	{
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
				uint32 queueFamily = 0;
				for (uint32 i = 0; i < queueFamilyCount; ++i)
				{
					if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
					{
						queueFamily = i;
						break;
					}
				}

				// Validate the queue family support
				if (glfwGetPhysicalDevicePresentationSupport(m_vkInstance, m_physicalDevice, queueFamily) == GLFW_FALSE)
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
					.queueFamilyIndex = queueFamily,
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
				vkGetDeviceQueue(m_device, queueFamily, 0, &m_queue);
			},
			[this]
			{
				vkDestroyDevice(m_device, nullptr);
			}
		);

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
				CleanupSwapChain();
			}
		);

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
					.subresourceRange{ .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 }
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

		// All functions ran safely, so we loaded correctly. 
		m_loaded = true;
	}
	catch (exception& e)
	{
		Console::Exception(e);
	}
}

void Vulkan::CleanupSwapChain()
{
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void Vulkan::InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const
{
	init();
	m_resourceStack->Push(cleanup);
}