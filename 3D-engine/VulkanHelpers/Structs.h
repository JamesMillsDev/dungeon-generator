#pragma once

#include "Common.h"

#include <optional>

using std::optional;

struct QueueFamilyIndices
{
	optional<uint32_t> graphicsFamily;
	optional<uint32_t> presentFamily;

	[[nodiscard]] bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	vector<VkSurfaceFormatKHR> formats;
	vector<VkPresentModeKHR> presentModes;
};