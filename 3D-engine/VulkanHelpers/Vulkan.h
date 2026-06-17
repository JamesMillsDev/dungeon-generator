#pragma once

#include <string>

#include "Common.h"
#include "Structs.h"
#include "Maths/Alias.h"

using std::string;

class Config;
struct GLFWwindow;
class Version;

class Vulkan
{
	friend class Renderer;

private:
	static bool CheckValidationLayerSupport();
	static vector<const char*> GetRequiredExtensions();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static uint32 RateDeviceSuitability(VkPhysicalDevice device);
	static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

private:
	string m_engineTitle;
	Version* m_engineVersion;
	Version* m_vulkanVersion;
	string m_appTitle;
	Version* m_appVersion;

	bool m_loaded;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	VkSurfaceKHR m_surface;

private:
	Vulkan(Config* config);
	~Vulkan();

private:
	[[nodiscard]] bool Loaded() const;

	void Create(GLFWwindow* window);
	void Destroy();

	void CreateInstance();
	void SetupDebugMessenger();

	void CreateSurface(GLFWwindow* window);

	void PickPhysicalDevice();
	void CreateLogicalDevice();

};