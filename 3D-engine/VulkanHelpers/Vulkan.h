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

	bool m_loaded;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;

	VkSwapchainKHR m_swapChain;
	vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainFormat;
	VkExtent2D m_swapChainExtent;

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

	void CreateSwapChain(GLFWwindow* window);

};