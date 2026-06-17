#include "pch.h"
#include "Vulkan.h"

#include <stdexcept>

#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <GLFW/glfw3.h>

using std::runtime_error;

Vulkan::Vulkan(Config* config, GLFWwindow* window)
	: m_instance{ VK_NULL_HANDLE }
{
	m_engineTitle = config->Get<string>("Engine.Title");
	m_engineVersion = new Version{ "Engine.Version", config };
	m_vulkanVersion = new Version{ "Engine.Vulkan", config };
	m_appTitle = config->Get<string>("Application.Title");
	m_appVersion = new Version{ "Application.Version", config };

	Create(window);
}

Vulkan::~Vulkan()
{
	Destroy();

	delete m_engineVersion;
	m_engineVersion = nullptr;

	delete m_vulkanVersion;
	m_vulkanVersion = nullptr;

	delete m_appVersion;
	m_appVersion = nullptr;
}

void Vulkan::Create(GLFWwindow* window)
{
	CreateInstance();
}

void Vulkan::Destroy()
{
	vkDestroyInstance(m_instance, nullptr);
	m_instance = nullptr;
}

void Vulkan::CreateInstance()
{
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

	// Get the GLFW extensions and the extension count
	uint32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector<const char*> requiredExtensions;
	requiredExtensions.reserve(glfwExtensionCount);
	for (uint32 i = 0; i < glfwExtensionCount; ++i)
	{
		requiredExtensions.emplace_back(glfwExtensions[i]);
	}

	// Get the possible extension properties
	uint32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// Log out the available extensions
	std::cout << "available extensions:\n";

	for (const auto& extension : extensions)
	{
		std::cout << '\t' << extension.extensionName << '\n';
	}

	// Add in the extra required extensions
	requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

	// Update the flags to use the portability bit
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	createInfo.enabledExtensionCount = static_cast<uint32>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	createInfo.enabledLayerCount = 0;

	// Attempt to create the instance, throwing a runtime error with the error code if failed.
	if (VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance); result != VK_SUCCESS)
	{
		throw runtime_error(std::format("Failed to create Vulkan Instance! Error Code: {}", static_cast<uint32>(result)));
	}
}
