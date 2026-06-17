#include "pch.h"
#include "Vulkan.h"

#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using std::exception;
using std::multimap;
using std::runtime_error;
using std::set;

#pragma region VkInstance / Debug Messenger
namespace
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
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

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
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

	createInfo.enabledLayerCount = 0;
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
void Vulkan::CreateSurface(GLFWwindow* window)
{
	if (const VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface);
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

	return score;
}

QueueFamilyIndices Vulkan::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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

	createInfo.enabledExtensionCount = 0;
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
}
#pragma endregion

#pragma region Common
Vulkan::Vulkan(Config* config)
	: m_loaded{ false }, m_instance{ VK_NULL_HANDLE }, m_debugMessenger{ VK_NULL_HANDLE },
	m_surface{ VK_NULL_HANDLE }, m_physicalDevice{ VK_NULL_HANDLE }, m_device{ VK_NULL_HANDLE }
{
	m_engineTitle = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };
	m_vulkanVersion = new Version{ "Engine.Vulkan", config };
	m_appTitle = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };
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

bool Vulkan::Loaded() const
{
	return m_loaded;
}

void Vulkan::Create(GLFWwindow* window)
{
	try
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface(window);
		PickPhysicalDevice();
		CreateLogicalDevice();

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
#pragma endregion