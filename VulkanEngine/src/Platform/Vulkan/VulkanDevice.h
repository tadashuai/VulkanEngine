#pragma once

#include "Core/Ref.h"
#include "Platform/Vulkan/Vulkan.h"

#include <unordered_set>

namespace VE
{
	struct DeviceSpecification
	{
		bool Graphics = true;
		bool Present = true;
		bool Compute = true;
		bool Transfer = true;

		bool SamplerAnisotropy = true;
		bool DiscreteGPU = true;
		std::vector<const char*> DeviceExtensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};

	class VulkanPhysicalDevice : public ReferenceCount
	{
	public:
		struct QueueFamilyIndices
		{
			uint32_t GraphicsFamilyIndex = -1;
			uint32_t PresentFamilyIndex = -1;
			uint32_t ComputeFamilyIndex = -1;
			uint32_t TransferFamilyIndex = -1;
		};

		VulkanPhysicalDevice( const DeviceSpecification& specification );

		bool IsExtensionSupported( const std::string& extensionName ) const;

		VkPhysicalDevice GetVulkanPhysicalDevice() const
		{
			return m_PhysicalDevice;
		}
		const QueueFamilyIndices& GetQueueFamilyIndices() const
		{
			return m_QueueFamilyIndices;
		}

		const VkFormat GetDepthFormat() const
		{
			return m_DepthFormat;
		}

		const DeviceSpecification& GetSpecification() const
		{
			return m_Specification;
		}

	private:
		void Pick();
		bool IsDeviceSuitable( VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features );
		QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device );

		VkFormat FindDepthFormat() const;

	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		DeviceSpecification m_Specification;

		QueueFamilyIndices m_QueueFamilyIndices;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;

		std::unordered_set<std::string> m_SupportedExtensions;

		VkFormat m_DepthFormat;

		friend class VulkanLogicalDevice;
	};

	class VulkanLogicalDevice : public ReferenceCount
	{
	public:
		VulkanLogicalDevice( const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures physicalDeviceFeatures );

		void CreateCommandPool();
		void Destroy();

		VkQueue GetGraphicsQueue()
		{
			return m_GraphicsQueue;
		}
		VkQueue GetPresentQueue()
		{
			return m_PresentQueue;
		}
		VkQueue GetComputeQueue()
		{
			return m_ComputeQueue;
		}
		void SetPresentFamilyIndex( uint32_t index );

		VkCommandBuffer BeginCommandBuffer( bool begin, bool compute = false );
		void EndCommandBuffer( VkCommandBuffer commandBuffer );

		VkDevice GetVulkanLogicalDevice() const
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
		VkCommandPool m_GraphicsCommandPool, m_ComputeCommandPool;

		VkQueue m_GraphicsQueue, m_PresentQueue, m_ComputeQueue, m_TransferQueue;
	};
}
