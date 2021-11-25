#pragma once

#include "Platform/Vulkan/Vulkan.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace VE
{
	class VulkanInstance
	{
	public:
		VulkanInstance();
		~VulkanInstance();

		void Init();

		Ref<VulkanPhysicalDevice> GetPhysicsDevice()
		{
			return m_PhysicalDevice;
		}

		Ref<VulkanLogicalDevice> GetDevice()
		{
			return m_LogicalDevice;
		}

		static VkInstance GetInstance()
		{
			return s_Instance;
		}

	private:
		void CreateInstance();

		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo );
		void SetupDebugMessenger();
		void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator );
		bool CheckValidationLayerSupport();

	private:
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanLogicalDevice> m_LogicalDevice;

		inline static VkInstance s_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
	};
}
