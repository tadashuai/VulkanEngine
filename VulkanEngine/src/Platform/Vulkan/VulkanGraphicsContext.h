#pragma once

#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace VE
{
	class VulkanGraphicsContext : public ReferenceCount
	{
	public:
		VulkanGraphicsContext() = default;
		virtual ~VulkanGraphicsContext() = default;

		void Init();
		void Shutdown();

		Ref<VulkanLogicalDevice> GetLogicalDevice()
		{
			return m_LogicalDevice;
		}

		static Ref<VulkanGraphicsContext> Get();
		static Ref<VulkanLogicalDevice> GetCurrentDevice()
		{
			return Get()->GetLogicalDevice();
		}
		static VkInstance GetVulkanInstance()
		{
			return ms_Instance;
		}
		static VkAllocationCallbacks* GetAllocator()
		{
			return ms_Allocator;
		}
		static VkPipelineCache GetPipelineCache()
		{
			return Get()->m_PipelineCache;
		}

		static std::vector<Ref<VulkanCommandBuffer>>& GetCommandBuffers()
		{
			return Get()->m_CommandBuffers;
		}
		static Ref<VulkanCommandBuffer>& GetCommandBuffer( uint32_t index )
		{
			auto& commandBuffers = Get()->m_CommandBuffers;
			VE_ASSERT( index < commandBuffers.size() );
			return commandBuffers[ index ];
		}
		static Ref<VulkanCommandBuffer>& GetCurrentCommandBuffer();

	private:
		void CreateVulkanInstance();

		std::vector<const char*> GetRequiredExtensions();
		void PopulateDebugMessengerCreateInfo( VkDebugUtilsMessengerCreateInfoEXT& createInfo );
		void SetupDebugMessenger();
		void DestroyDebugUtilsMessengerEXT( VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator );
		void DestroyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT reportCallback, const VkAllocationCallbacks* pAllocator );
		bool CheckValidationLayerSupport();

	private:
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanLogicalDevice> m_LogicalDevice;

		inline static VkInstance ms_Instance = VK_NULL_HANDLE;
		inline static VkAllocationCallbacks* ms_Allocator = nullptr;
		VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;

		std::vector<Ref<VulkanCommandBuffer>> m_CommandBuffers;
	};
}
