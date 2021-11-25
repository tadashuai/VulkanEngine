#include "vepch.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include "Platform/Vulkan/VulkanInstance.h"

#include <set>

namespace VE
{

	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		auto instance = VulkanInstance::GetInstance();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );

		VE_ASSERT( deviceCount != 0, "Failed to find GPUs with Vulkan support!" );

		std::vector<VkPhysicalDevice> devices( deviceCount );
		vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() );

		for ( const auto& device : devices )
		{
			if ( IsDeviceSuitable( device ) )
			{
				m_PhysicalDevice = device;
				break;
			}
		}

		VE_ASSERT( m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU!" );

		vkGetPhysicalDeviceProperties( m_PhysicalDevice, &m_Properties );
		VE_TRACE( "physical device: {0}", m_Properties.deviceName );

		vkGetPhysicalDeviceFeatures( m_PhysicalDevice, &m_Features );

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queueFamilyCount, nullptr );
		VE_ASSERT( queueFamilyCount > 0 );
		m_QueueFamilyProperties.resize( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data() );

		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &extCount, nullptr );
		if ( extCount > 0 )
		{
			std::vector<VkExtensionProperties> extensions( extCount );
			if ( vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &extCount, &extensions.front() ) == VK_SUCCESS )
			{
				VE_TRACE( "Selected physical device has {0} extensions", extensions.size() );
				for ( const auto& ext : extensions )
				{
					m_SupportedExtensions.emplace( ext.extensionName );
					VE_TRACE( "  {0}", ext.extensionName );
				}
			}
		}

		static const float defaultQueuePriority = 0.0f;
		m_QueueFamilyIndices = GetQueueFamilyIndices( VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT );

		// Graphics queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		m_QueueCreateInfos.push_back( queueInfo );

		// Dedicated compute queue
		if ( m_QueueFamilyIndices.Compute != m_QueueFamilyIndices.Graphics )
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Compute;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back( queueInfo );
		}

		// Dedicated transfer queue
		if ( ( m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Graphics ) && ( m_QueueFamilyIndices.Transfer != m_QueueFamilyIndices.Compute ) )
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.Transfer;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back( queueInfo );
		}

		m_DepthFormat = FindDepthFormat();
		VE_ASSERT( m_DepthFormat );
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{
	}

	bool VulkanPhysicalDevice::IsExtensionSupported( const std::string& extensionName ) const
	{
		return m_SupportedExtensions.find( extensionName ) != m_SupportedExtensions.end();
	}

	Ref<VulkanPhysicalDevice> VulkanPhysicalDevice::Pick()
	{
		return CreateRef<VulkanPhysicalDevice>();
	}

	bool VulkanPhysicalDevice::IsDeviceSuitable( VkPhysicalDevice device )
	{
		QueueFamilyIndices indices = FindQueueFamilies( device );

		return indices.IsComplete();
	}

	VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies( VkPhysicalDevice device )
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

		int i = 0;
		for ( const auto& queueFamily : queueFamilies )
		{
			if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
			{
				indices.Graphics = i;
			}
			if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
			{
				indices.Compute = i;
			}
			if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT )
			{
				indices.Transfer = i;
			}
			if ( indices.IsComplete() )
				break;

			i++;
		}

		return indices;
	}

	VkFormat VulkanPhysicalDevice::FindDepthFormat() const
	{
		std::vector<VkFormat> depthFormats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for ( auto& format : depthFormats )
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties( m_PhysicalDevice, format, &formatProps );
			if ( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
				return format;
		}
		return VK_FORMAT_UNDEFINED;
	}

	VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices( int flags )
	{
		QueueFamilyIndices indices;

		// Dedicated queue for compute
		// Try to find a queue family index that supports compute but not graphics
		if ( flags & VK_QUEUE_COMPUTE_BIT )
		{
			for ( uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++ )
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[ i ];
				if ( ( queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT ) && ( ( queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT ) == 0 ) )
				{
					indices.Compute = i;
					break;
				}
			}
		}

		// Dedicated queue for transfer
		// Try to find a queue family index that supports transfer but not graphics and compute
		if ( flags & VK_QUEUE_TRANSFER_BIT )
		{
			for ( uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++ )
			{
				auto& queueFamilyProperties = m_QueueFamilyProperties[ i ];
				if ( ( queueFamilyProperties.queueFlags & VK_QUEUE_TRANSFER_BIT ) && ( ( queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT ) == 0 ) && ( ( queueFamilyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT ) == 0 ) )
				{
					indices.Transfer = i;
					break;
				}
			}
		}

		// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
		for ( uint32_t i = 0; i < m_QueueFamilyProperties.size(); i++ )
		{
			if ( flags & VK_QUEUE_GRAPHICS_BIT )
			{
				if ( m_QueueFamilyProperties[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
					indices.Graphics = i;
			}
		}

		return indices;
	}

	VulkanLogicalDevice::VulkanLogicalDevice( const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures physicalDeviceFeatures )
		: m_PhysicalDevice( physicalDevice ), m_PhysicalDeviceFeatures( physicalDeviceFeatures )
	{
		std::vector<const char*> deviceExtensions;
		// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
		VE_ASSERT( m_PhysicalDevice->IsExtensionSupported( VK_KHR_SWAPCHAIN_EXTENSION_NAME ) );
		deviceExtensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );

		if ( m_PhysicalDevice->IsExtensionSupported( VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME ) )
			deviceExtensions.push_back( VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME );
		if ( m_PhysicalDevice->IsExtensionSupported( VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME ) )
			deviceExtensions.push_back( VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME );
		if ( m_PhysicalDevice->IsExtensionSupported( VK_EXT_DEBUG_MARKER_EXTENSION_NAME ) )
			deviceExtensions.push_back( VK_EXT_DEBUG_MARKER_EXTENSION_NAME );

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast< uint32_t >( physicalDevice->m_QueueCreateInfos.size() );
		createInfo.pQueueCreateInfos = physicalDevice->m_QueueCreateInfos.data();
		createInfo.pEnabledFeatures = &physicalDeviceFeatures;
		if ( deviceExtensions.size() > 0 )
		{
			createInfo.enabledExtensionCount = static_cast< uint32_t >( deviceExtensions.size() );
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		VK_CHECK_RESULT( vkCreateDevice( m_PhysicalDevice->GetVulkanPhysicalDevice(), &createInfo, nullptr, &m_LogicalDevice ) );

		vkGetDeviceQueue( m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Graphics, 0, &m_GraphicsQueue );
		vkGetDeviceQueue( m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.Compute, 0, &m_ComputeQueue );

		CreateCommandPool();
	}

	VulkanLogicalDevice::~VulkanLogicalDevice()
	{
	}

	void VulkanLogicalDevice::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Graphics;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool ) );

		poolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.Compute;
		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice, &poolInfo, nullptr, &m_ComputeCommandPool ) );
	}

	void VulkanLogicalDevice::Destroy()
	{
		vkDestroyCommandPool( m_LogicalDevice, m_CommandPool, nullptr );
		vkDestroyCommandPool( m_LogicalDevice, m_ComputeCommandPool, nullptr );

		vkDeviceWaitIdle( m_LogicalDevice );
		vkDestroyDevice( m_LogicalDevice, nullptr );
	}

}
