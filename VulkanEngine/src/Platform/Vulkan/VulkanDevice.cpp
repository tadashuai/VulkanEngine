#include "vepch.h"
#include "Platform/Vulkan/VulkanDevice.h"

#include "Platform/Vulkan/VulkanGraphicsContext.h"

namespace VE
{
	VulkanPhysicalDevice::VulkanPhysicalDevice( const DeviceSpecification& specification )
		: m_Specification( specification )
	{
		Pick();

		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &extCount, nullptr );
		if ( extCount > 0 )
		{
			std::vector<VkExtensionProperties> extensions( extCount );
			if ( vkEnumerateDeviceExtensionProperties( m_PhysicalDevice, nullptr, &extCount, &extensions.front() ) == VK_SUCCESS )
			{
				//VE_TRACE( "Selected physical device has {0} extensions", extensions.size() );
				for ( const auto& ext : extensions )
				{
					m_SupportedExtensions.emplace( ext.extensionName );
					//VE_TRACE( "  {0}", ext.extensionName );
				}
			}
		}

		static const float defaultQueuePriority = 1.0f;

		// Graphics queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_QueueFamilyIndices.GraphicsFamilyIndex;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		m_QueueCreateInfos.push_back( queueInfo );

		// Dedicated compute queue
		if ( m_QueueFamilyIndices.ComputeFamilyIndex != m_QueueFamilyIndices.GraphicsFamilyIndex )
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.ComputeFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back( queueInfo );
		}

		// Dedicated transfer queue
		if ( ( m_QueueFamilyIndices.TransferFamilyIndex != m_QueueFamilyIndices.GraphicsFamilyIndex ) && ( m_QueueFamilyIndices.TransferFamilyIndex != m_QueueFamilyIndices.ComputeFamilyIndex ) )
		{
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = m_QueueFamilyIndices.TransferFamilyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			m_QueueCreateInfos.push_back( queueInfo );
		}

		m_DepthFormat = FindDepthFormat();
		VE_ASSERT( m_DepthFormat, "Failed to find a supported format for Depth Buffer!" );
	}

	bool VulkanPhysicalDevice::IsExtensionSupported( const std::string& extensionName ) const
	{
		return m_SupportedExtensions.find( extensionName ) != m_SupportedExtensions.end();
	}

	void VulkanPhysicalDevice::Pick()
	{
		auto instance = VulkanGraphicsContext::GetVulkanInstance();

		uint32_t deviceCount = 0;
		VK_CHECK_RESULT( vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr ) );

		VE_ASSERT( deviceCount != 0, "Failed to find GPUs with Vulkan support!" );

		std::vector<VkPhysicalDevice> devices( deviceCount );
		VK_CHECK_RESULT( vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() ) );

		for ( const auto& device : devices )
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties( device, &properties );

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures( device, &features );

			VkPhysicalDeviceMemoryProperties memory;
			vkGetPhysicalDeviceMemoryProperties( device, &memory );

			if ( IsDeviceSuitable( device, properties, features ) )
			{
				m_PhysicalDevice = device;
				VE_INFO( "Selected physical device: {0}", properties.deviceName );
				switch ( properties.deviceType )
				{
					default:
					case VK_PHYSICAL_DEVICE_TYPE_OTHER:
						VE_INFO( "GPU type is Unknown." );
						break;
					case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
						VE_INFO( "GPU type is Integrated." );
						break;
					case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
						VE_INFO( "GPU type is Discrete." );
						break;
					case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
						VE_INFO( "GPU type is Virtual." );
						break;
					case VK_PHYSICAL_DEVICE_TYPE_CPU:
						VE_INFO( "GPU type is CPU." );
						break;
				}
				VE_INFO( "GPU Driver version: {0}.{1}.{2}", VK_VERSION_MAJOR( properties.driverVersion ), VK_VERSION_MINOR( properties.driverVersion ), VK_VERSION_PATCH( properties.driverVersion ) );
				VE_INFO( "Vulkan API version: {0}.{1}.{2}", VK_VERSION_MAJOR( properties.apiVersion ), VK_VERSION_MINOR( properties.apiVersion ), VK_VERSION_PATCH( properties.apiVersion ) );

				for ( uint32_t i = 0; i < memory.memoryHeapCount; i++ )
				{
					float memorySizeGib = ( ( float )( memory.memoryHeaps[ i ].size ) ) / 1024.0f / 1024.0f / 1024.0f;
					if ( memory.memoryHeaps[ i ].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT )
					{
						VE_INFO( "Local GPU memory: {0:.2f} GiB", memorySizeGib );
					}
					else
					{
						VE_INFO( "Shared System memory: {0:.2f} GiB", memorySizeGib );
					}
				}

				break;
			}
		}

		VE_ASSERT( m_PhysicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU!" );
		VE_INFO( "Physical device selected." );
	}

	bool VulkanPhysicalDevice::IsDeviceSuitable( VkPhysicalDevice device, VkPhysicalDeviceProperties properties, VkPhysicalDeviceFeatures features )
	{
		if ( m_Specification.DiscreteGPU )
		{
			if ( properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU )
			{
				VE_INFO( "Device is not a discrete GPU, and one is required. Skipping." );
				return false;
			}
		}

		QueueFamilyIndices indices = FindQueueFamilies( device );
		if ( ( !m_Specification.Graphics || indices.GraphicsFamilyIndex != -1 ) &&
			( !m_Specification.Compute || indices.ComputeFamilyIndex != -1 ) &&
			( !m_Specification.Transfer || indices.TransferFamilyIndex != -1 ) )
		{
			VE_TRACE( "Graphics Family Index: {0}", indices.GraphicsFamilyIndex );
			VE_TRACE( "Compute  Family Index: {0}", indices.ComputeFamilyIndex );
			VE_TRACE( "Transfer Family Index: {0}", indices.TransferFamilyIndex );

			if ( m_Specification.DeviceExtensionNames.size() > 0 )
			{
				uint32_t extCount = 0;
				VK_CHECK_RESULT( vkEnumerateDeviceExtensionProperties( device, nullptr, &extCount, nullptr ) );
				if ( extCount > 0 )
				{
					std::vector<VkExtensionProperties> extensions( extCount );
					VK_CHECK_RESULT( vkEnumerateDeviceExtensionProperties( device, nullptr, &extCount, &extensions.front() ) );

					for ( const auto& extRequired : m_Specification.DeviceExtensionNames )
					{
						bool found = false;
						for ( const auto& ext : extensions )
						{
							if ( strcmp( extRequired, ext.extensionName ) == 0 )
							{
								found = true;
								break;
							}
						}
						if ( !found )
						{
							VE_INFO( "Required extension not found: {0}, skipping device.", extRequired );
							return false;
						}
					}
				}
			}

			if ( m_Specification.SamplerAnisotropy && !features.samplerAnisotropy )
			{
				VE_INFO( "Device does not support samplerAnisotropy, skipping." );
				return false;
			}

			m_QueueFamilyIndices = indices;
			return true;
		}

		return false;
	}

	VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies( VkPhysicalDevice device )
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

		std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
		vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

		int i = 0;
		uint8_t minTransferScore = 255;
		for ( const auto& queueFamily : queueFamilies )
		{
			uint8_t currentTransferScore = 0;
			if ( queueFamily.queueCount > 0 )
			{
				if ( indices.GraphicsFamilyIndex == -1 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
				{
					indices.GraphicsFamilyIndex = i;
					currentTransferScore++;
				}
				if ( queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT )
				{
					indices.ComputeFamilyIndex = i;
					currentTransferScore++;
				}
				if ( queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT )
				{
					if ( currentTransferScore <= minTransferScore )
					{
						minTransferScore = currentTransferScore;
						indices.TransferFamilyIndex = i;
					}
				}
			}
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
			if ( formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
				return format;
			else if ( formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT )
				return format;
		}
		return VK_FORMAT_UNDEFINED;
	}

	VulkanLogicalDevice::VulkanLogicalDevice( const Ref<VulkanPhysicalDevice>& physicalDevice, VkPhysicalDeviceFeatures physicalDeviceFeatures )
		: m_PhysicalDevice( physicalDevice ), m_PhysicalDeviceFeatures( physicalDeviceFeatures )
	{
		std::vector<const char*> deviceExtensions = m_PhysicalDevice->GetSpecification().DeviceExtensionNames;

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

		VK_CHECK_RESULT( vkCreateDevice( m_PhysicalDevice->GetVulkanPhysicalDevice(), &createInfo, VulkanGraphicsContext::GetAllocator(), &m_LogicalDevice ) );
		VE_INFO( "Logical device created." );

		vkGetDeviceQueue( m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.GraphicsFamilyIndex, 0, &m_GraphicsQueue );
		vkGetDeviceQueue( m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.ComputeFamilyIndex, 0, &m_ComputeQueue );
		vkGetDeviceQueue( m_LogicalDevice, m_PhysicalDevice->m_QueueFamilyIndices.TransferFamilyIndex, 0, &m_TransferQueue );
		VE_INFO( "Queues obtained." );

		CreateCommandPool();
	}

	void VulkanLogicalDevice::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.GraphicsFamilyIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice, &poolInfo, VulkanGraphicsContext::GetAllocator(), &m_GraphicsCommandPool ) );
		VE_INFO( "Graphics command pool created." );

		poolInfo.queueFamilyIndex = m_PhysicalDevice->m_QueueFamilyIndices.ComputeFamilyIndex;
		VK_CHECK_RESULT( vkCreateCommandPool( m_LogicalDevice, &poolInfo, VulkanGraphicsContext::GetAllocator(), &m_ComputeCommandPool ) );
		VE_INFO( "Compute command pool created." );
	}

	void VulkanLogicalDevice::Destroy()
	{
		vkDestroyCommandPool( m_LogicalDevice, m_GraphicsCommandPool, VulkanGraphicsContext::GetAllocator() );
		VE_INFO( "Graphics command pool destroyed." );
		vkDestroyCommandPool( m_LogicalDevice, m_ComputeCommandPool, VulkanGraphicsContext::GetAllocator() );
		VE_INFO( "Compute command pool destroyed." );

		vkDeviceWaitIdle( m_LogicalDevice );
		vkDestroyDevice( m_LogicalDevice, VulkanGraphicsContext::GetAllocator() );
		VE_INFO( "Logical deveice destroyed." );
	}

	void VulkanLogicalDevice::SetPresentFamilyIndex( uint32_t index )
	{
		m_PhysicalDevice->m_QueueFamilyIndices.PresentFamilyIndex = index;
		VE_TRACE( "Present Family Index: {0}", index );
		vkGetDeviceQueue( m_LogicalDevice, index, 0, &m_PresentQueue );
	}

	VkCommandBuffer VulkanLogicalDevice::BeginCommandBuffer( bool begin, bool compute /*= false */ )
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
		cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocateInfo.commandPool = compute ? m_ComputeCommandPool : m_GraphicsCommandPool;
		cmdBufAllocateInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VK_CHECK_RESULT( vkAllocateCommandBuffers( m_LogicalDevice, &cmdBufAllocateInfo, &cmdBuffer ) );

		if ( begin )
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if ( !compute )
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			VK_CHECK_RESULT( vkBeginCommandBuffer( cmdBuffer, &beginInfo ) );
		}

		return cmdBuffer;
	}

	void VulkanLogicalDevice::EndCommandBuffer( VkCommandBuffer commandBuffer )
	{
		VE_ASSERT( commandBuffer != VK_NULL_HANDLE );

		VK_CHECK_RESULT( vkEndCommandBuffer( commandBuffer ) );

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//VkFenceCreateInfo fenceCreateInfo{};
		//fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		//fenceCreateInfo.flags = 0;
		//VkFence fence;
		//VK_CHECK_RESULT( vkCreateFence( m_LogicalDevice, &fenceCreateInfo, VulkanGraphicsContext::GetAllocator(), &fence ) );

		//VK_CHECK_RESULT( vkWaitForFences( m_LogicalDevice, 1, &fence, VK_TRUE, UINT_MAX ) );
		VK_CHECK_RESULT( vkQueueSubmit( m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) );
		VK_CHECK_RESULT( vkQueueWaitIdle( m_GraphicsQueue ) );

		//vkDestroyFence( m_LogicalDevice, fence, VulkanGraphicsContext::GetAllocator() );
		vkFreeCommandBuffers( m_LogicalDevice, m_GraphicsCommandPool, 1, &commandBuffer );
	}
}
