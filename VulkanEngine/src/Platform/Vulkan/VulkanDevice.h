#pragma once

#include "Platform/Vulkan/Vulkan.h"

#include <unordered_set>

namespace VE
{
	class VulkanPhysicalDevice
	{
	public:
		struct QueueFamilyIndices
		{
			uint32_t Graphics = -1;
			uint32_t Compute = -1;
			uint32_t Transfer = -1;
			bool IsComplete()
			{
				return Graphics != -1 && Compute != -1 && Transfer != -1;
			}
		};

		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();

		bool IsExtensionSupported( const std::string& extensionName ) const;

		VkPhysicalDevice GetVulkanPhysicalDevice() const
		{
			return m_PhysicalDevice;
		}

		static Ref<VulkanPhysicalDevice> Pick();

	private:
		bool IsDeviceSuitable( VkPhysicalDevice device );
		QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device );

		VkFormat FindDepthFormat() const;
		QueueFamilyIndices GetQueueFamilyIndices( int flags );

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_Properties;
		VkPhysicalDeviceFeatures m_Features;

		QueueFamilyIndices m_QueueFamilyIndices;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

		std::unordered_set<std::string> m_SupportedExtensions;

		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;

		friend class VulkanLogicalDevice;
	};

	class VulkanLogicalDevice
	{
	public:
		VulkanLogicalDevice( const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures physicalDeviceFeatures );
		~VulkanLogicalDevice();

		void CreateCommandPool();
		void Destroy();

		VkQueue GetGraphicsQueue()
		{
			return m_GraphicsQueue;
		}
		VkQueue GetComputeQueue()
		{
			return m_ComputeQueue;
		}

		VkDevice GetVulkantLogicalDevice() const
		{
			return m_LogicalDevice;
		}
		const Ref<VulkanPhysicalDevice>& GetPhysicalDevice() const
		{
			return m_PhysicalDevice;
		}

	private:
		VkDevice m_LogicalDevice = nullptr;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
		VkCommandPool m_CommandPool, m_ComputeCommandPool;

		VkQueue m_GraphicsQueue, m_ComputeQueue;
	};
}
