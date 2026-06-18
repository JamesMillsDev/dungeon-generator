#include "pch.h"
#include "Vulkan.h"

#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Buffer.h"
#include "Rendering/Mesh.h"
#include "Rendering/Shader.h"

using std::exception;
using std::multimap;
using std::runtime_error;
using std::set;

#pragma region VkInstance / Debug Messenger
namespace
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			std::cerr << "validation layer: " << pCallbackData->pMessage << "\n";
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

void Vulkan::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	Vulkan* vulkan = static_cast<Vulkan*>(glfwGetWindowUserPointer(window));
	if (vulkan != nullptr)
	{
		vulkan->m_frameBufferResized = true;
	}
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

vector<const char*> Vulkan::GetRequiredExtensions()
{
	// Get the GLFW extensions and the extension count
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		// Add in the extra required extensions
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
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

void Vulkan::CreateInstance()
{
	// ReSharper disable once CppRedundantBooleanExpressionArgument
	if (ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport())
	{
		throw runtime_error("Validation layers are requested, but not available.");
	}

	// Set up the information required for the application using the config values
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pEngineName = m_engineTitle.c_str();
	appInfo.engineVersion = VK_MAKE_VERSION(m_engineVersion->major, m_engineVersion->minor, m_engineVersion->patch);
	appInfo.pApplicationName = m_appTitle.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(m_appVersion->major, m_appVersion->minor, m_appVersion->patch);
	appInfo.apiVersion = VK_MAKE_API_VERSION(0, m_vulkanVersion->major, m_vulkanVersion->minor, 0);

	// Begin the creation of the instance create info using the application info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get all the required extensions
	const vector<const char*> extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = static_cast<uint32>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	createInfo.pNext = nullptr;

	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		// Add the layers into the create info if we requested it (Debug only)
		createInfo.enabledLayerCount = static_cast<uint32>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

		// Set the next create info to be the debug messenger
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}

	// Attempt to create the instance, throwing a runtime error with the error code if failed.
	if (const VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance); result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Vulkan Instance! Error Code: {}", static_cast<uint32>(result))
		);
	}
}

void Vulkan::SetupDebugMessenger()
{
	// If we don't want validation layers, exit early
	if constexpr (!ENABLE_VALIDATION_LAYERS)
	{
		return;
	}

	// Set up the create info
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (const VkResult result = CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger);
		result != VK_SUCCESS)
	{
		throw runtime_error(std::format(
			"Failed to set up Debug Messenger! Error Code: {}", static_cast<uint32>(result))
		);
	}
}
#pragma endregion

#pragma region Surface
void Vulkan::CreateSurface()
{
	if (const VkResult result = glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create window surface! Error Code: {}", static_cast<uint32>(result))
		);
	}
}
#pragma endregion

#pragma region Physical / Logical Device
uint32 Vulkan::RateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	// Get the properties of the device
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Get the features of the device
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	uint32 score = 0;

	// Discrete GPUs (non-integrated) have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders due to GPU particles
	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	// Application can't function without queue families
	QueueFamilyIndices queueFamily = FindQueueFamilies(device, surface);
	if (!queueFamily.IsComplete())
	{
		return 0;
	}

	// Check if the extensions are supported on this device
	bool extensionsSupported = CheckDeviceExtensionSupport(device);

	// If extensions are supported, check the swap chain support
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// Application can't function without swap chain support
	if (!swapChainAdequate)
	{
		return 0;
	}

	return score;
}

QueueFamilyIndices Vulkan::FindQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
	// Get the queue families from this physical device
	uint32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	QueueFamilyIndices indices;

	int i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		// If this family has the graphics bit, we can use it
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Attempt to get the surface support of this device
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
		{
			// It supports it, so store the indices
			indices.presentFamily = i;
		}

		// If the queue family index has been completed, we can break this loop
		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

bool Vulkan::CheckDeviceExtensionSupport(const VkPhysicalDevice device)
{
	// Get the extensions available to do this device
	uint32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// Iterate over every available extension, and remove it from the required set
	set<string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
	for (const VkExtensionProperties& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	// Succeeds if there are no more remaining extensions.
	return requiredExtensions.empty();
}

SwapChainSupportDetails Vulkan::QuerySwapChainSupport(const VkPhysicalDevice device, const VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	// Get the physical device surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Get the surface formats that the device supports
	uint32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If there are any, fill the details buffer
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Get the surface formats that the device supports
	uint32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	// If there are any, fill the details buffer
	if (formatCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR Vulkan::ChooseSwapChainFormat(const vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const VkSurfaceFormatKHR& format : availableFormats)
	{
		// If the format is sRGB and in BGRA uint format, this is a valid surface format
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	// We need a format, but since none where chosen, use the first one
	return availableFormats[0];
}

VkPresentModeKHR Vulkan::ChooseSwapPresentMode(const vector<VkPresentModeKHR>& availablePresetModes)
{
	for (const VkPresentModeKHR& presentMode : availablePresetModes)
	{
		// Use the mailbox mode if it is available
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentMode;
		}
	}

	// Default to double buffering in case mailbox mode isn't available
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Vulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max())
	{
		return capabilities.currentExtent;
	}

	// Get the current window size in pixels
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	VkExtent2D actualExtent
	{
		static_cast<uint32>(w),
		static_cast<uint32>(h)
	};

	// Clamp the actual extent to the maximum and minimum ranges just in case
	actualExtent.width = std::clamp(
		actualExtent.width, capabilities.maxImageExtent.width, capabilities.maxImageExtent.width
	);

	actualExtent.height = std::clamp(
		actualExtent.height, capabilities.maxImageExtent.height, capabilities.maxImageExtent.height
	);

	return actualExtent;
}

void Vulkan::PickPhysicalDevice()
{
	// Get the number of devices that support Vulkan.
	uint32 deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	// If somehow no devices support vulkan, throw a runtime error
	if (deviceCount == 0)
	{
		throw runtime_error("Failed to find GPUs with Vulkan support!");
	}

	// Get all devices again, filling the buffer
	vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	// Score each GPU
	multimap<uint32, VkPhysicalDevice> candidates;
	for (const VkPhysicalDevice& device : devices)
	{
		uint32 suitability = RateDeviceSuitability(device, m_surface);
		candidates.emplace(suitability, device);
	}

	// Use the highest scoring item as the GPU to use
	if (candidates.rbegin()->first != 0)
	{
		m_physicalDevice = candidates.rbegin()->second;
	}
	else
	{
		// No elements or none with non-zero scores
		throw runtime_error("Failed to find a suitable GPU!");
	}
}

void Vulkan::CreateLogicalDevice()
{
	// Attempt to get the queue family indices
	QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_surface);
	if (!indices.IsComplete())
	{
		throw runtime_error("Queue families failed to initialise!");
	}

	// Set up the queues for the logical device
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	const set uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };  // NOLINT(bugprone-unchecked-optional-access)

	float queuePriority = 1.f;
	for (uint32 queueFamily : uniqueQueueFamilies)
	{
		// Generate the create info for this queue family
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Set up the create info for the logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	VkPhysicalDeviceFeatures deviceFeatures{};
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32>(DEVICE_EXTENSIONS.size());
	createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

	createInfo.enabledLayerCount = 0;

	// Add the validation layer data if they are enabled
	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount = static_cast<uint32>(VALIDATION_LAYERS.size());
		createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
	}

	// Attempt to create the device, throwing a runtime_error with the error code if failed
	if (const VkResult result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Logical Device! Error Code: {}", static_cast<uint32>(result))
		);
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);  // NOLINT(bugprone-unchecked-optional-access, bugprone-unchecked-optional-access, bugprone-unchecked-optional-access)
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);  // NOLINT(bugprone-unchecked-optional-access)
}
#pragma endregion

#pragma region Swap Chain
void Vulkan::RecreateSwapChain()
{
	// Get the framebuffers size (this is typically the window size)
	int w = 0, h = 0;
	glfwGetFramebufferSize(m_window, &w, &h);

	// While "minimised"
	while (w == 0 || h == 0)
	{
		glfwGetFramebufferSize(m_window, &w, &h);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_device);

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateFrameBuffers();
}

void Vulkan::CleanupSwapChain() const
{
	for (const VkFramebuffer& framebuffer : m_swapChainFrameBuffers)
	{
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}

	for (const VkImageView& imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
}

void Vulkan::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);

	// Choose the correct format, mode and extent for this device
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, m_window);

	// Get one more than the minimum supported images to limit waiting on the driver, clamping to maximum
	uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Set up the initial swap chain create info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;

	// Provide the details about the swap images
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // This will always be 1 unless developing a stereoscopic application
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // This can be changed for post-processing

	// Set up the image families
	auto [graphicsFamily, presentFamily] = FindQueueFamilies(m_physicalDevice, m_surface);
	const uint32 queueFamilyIndices[] = { graphicsFamily.value(), presentFamily.value() };  // NOLINT(bugprone-unchecked-optional-access)

	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (graphicsFamily != presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // It is extremely rare for this to be anything else

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swap chain, throwing runtime_error if anything other than success
	if (const VkResult result = vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Swap Chain! Error Code: {}", static_cast<uint32>(result))
		);
	}

	// Obtain the swap chain images
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

	// Cache the extent and format
	m_swapChainFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

void Vulkan::CreateImageViews()
{
	// Match the image view vector size to the other one
	m_swapChainImageViews.resize(m_swapChainImages.size());

	// Iterate over each image
	for (uint64 i = 0; i < m_swapChainImages.size(); ++i)
	{
		// Set up the create info struct for this image view
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapChainImages[i];

		// Notify the struct of the view type (this can be used for 1,2,3D or cube map images)
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainFormat;

		// Assign the swizzles, in case you want to handle mono-chromatics
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Assign the purpose of this image view
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Attempt to create the image view for this index
		if (const VkResult result = vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create Image View for index '{}'! Error Code: {}", i, static_cast<uint32>(result))
			);
		}
	}
}

void Vulkan::CreateFrameBuffers()
{
	// Sync the frame buffer and image sizes
	m_swapChainFrameBuffers.resize(m_swapChainImages.size());

	// Iterate over each image view
	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		VkImageView attachments[] =
		{
			m_swapChainImageViews[i]
		};

		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = m_renderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = attachments;
		createInfo.width = m_swapChainExtent.width;
		createInfo.height = m_swapChainExtent.height;
		createInfo.layers = 1;

		if (const VkResult result = vkCreateFramebuffer(m_device, &createInfo, nullptr, &m_swapChainFrameBuffers[i]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create Framebuffer for index '{}'! Error Code: {}", i, static_cast<uint32>(result))
			);
		}
	}
}
#pragma endregion

#pragma region Graphics Pipeline
void Vulkan::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (const VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Render Pass! Error Code: {}", static_cast<uint32>(result))
		);
	}
}

void Vulkan::CreateGraphicsPipeline(const vector<initializer_list<ShaderInfo>>& shaderInfos)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (const VkResult result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Pipeline Layout! Error Code: {}", static_cast<uint32>(result))
		);
	}

	m_pipelines.reserve(shaderInfos.size());
	int pipelineIndex = 0;
	for (initializer_list shaderInfoSet : shaderInfos)
	{
		vector<Shader*> shaders;
		shaders.reserve(shaderInfoSet.size());

		for (size_t i = 0; i < shaderInfoSet.size(); ++i)
		{
			ShaderInfo info = *(shaderInfoSet.begin() + i);

			Shader* shader = new Shader{ info.shader };
			shader->Load(m_device);

			shaders.emplace_back(shader);
		}

		vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(shaderInfoSet.size());

		for (size_t i = 0; i < shaderInfoSet.size(); ++i)
		{
			const auto& [stage, shaderName, entryPoint] = *(shaderInfoSet.begin() + i);

			VkPipelineShaderStageCreateInfo stageCreateInfo{};
			stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCreateInfo.stage = stage;
			stageCreateInfo.module = shaders[i]->GetModule();
			stageCreateInfo.pName = entryPoint.c_str();

			shaderStages.emplace_back(stageCreateInfo);
		}

		auto bindingDescription = Vertex::GetBindingDescription();
		auto attributeDescription = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		vector dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		m_pipelines.emplace_back(nullptr);
		if (const VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipelines[pipelineIndex++]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create Graphics Pipeline for index '{}'! Error Code: {}",
				pipelineIndex, static_cast<uint32>(result))
			);
		}

		for (Shader* shader : shaders)
		{
			delete shader;
		}
	}
}
#pragma endregion

#pragma region Commands
void Vulkan::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_surface);

	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // This is the best option for recreating every frame
	createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();  // NOLINT(bugprone-unchecked-optional-access)

	if (const VkResult result = vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to create Command Pool! Error Code: {}", static_cast<uint32>(result))
		);
	}
}

void Vulkan::CreateCommandBuffer()
{
	m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // This is what is used for submitting to a queue for execution.
	allocateInfo.commandBufferCount = static_cast<uint32>(m_commandBuffers.size());

	if (const VkResult result = vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data());
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to allocate Command Buffers! Error Code: {}", static_cast<uint32>(result))
		);
	}
}

void Vulkan::RecordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32 imageIndex, const function<void()>& drawCommand) const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (const VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to begin recording Command Buffer! Error Code: {}", static_cast<uint32>(result))
		);
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { .x = 0, .y = 0 };
	renderPassInfo.renderArea.extent = m_swapChainExtent;

	VkClearValue clearColor = { { { m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a } } };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[0]);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapChainExtent.width);
	viewport.height = static_cast<float>(m_swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor;
	scissor.offset = { .x = 0, .y = 0 };
	scissor.extent = m_swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	drawCommand();

	vkCmdEndRenderPass(commandBuffer);

	if (const VkResult result = vkEndCommandBuffer(commandBuffer);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to end recording Command Buffer! Error Code: {}", static_cast<uint32>(result))
		);
	}
}
#pragma endregion

#pragma region Vertex Buffer
void Vulkan::CreateVertexBuffer()
{
	screenTriangleMesh = Mesh::MakeQuad();
	screenTriangleMesh->CreateBuffers(this);
}
#pragma endregion

#pragma region Semaphores
void Vulkan::CreateSyncObjects()
{
	m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// We won't have anything in the buffer the first frame, so we need to mark it as 
	// signaled to essentially skip the first call
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (const VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create Image Available Semaphore! Error Code: {}", static_cast<uint32>(result))
			);
		}

		if (const VkResult result = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create Render Finished Semaphore! Error Code: {}", static_cast<uint32>(result))
			);
		}

		if (const VkResult result = vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]);
			result != VK_SUCCESS)
		{
			throw runtime_error(
				std::format("Failed to create In Flight Fence! Error Code: {}", static_cast<uint32>(result))
			);
		}
	}
}
#pragma endregion

#pragma region Common
Vulkan::Vulkan(GLFWwindow* window, Config* config) :
	m_window{ window }, m_loaded{ false }, m_instance{ VK_NULL_HANDLE }, m_debugMessenger{ VK_NULL_HANDLE },
	m_surface{ VK_NULL_HANDLE }, m_physicalDevice{ VK_NULL_HANDLE }, m_device{ VK_NULL_HANDLE },
	m_graphicsQueue{ VK_NULL_HANDLE }, m_presentQueue{ VK_NULL_HANDLE }, m_swapChain{ VK_NULL_HANDLE },
	m_swapChainFormat{}, m_swapChainExtent{}, m_renderPass{ VK_NULL_HANDLE },
	m_pipelineLayout{ VK_NULL_HANDLE }, m_commandPool{ VK_NULL_HANDLE }, m_commandBuffers{ VK_NULL_HANDLE },
	m_imageAvailableSemaphores{ VK_NULL_HANDLE }, m_renderFinishedSemaphores{ VK_NULL_HANDLE },
	m_inFlightFences{ VK_NULL_HANDLE }, m_frameBufferResized{ false },
	m_currentFrame{ 0 }, m_currentImageIndex{ 0 }
{
	m_engineTitle = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };
	m_vulkanVersion = new Version{ "Engine.Vulkan", config };
	m_appTitle = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };

	m_clearColor = config->Get<Color>("Window.ClrColor");

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
}

Vulkan::~Vulkan()
{
	delete m_engineVersion;
	m_engineVersion = nullptr;

	delete m_vulkanVersion;
	m_vulkanVersion = nullptr;

	delete m_appVersion;
	m_appVersion = nullptr;
}

Buffer* Vulkan::MakeVertexBuffer(const size_t vertexCount) const
{
	Buffer* buffer = new Buffer
	{
		m_physicalDevice, m_device, m_commandPool, m_graphicsQueue,
		sizeof(Vertex), vertexCount,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	buffer->Create();

	return buffer;
}

Buffer* Vulkan::MakeIndexBuffer(const size_t indexCount) const
{
	Buffer* buffer = new Buffer
	{
		m_physicalDevice, m_device, m_commandPool, m_graphicsQueue,
		sizeof(uint16), indexCount,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	buffer->Create();

	return buffer;
}

Buffer* Vulkan::MakeStagingBuffer(const size_t size, const size_t count) const
{
	Buffer* buffer = new Buffer
	{
		m_physicalDevice, m_device, m_commandPool, m_graphicsQueue,
		size, count, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	buffer->Create();

	return buffer;
}

void Vulkan::DestroyBuffer(Buffer*& buffer)
{
	buffer->Destroy();
	delete buffer;
	buffer = nullptr;
}

bool Vulkan::Loaded() const
{
	return m_loaded;
}

void Vulkan::Create(const vector<initializer_list<ShaderInfo>>& shaderInfos)
{
	try
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline(shaderInfos);
		CreateFrameBuffers();
		CreateCommandPool();
		CreateVertexBuffer();
		CreateCommandBuffer();
		CreateSyncObjects();

		// Vulkan fully setup correctly
		m_loaded = true;
	}
	catch (const exception& e)
	{
		// Log out the exception that occurred.
		std::cerr << e.what() << "\n";
	}
}

void Vulkan::Destroy()
{
	// We don't want to destroy Vulkan if it didn't succeed in the first place
	if (!m_loaded)
	{
		return;
	}

	CleanupSwapChain();

	screenTriangleMesh->DestroyBuffer();
	delete screenTriangleMesh;

	for (const VkPipeline& pipeline : m_pipelines)
	{
		vkDestroyPipeline(m_device, pipeline, nullptr);
	}
	m_pipelines.clear();

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for (uint32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	vkDestroyDevice(m_device, nullptr);

	if constexpr (ENABLE_VALIDATION_LAYERS)
	{
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	m_instance = nullptr;

	// Reset the loaded flag for safety
	m_loaded = false;
}

VkCommandBuffer Vulkan::BeginRender()
{
	vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

	m_currentImageIndex = 0;
	const VkResult nIResult = vkAcquireNextImageKHR(
		m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_currentImageIndex
	);

	if (nIResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return nullptr;
	}

	if (nIResult != VK_SUCCESS && nIResult != VK_SUBOPTIMAL_KHR)
	{
		throw runtime_error(
			std::format("Failed to acquire Swap Chain Image! Error Code: {}", static_cast<uint32>(nIResult))
		);
	}

	vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

	vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

	return m_commandBuffers[m_currentFrame];
}

void Vulkan::EndRender()
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (const VkResult result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]);
		result != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to submit draw Command Buffer! Error Code: {}", static_cast<uint32>(result))
		);
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapchains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &m_currentImageIndex;

	presentInfo.pResults = nullptr; // Only useful if doing more than one swap chain

	const VkResult qPResult = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	if (qPResult == VK_ERROR_OUT_OF_DATE_KHR || qPResult == VK_SUBOPTIMAL_KHR || m_frameBufferResized)
	{
		m_frameBufferResized = false;
		RecreateSwapChain();
	}
	else if (qPResult != VK_SUCCESS)
	{
		throw runtime_error(
			std::format("Failed to present Swap Chain! Error Code: {}", static_cast<uint32>(qPResult))
		);
	}

	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
#pragma endregion
