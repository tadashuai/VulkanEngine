#pragma once

#include "Platform/Vulkan/VulkanGraphicsContext.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

namespace VE
{
	class VulkanBackend
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<VulkanGraphicsContext> GetVulkanGraphicsContext();
		static Ref<VulkanSwapChain> GetVulkanSwapChain();
	};
}