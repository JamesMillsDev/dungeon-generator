#include "pch.h"
#include "Vulkan.h"

#include <stdexcept>
#include <GLFW/glfw3.h>

#include "Utility/Console.h"
#include "Utility/ResourceStack.h"

using std::exception;
using std::runtime_error;

constexpr int32 DEFAULT_RESOURCE_STACK_SIZE = 16;

Vulkan::Vulkan(Config* config)
	: m_callbacks{ VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE },
	m_resourceStack{ new ResourceStack{ DEFAULT_RESOURCE_STACK_SIZE } }, m_loaded{ false }
{
	m_appName = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };
	m_engineName = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };

	Init();
}

Vulkan::~Vulkan()
{
	delete m_resourceStack;
	delete m_appVersion;
	delete m_engineVersion;
}

void Vulkan::Init()
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

				if (vkCreateInstance(&instanceInfo, &m_callbacks, &m_instance) != VK_SUCCESS)
				{
					throw runtime_error("Failed to create Vulkan Instance!");
				}
			},
			[this]
			{
				vkDestroyInstance(m_instance, &m_callbacks);
			}
		);

		InitAndPushResource(
			[this]
			{
				// Get a list and number of all GPU's attached to the computer
				uint32 deviceCount = 0;
				vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
				vector<VkPhysicalDevice> devices(deviceCount);
				vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

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
				if (glfwGetPhysicalDevicePresentationSupport(m_instance, m_physicalDevice, queueFamily) == GLFW_FALSE)
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
				if (vkCreateDevice(m_physicalDevice, &deviceCI, &m_callbacks, &m_device) != VK_SUCCESS)
				{
					throw runtime_error("Failed to create Logical Device!");
				}

				// Get the graphics queue
				vkGetDeviceQueue(m_device, queueFamily, 0, &m_queue);
			},
			[this]
			{
				vkDestroyDevice(m_device, &m_callbacks);
			}
		);

		m_loaded = true;
	}
	catch (exception& e)
	{
		Console::Exception(e);
	}
}

bool Vulkan::IsLoaded() const
{
	return m_loaded;
}

void Vulkan::InitAndPushResource(const InitFunction& init, const CleanupFunction& cleanup) const
{
	init();
	m_resourceStack->Push(cleanup);
}