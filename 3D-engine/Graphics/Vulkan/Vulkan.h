#pragma once

#include <functional>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "Utility/ResourceStack.h"

class Config;
class Version;

using InitFunction = std::function<void()>;
using std::string;
using std::vector;

class Vulkan
{
	friend class Renderer;

private:
	Version* m_appVersion;
	string m_appName;
	Version* m_engineVersion;
	string m_engineName;

	VkAllocationCallbacks m_callbacks;
	ResourceStack* m_resourceStack;
	bool m_loaded;

	VkInstance m_instance;

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkQueue m_queue;

private:
	explicit Vulkan(Config* config);
	~Vulkan();

private:
	void Init();
	[[nodiscard]] bool IsLoaded() const;

	void InitAndPushResource( const InitFunction& init, const CleanupFunction& cleanup) const;

};
